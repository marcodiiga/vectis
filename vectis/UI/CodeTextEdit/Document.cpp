#include <UI/CodeTextEdit/Document.h>
#include <UI/CodeTextEdit/CodeTextEdit.h>
#include <UI/CodeTextEdit/Lexers/Lexer.h>
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include <deque>

#include <QDebug>

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
  m_plainText = in.readAll();
  f.close();

  // This is also necessary: normalize all line endings to \n (Unix-style)
  QRegExp invalidEnding("\r\n|\r");
  m_plainText.replace(invalidEnding, "\n");
  // For simplicity convert all tabs into 4 spaces and just deal with those
  QRegExp tabs("\t");
  m_plainText.replace(tabs, "    ");

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


void Document::recalculateDocumentLines () {
  //qDebug() << "Recalculating document lines..";
  m_firstDocumentRecalculate = false;

  if (m_needReLexing) {
    m_lexer->lexInput(m_plainText.toStdString(), m_styleDb);
    m_needReLexing = false;
  }

  // Drop previous lines
  m_physicalLines.clear();
  m_numberOfEditorLines = 0;
  m_maximumCharactersLine = 0;

  // Scan each line (until a newline is found)
  QTextStream ss(&m_plainText);
  QString line;
  do {
    line = ss.readLine();
    if (line.isNull())
      continue;

    if (m_wrapWidth != -1 && // Also check if the monospace'd width isn't exceeding the viewport
        line.count() * m_codeTextEdit.getCharacterWidthPixels() > m_wrapWidth) {
      // We have a wrap and the line is too big - WRAP IT

      std::vector<EditorLine> edLines;
      QString restOfLine = line;

      // Calculate the allowed number of characters per editor line
      int maxChars = static_cast<int>( m_wrapWidth / m_codeTextEdit.getCharacterWidthPixels() );
      if (maxChars < 5)
        maxChars = 5; // Keep it to a minimum

      // Start the wrap-splitting algorithm or resort to a brute-force character splitting one if
      // no suitable spaces could be found to split the line
      while(restOfLine.size() > maxChars) {

        int bestSplittingPointFound = -1;
        for(int i = 0; i < restOfLine.count(); ++i) {
          if (i > maxChars)
            break; // We couldn't find a suitable space split point for restOfLine
          if ( restOfLine[i] == ' ' )
            bestSplittingPointFound = i;
        }

        if (bestSplittingPointFound != -1) { // We found a suitable space point to split this line
          edLines.push_back( restOfLine.left( bestSplittingPointFound ));
          restOfLine = restOfLine.right( restOfLine.size() - bestSplittingPointFound );
        } else {
          // No space found, brutally split characters
          edLines.push_back( restOfLine.left( maxChars ));
          restOfLine = restOfLine.right( restOfLine.size() - maxChars );
        }
      }
      edLines.push_back( restOfLine ); // Insert the last part and proceed

      for(auto& ll : edLines)
        if (ll.m_characters.size() > m_maximumCharactersLine) // Update if this is the longest line ever found
          m_maximumCharactersLine = static_cast<int>(ll.m_characters.size());

      // No need to do anything special for tabs - they're automatically converted into spaces
      PhysicalLine phLine;
      std::vector<EditorLine> edVector;
      std::copy( edLines.begin(), edLines.end(), std::back_inserter(edVector) );
      phLine.m_editorLines = std::move(edVector);
      m_physicalLines.emplace_back( std::move(phLine) );

      m_numberOfEditorLines += static_cast<int>(edLines.size()); // Some more EditorLine

//      // Try to find a space from the right to the left WITHIN the limit. If found - split the line there
//      for(int i = line.count()-1; i >= 0; --i) {
//        if ( line[i] == ' ' && i <= splitPos) {
//          splitPointFound = true;
//          edLines.emplace_back(line.left(i));
//          edLines.emplace_back(line.right(line.count() - i));
//          break;
//        }
//      }
//      if (splitPointFound == false) {
//        // No space found, brutally split characters
//        edLines.emplace_back( line.left( splitPos) );
//        edLines.emplace_back( line.right( line.count() - splitPos) );
//      }

//      if (edLines[0].m_characters.size() > m_maximumCharactersLine) // Check if this is the longest line found ever
//        m_maximumCharactersLine = line.size();
//      if (edLines[1].m_characters.size() > m_maximumCharactersLine) // Check if this is the longest line found ever
//        m_maximumCharactersLine = line.size();

//      // No need to do anything special for tabs - they're automatically converted into spaces
//      PhysicalLine phLine;
//      phLine.m_editorLines = std::move(edLines);
//      m_physicalLines.emplace_back( std::move(phLine) );

//      m_numberOfEditorLines += 2; // Some more EditorLine

    } else { // No wrap or the line fits perfectly within the wrap limits

      EditorLine el(line);
      PhysicalLine phLine(std::move(el));
      m_physicalLines.emplace_back( std::move(phLine) );

      ++m_numberOfEditorLines; // One more EditorLine
      if (line.size() > m_maximumCharactersLine) // Check if this is the longest line found ever
        m_maximumCharactersLine = line.size();
    }

  } while(line.isNull() == false);

  // At this point the PhysicalLines vector has been populated (or is still empty)
  // and the document structure is stored in memory
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
  m_characters.resize(str.length());
  std::copy(str.begin(), str.end(), m_characters.begin());
}
