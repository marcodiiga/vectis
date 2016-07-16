#include <UI/CodeTextEdit/Document.h>
#include <UI/CodeTextEdit/CodeTextEdit.h>
#include <UI/CodeTextEdit/Lexers/Lexer.h>
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include <QtConcurrent>
#include <functional>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

#include <QDebug>
#include <QElapsedTimer>

Document::Document(const CodeTextEdit& codeTextEdit) :
  m_codeTextEdit(codeTextEdit),
  m_wrapWidth(-1),
  m_needReLexing(false)
{  
  // A document has always at least one physical line and an editorline
  m_physicalLines.resize(1);
  m_physicalLines.back().m_editorLines.emplace_back(QString());
  setCursorPos(0, 0);
}

// The following function loads the contents of a text file into memory.
// This is a memory-expensive operation but documents need to be available at any time
// Returns true on success
bool Document::loadFromFile(QString file) {

  std::ifstream f(file.toStdString(), std::ios::binary /* Necessary to preserve e.g. \r on Windows */);
  if (!f)
    return false;

  // Load the entire file into memory (memory-expensive but necessary)
  std::stringstream strStream;
  strStream << f.rdbuf();
  std::string str = strStream.str();

  f.close();

  // Separate lines into different chunks when \n or \r\n is found
  m_plainTextLines.clear();
  QString line;
  for(auto ch : str) {
    line += ch;
    if (ch == '\n') {

      QRegExp tabs("\t");
      line.replace(tabs, QString(4, 0x07)); // Use the 0x07 BELL ascii char as tabulation marker

      m_plainTextLines.push_back(std::move(line));
      line.clear();
    }
  }
  // Add an extra empty line if the last line ended with a \n
  if (!m_plainTextLines.empty()) {
    auto& lastLine = m_plainTextLines.back();
    if (!lastLine.isEmpty() && lastLine.endsWith('\n'))
      m_plainTextLines.emplace_back();
  }

  return true;
}

void Document::applySyntaxHighlight(SyntaxHighlight s) {
  m_needReLexing = false;
  switch(s) {
    case NONE: {
      if (m_lexer) { // Check if there were a lexer before (i.e. the smart pointer was set)
        m_lexer.release();
        m_needReLexing = true; // Syntax has been changed, re-lex the document at the next recalculate
      }
    } break;
    case CPP: {
      if (!m_lexer || isLexer(m_lexer.get()).ofType(CPPLexerType) == false) {
        m_lexer.reset( LexerBase::createLexerOfType(CPPLexerType) );
        m_needReLexing = true; // Syntax has been changed, re-lex the document at the next recalculate
      }
    } break;
  }


  // If the document was already recalculated, just recalculate and apply the new syntax highlight
  if (m_firstDocumentRecalculate == false)
    recalculateDocumentLines ();
}

// Triggers a lines/breakpoints recalculation for the entire document. -1 means
// "no wrap" and it's the default
void Document::setWrapWidth(int width) {
  m_wrapWidth = width;
  // qDebug() << "setWrapWidth() to " << width;
  recalculateDocumentLines (); // Recalculate the editor lines with this wrap value
}


namespace {

  struct VecAccumulator {
    VecAccumulator() = default;
    std::vector<PhysicalLine> m_physicalLines;
  };

}

// A complex multithreaded function which performs wrap-around calculations and, if necessary, triggers
// a complete relexing of the document
void Document::recalculateDocumentLines () {

  //QElapsedTimer timer;
  //timer.start();

  m_firstDocumentRecalculate = false;

  if (m_needReLexing) {
    QString m_plainText;
    for(auto& line : m_plainTextLines) // Expensive, hopefully this doesn't happen too often - LEX DIRECTLY FROM VECTOR
      m_plainText.append(line);
    m_lexer->lexInput(std::move(m_plainText.toStdString()), m_styleDb);
    m_needReLexing = false;
  }

  // Store how many lines

  // Drop previous lines
  m_physicalLines.clear();
  m_numberOfEditorLines = 0;
  // m_maximumCharactersLine = 0;


  std::function<std::vector<PhysicalLine> (const QString&)> mapFn = [&,this](const QString& line) {

    std::vector<PhysicalLine> phLineVec;
    QString restOfLine;
    std::vector<EditorLine> edLines;
    edLines.reserve(10); // Should be enough for every splitting
    if (m_wrapWidth != -1 && // Also check if the monospace'd width isn't exceeding the viewport
        line.count() * m_codeTextEdit.getCharacterWidthPixels() > m_wrapWidth) {
      // We have a wrap and the line is too big - WRAP IT

      edLines.clear();
      restOfLine = line;

      // Calculate the allowed number of characters per editor line
      int maxChars = static_cast<int>( m_wrapWidth / m_codeTextEdit.getCharacterWidthPixels() );
      if (maxChars < 10)
        maxChars = 10; // Keep it to a minimum

      // Start the wrap-splitting algorithm or resort to a brute-force character splitting one if
      // no suitable spaces could be found to split the line
      while(restOfLine.size() > maxChars) {

        int bestSplittingPointFound = -1;
        for(int i = 0; i < restOfLine.count(); ++i) {
          if (i > maxChars)
            break; // We couldn't find a suitable space split point for restOfLine
          if ( restOfLine[i] == ' ' && i != 0 /* Doesn't make sense to split at 0 pos */)
            bestSplittingPointFound = i;
        }

        if (bestSplittingPointFound != -1) { // We found a suitable space point to split this line
          edLines.push_back( restOfLine.left( bestSplittingPointFound ));
          restOfLine = restOfLine.right( restOfLine.size() - bestSplittingPointFound );
        } else {
          // No space found, brutally split characters
          edLines.push_back( restOfLine.left( maxChars ));
          restOfLine = restOfLine.right( restOfLine.size() - maxChars );
          //qDebug() << "BRUTAL SPLIT" << bestSplittingPointFound;
        }
      }
      edLines.push_back( restOfLine ); // Insert the last part and proceed

      // No need to do anything special for tabs - they're automatically converted into spaces

      std::vector<EditorLine> edVector;
      std::copy( edLines.begin(), edLines.end(), std::back_inserter(edVector) );
      phLineVec.resize(1);
      phLineVec[0].m_editorLines = std::move(edVector);

    } else { // No wrap or the line fits perfectly within the wrap limits

      EditorLine el(line);
      phLineVec.emplace_back( std::move(el) );
    }
    return phLineVec;
  };


  std::function<void (std::vector<PhysicalLine>&, const std::vector<PhysicalLine>&)> reduceFn =
      [&,this](std::vector<PhysicalLine>& accumulator, const std::vector<PhysicalLine>& pl) {

    accumulator.insert(accumulator.end(), pl.begin(), pl.end());

    m_numberOfEditorLines += static_cast<int>(pl[0].m_editorLines.size()); // Some more EditorLine
//    std::for_each(pl[0].m_editorLines.begin(), pl[0].m_editorLines.end(), [this](const EditorLine& eline) {
//      int lineLength = static_cast<int>(eline.m_characters.size());
//      if (lineLength > m_maximumCharactersLine) // Check if this is the longest line found ever
//        m_maximumCharactersLine = lineLength;
//    });

  };

  // >> Launch the workload <<
  m_physicalLines =
      QtConcurrent::blockingMappedReduced<std::vector<PhysicalLine>>(m_plainTextLines.begin(), m_plainTextLines.end(),
                                                                     mapFn, reduceFn, QtConcurrent::SequentialReduce );


  // Update the cursor position according to the new wrapping
  // Invariant: m_documentCursorPos.pl and m_documentCursorPos.ch shouldn't change here. The Els might change.
  if(!m_plainTextLines.empty()) {
    int countPL = 0;
    int countEL = 0;
    for(auto& pl : m_physicalLines) {
      if (m_documentCursorPos.pl == countPL)
        break;
      countEL += (int)pl.m_editorLines.size();
      ++countPL;
    }

    Q_ASSERT (countPL < m_physicalLines.size()); // Should never be fired

    // Find the EL where the character requested is now stored
    int countCH = 0;
    int ELrelativeCH = 0; // Character position relative to the beginning of the EL
    int relativeEL = 0; // EL inside this PL till the cursor position
    for(int i = 0; i < m_physicalLines[countPL].m_editorLines.size(); ++i) {
      // >, because being equal (caret blinking at the end of the line) also works
      if (m_documentCursorPos.ch > countCH + m_physicalLines[countPL].m_editorLines[relativeEL].m_characters.size()) {
        // Explore another EL inside this PL
        countCH += (int)m_physicalLines[countPL].m_editorLines[relativeEL].m_characters.size();
        ++relativeEL;
      } else {
        // Found the EL where the cursor position was before the wrap
        ELrelativeCH = m_documentCursorPos.ch - countCH;
        break;
      }
    }
    countEL += relativeEL;


    m_documentCursorPos.el = countEL;
    m_documentCursorPos.relativeEl = relativeEL;
    m_documentCursorPos.relativeCh = ELrelativeCH;

    // Set the correct position within the viewport
    m_viewportCursorPos.y = countEL;
    m_viewportCursorPos.x = ELrelativeCH;
  }


//qDebug() << "done";



//qDebug() << "Done recalculating document lines in " << timer.elapsed() << " milliseconds";







//  // Drop previous lines
//  m_physicalLines.clear();
//  m_numberOfEditorLines = 0;
//  m_maximumCharactersLine = 0;
//  // Reset temporary buffers
//  std::vector<EditorLine> edLines;
//  QString restOfLine;
//  edLines.reserve(10); // Should be enough for every splitting

//  // Scan each line (until a newline is found)
//  QString m_plainText;
//  for(auto& line : m_plainTextLines) { // Expensive, hopefully this doesn't happen too often - LEX DIRECTLY FROM VECTOR
//    m_plainText.append(line);
//    m_plainText += '\n';
//  }
//  QTextStream ss(&m_plainText);
//  QString line;
//  do {
//    line = ss.readLine();
//    if (line.isNull())
//      continue;

//    if (m_wrapWidth != -1 && // Also check if the monospace'd width isn't exceeding the viewport
//        line.count() * m_codeTextEdit.getCharacterWidthPixels() > m_wrapWidth) {
//      // We have a wrap and the line is too big - WRAP IT

//      edLines.clear();
//      restOfLine = line;

//      // Calculate the allowed number of characters per editor line
//      int maxChars = static_cast<int>( m_wrapWidth / m_codeTextEdit.getCharacterWidthPixels() );
//      if (maxChars < 10)
//        maxChars = 10; // Keep it to a minimum

//      // Start the wrap-splitting algorithm or resort to a brute-force character splitting one if
//      // no suitable spaces could be found to split the line
//      while(restOfLine.size() > maxChars) {

//        int bestSplittingPointFound = -1;
//        for(int i = 0; i < restOfLine.count(); ++i) {
//          if (i > maxChars)
//            break; // We couldn't find a suitable space split point for restOfLine
//          if ( restOfLine[i] == ' ' && i != 0 /* Doesn't make sense to split at 0 pos */)
//            bestSplittingPointFound = i;
//        }

//        if (bestSplittingPointFound != -1) { // We found a suitable space point to split this line
//          edLines.push_back( restOfLine.left( bestSplittingPointFound ));
//          restOfLine = restOfLine.right( restOfLine.size() - bestSplittingPointFound );
//        } else {
//          // No space found, brutally split characters
//          edLines.push_back( restOfLine.left( maxChars ));
//          restOfLine = restOfLine.right( restOfLine.size() - maxChars );
//          qDebug() << "BRUTAL SPLIT" << bestSplittingPointFound;
//        }
//      }
//      edLines.push_back( restOfLine ); // Insert the last part and proceed

//      for(auto& ll : edLines)
//        if (ll.m_characters.size() > m_maximumCharactersLine) // Update if this is the longest line ever found
//          m_maximumCharactersLine = static_cast<int>(ll.m_characters.size());

//      // No need to do anything special for tabs - they're automatically converted into spaces
//      PhysicalLine phLine;
//      std::vector<EditorLine> edVector;
//      std::copy( edLines.begin(), edLines.end(), std::back_inserter(edVector) );
//      phLine.m_editorLines = std::move(edVector);
//      m_physicalLines.emplace_back( std::move(phLine) );

//      m_numberOfEditorLines += static_cast<int>(edLines.size()); // Some more EditorLine

//    } else { // No wrap or the line fits perfectly within the wrap limits

//      EditorLine el(line);
//      PhysicalLine phLine(std::move(el));
//      m_physicalLines.emplace_back( std::move(phLine) );

//      ++m_numberOfEditorLines; // One more EditorLine
//      if (line.size() > m_maximumCharactersLine) // Check if this is the longest line found ever
//        m_maximumCharactersLine = line.size();
//    }

//  } while(line.isNull() == false);

//  qDebug() << "done";

  // At this point the PhysicalLines vector has been populated (or is still empty)
  // and the document structure is stored in memory

  //qDebug() << "Done recalculating document lines in " << timer.elapsed() << " milliseconds";
}

void Document::setCursorPos(int x, int y) {
  // This code needs to find, given a position into the document, a valid point where to set
  // the caret at

  if (m_plainTextLines.empty()) {

    // Empty doc
    m_viewportCursorPos.y = 0;
    m_viewportCursorPos.x = 0;

    m_documentCursorPos.pl = 0;
    m_documentCursorPos.el = 0;
    m_documentCursorPos.relativeEl = 0;
    m_documentCursorPos.ch = 0;
    m_documentCursorPos.relativeCh = 0;

    return;
  }

  // Find the editorLine this position corresponds to
  EditorLine *el = nullptr;
  int currentEL = 0;
  int currentPL = 0;
  int relativeEL = 0;
  for(auto& pl : m_physicalLines) {
    int elCount = (int)pl.m_editorLines.size();
    if (y >= currentEL + elCount) // Still not in the requested EL
      currentEL += elCount;
    else {
      // We'll arrive at the requested EL in this PL
      relativeEL = y - currentEL;
      el = &pl.m_editorLines[relativeEL];
      break;
    }
    ++currentPL;
  }

  if (!el) {
    --currentPL; // Fix out-of-loop values
    --currentEL;
    // Cannot validate or out of the document, set it to the last possible position
    m_viewportCursorPos.y = currentEL;
    m_viewportCursorPos.x = (int)m_physicalLines.back().m_editorLines.back().m_characters.size();

    m_documentCursorPos.pl = currentPL;
    m_documentCursorPos.el = currentEL;
    m_documentCursorPos.relativeEl = (int)(m_physicalLines.back().m_editorLines.size() - 1);

    // Calculate the ch count from the beginning of the Pl
    m_documentCursorPos.ch = std::accumulate( m_physicalLines.back().m_editorLines.begin(),
                                              m_physicalLines.back().m_editorLines.end(), 0,
        [](const int tot, EditorLine& edl) {
          return tot + (int)edl.m_characters.size();
    });

    m_documentCursorPos.relativeCh = m_viewportCursorPos.x;
    return;
  }

  // We found a valid EL, y is fine
  m_viewportCursorPos.y = y;

  // If the X coordinate is longer than the EL line itself, grab the minimum and drop the EOLs
  auto& vec = el->m_characters;
  int newXCoord = x;
  int EOLs = 0;
  if (vec.size() >= 1 && vec.back() == '\n') {
    ++EOLs;
    if (vec.size() >= 2 && vec[vec.size() - 2] == '\r')
      ++EOLs;
  }
  if (x >= vec.size() - EOLs)
    newXCoord = (int)vec.size() - EOLs; // Put the caret before the EOLs if we clicked where the EL can't reach
  else {
    // Into the EL, mind the tab-adjustment if we're right into one
    if (vec[x] == 0x07) {
      // Find the first tab of the series
      int first = x;
      while(first >= 0 && vec[first] == 0x07)
        --first;
      ++first;
      // Find the last tab of the series
      int last = x;
      while(last < vec.size() && vec[last] == 0x07)
        ++last;
      --last;

      int index = x - first; // index into the sequence

      // |_|_|_|_|  |_|_|_|_|
      //  ^ ^ * *
      //  | |
      //  these indices / 4 have decimal part less than 0.5 therefore the caret
      //  should be aligned to first + (int)(index/4)*4
      //  Similar reasoning for the last two (*)
      float res = index / 4.f;
      if (res < .5f)
        newXCoord = first + (int)(index / 4) * 4;
      else
        newXCoord = first + (int)(index / 4 + 1) * 4;
    }
  }

  // This is the position the user will see on the viewport grid
  m_viewportCursorPos.x = newXCoord;

  // Now calculate the internal position in the document (tabs and newlines will mess this up)
  m_documentCursorPos.pl = currentPL;
  m_documentCursorPos.el = currentEL;
  m_documentCursorPos.relativeEl = relativeEL;
  m_documentCursorPos.ch = std::accumulate( m_physicalLines[currentPL].m_editorLines.begin(),
                                            m_physicalLines[currentPL].m_editorLines.begin() + relativeEL, 0,
                            [](const int tot, EditorLine& edl) {
                              return tot + (int)edl.m_characters.size();
                        }) + newXCoord;
  m_documentCursorPos.relativeCh = newXCoord;
}

void Document::typeNewlineAtCursor() {
  if (m_plainTextLines.empty()) {
    // First digits, create a line we can write into
    m_plainTextLines.resize(1);
  }

  // Add newline at the current caret position
  QString& line = m_plainTextLines[m_documentCursorPos.pl];
  QString restOfLine = line.right(line.length() - m_documentCursorPos.ch);

  // Remove the rest from the physical line
  line.remove(m_documentCursorPos.ch, restOfLine.size());
  line += '\n';

  // Add another PL and stitch the rest of the split in there
  m_plainTextLines.insert(m_plainTextLines.begin() + m_documentCursorPos.pl + 1, std::move(restOfLine));

  // Advance the caret to the next PL
  ++(m_viewportCursorPos.y);
  m_viewportCursorPos.x = 0;
  ++(m_documentCursorPos.pl);
  ++(m_documentCursorPos.el);
  m_documentCursorPos.relativeEl = 0; // Obviously if the line did fit with the current wrap, a split will create
                                      // another line which fits the wrap
  m_documentCursorPos.ch = 0;
  m_documentCursorPos.relativeCh = 0;

  ++m_numberOfEditorLines;
}

void Document::typeAtCursor(QString keyStr) {

  if (keyStr.isEmpty())
    return; // Nothing to be done (unrecognized keystroke?)

  if (m_plainTextLines.empty()) {
    // First digits, create a line we can write into
    m_plainTextLines.resize(1);
  }

  // Add text at the current caret position
  QString& line = m_plainTextLines[m_documentCursorPos.pl];

  line.insert(m_documentCursorPos.ch, keyStr);

  // Advance the caret since we inserted text
  ++(m_viewportCursorPos.x);
  ++(m_documentCursorPos.ch);
  ++(m_documentCursorPos.relativeCh);
}


/*
 *
 * The class Document represents a grid for a text file
 *
 * It provides a vector of PhysicalLine objects (the physical ones) that can include one or more multiple
 * EditorLine objects (the fake ones due to wrap). A PhysicalLine has two values: an int beginValue (i.e. the
 * starting pixel line where the line begins to be rendered in a virtual document) and an int endValue (after
 * all the EditorLines have been rendered). fontMetrics() and m_wrapWidth are the values necessary to calculate
 * this grid.
 *
 * document_is_loaded()
 *  1) From its extension (e.g. cpp) a Lexer is chosen (or no lexer at all). If chosen it is lexed.
 *
 *  2) PhysicalLine objects are created, each one with ONE or MORE EditorLine. An EditorLine contains a list of words
 *     and their associated style (bold/italic/color/etc..) due to how the Lexer interpreted this.
 *
 *  3) PhysicalLine objects also have a beginValue and endValue (to facilitate queries when something is clicked inside
 *     the rendered region). After a click was received, a search for the character (or no-character in the line) is issued
 *     so the Document must have something like
 *       clickOnPoint(Point, hint LinesFrom, hint LinesTo (these hints are given since the viewport displays only a few lines) ->
 *         return the new click position
 *       doubleClickOnPoint (..) -> return the start-end positions for a selection (or a list of words in the entire document for
 *                                  similar selections)
 *       startDragSelection ().. etc.
 *
 *
 */


// Copy the characters for this editor line into our characters vector
EditorLine::EditorLine(QString str) {
  if (str.isEmpty())
    return;
  m_characters.resize(str.length());
  std::copy(str.begin(), str.end(), m_characters.begin());
}
