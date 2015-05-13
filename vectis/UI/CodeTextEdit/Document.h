#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>

// This class represents document loaded from the CodeTextEdit control.
// A text document is treated as a grid of rectangles (the monospaced characters)
// and a text 'physical' line might include one or more editor lines due to wrap
// factors.
class Document : public QObject {
    Q_OBJECT
public:
    explicit Document();

private:

    //std::unique_ptr<ScrollBar>    m_verticalScrollBar;
};

#endif // DOCUMENT_H
