#include <UI/CodeTextEdit/Document.h>
#include <UI/CodeTextEdit/CodeTextEdit.h>
#include <UI/CodeTextEdit/Lexers/Lexer.h>
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include <QtConcurrent>
#include <functional>

#include <QDebug>
#include <QElapsedTimer>

Document::Document(const CodeTextEdit& codeTextEdit) :
  m_codeTextEdit(codeTextEdit),
  m_wrapWidth(-1),
  m_needReLexing(false),
  m_firstDocumentRecalculate(true)
{  
  //qDebug() << m_codeTextEdit.fontMetrics().maxWidth() << " " << m_codeTextEdit.getViewportWidth();
}

// The following function loads the contents of a text file into memory.
// This is a memory-expensive operation but documents need to be available at any time
// Returns true on success
bool Document::loadFromFile(QString file) {

  QFile f(file);
  if (f.open(QFile::ReadWrite | QFile::Text) == false)
    return false;

  QTextStream in(&f);
  // Load the entire file into memory (expensive but necessary)
  QString line;
  while(!in.atEnd()) {
    line = in.readLine();

    // This is also necessary: normalize all line endings to \n (Unix-style)
    QRegExp invalidEnding("\r\n|\r");
    line.replace(invalidEnding, "\n");
    // For simplicity convert all tabs into 4 spaces and just deal with those
    QRegExp tabs("\t");
    line.replace(tabs, "    ");

    m_plainTextLines.push_back(std::move(line));
  }
  f.close();

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

void Document::recalculateDocumentLines () {

  //QElapsedTimer timer;
  //timer.start();

  m_firstDocumentRecalculate = false;

  if (m_needReLexing) {
    QString m_plainText;
    for(auto& line : m_plainTextLines) { // Expensive, hopefully this doesn't happen too often - LEX DIRECTLY FROM VECTOR
      m_plainText.append(line);
      m_plainText += '\n';
    }
    m_lexer->lexInput(std::move(m_plainText.toStdString()), m_styleDb);
    m_needReLexing = false;
  }

  // Drop previous lines
  m_physicalLines.clear();
  m_numberOfEditorLines = 0;
  m_maximumCharactersLine = 0;


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
    std::for_each(pl[0].m_editorLines.begin(), pl[0].m_editorLines.end(), [this](const EditorLine& eline) {
      int lineLength = static_cast<int>(eline.m_characters.size());
      if (lineLength > m_maximumCharactersLine) // Check if this is the longest line found ever
        m_maximumCharactersLine = lineLength;
    });

  };

  m_physicalLines =
      QtConcurrent::blockingMappedReduced<std::vector<PhysicalLine>>(m_plainTextLines.begin(), m_plainTextLines.end(),
                                                                     mapFn, reduceFn, QtConcurrent::SequentialReduce );

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


/*
 *
 * Document represents a grid for a text file
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
