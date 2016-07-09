#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <UI/CodeTextEdit/Lexers/Lexer.h>
#include <QObject>
#include <utility>
#include <memory>
#include <vector>

class CodeTextEdit;

// Three classes help to organize a document to be displayed and are set up by a document
// recalculate operation:
//
// PhysicalLine {
//   A physical line corresponds to a real file line (until a newline character)
//   it should have 0 (only a newline) or more EditorLine
//
//   EditorLine {
//     An editor line is a line for the editor, i.e. a line that might be the result
//     of wrapping or be equivalent to a physical line. EditorLine stores the characters
//   }
// }

struct EditorLine {
  EditorLine (QString str);

  std::vector<QChar> m_characters;
};

struct PhysicalLine {
  PhysicalLine (EditorLine&& editorLine) {
    m_editorLines.emplace_back(std::forward<EditorLine>(editorLine));
  }
  PhysicalLine () = default;

  std::vector<EditorLine> m_editorLines;
};

enum SyntaxHighlight { NONE, CPP };

// This class represents document loaded from the CodeTextEdit control.
// A text document is treated as a grid of rectangles (the monospaced characters)
// and a text 'physical' line might include one or more editor lines due to wrap
// factors.
class Document : public QObject {
    Q_OBJECT
public:
    explicit Document(const CodeTextEdit& codeTextEdit);

    bool loadFromFile (QString file);
    void applySyntaxHighlight(SyntaxHighlight s);
    void setWrapWidth(int width);    

private:
    friend class CodeTextEdit;
    friend class RenderingThread;

    void recalculateDocumentLines();
    // Qt hasn't a reliable way to detect whether all widgets have reached their stable
    // dimension (i.e. all resize() have been triggered), thus we delay syntax highlighting
    // and other expensive operations until the last resize() has been triggered
    bool m_firstDocumentRecalculate;    

    std::vector<QString> m_plainTextLines;
    const CodeTextEdit& m_codeTextEdit;

    // Variables related to how the control renders lines
    int m_characterWidthPixels;
    int m_wrapWidth;
    int m_numberOfEditorLines;
    int m_maximumCharactersLine; // According to wrapWidth

    std::unique_ptr<LexerBase> m_lexer;
    bool m_needReLexing; // Whether the document needs re-lexing
    std::vector<PhysicalLine> m_physicalLines;
    StyleDatabase m_styleDb;

    // Variable document-specific
    struct {
      int x = 0;
      int y = 0;
    } m_cursorPos; // Latest known cursor position

    int m_storeSliderPos = -1; // This variable is used to store the slider position when this document
                               // is either switched off or put on hold
};

#endif // DOCUMENT_H
