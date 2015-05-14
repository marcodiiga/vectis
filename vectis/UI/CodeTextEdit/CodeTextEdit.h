#ifndef CUSTOMCODEEDIT_H
#define CUSTOMCODEEDIT_H
#include <UI/CodeTextEdit/Document.h>
#include <UI/ScrollBar/ScrollBar.h>
#include <QAbstractScrollArea>
#include <memory>

// The code and text edit control (everything gets rendered to it)
class CodeTextEdit : public QAbstractScrollArea {
    Q_OBJECT
public:
    explicit CodeTextEdit(QWidget *parent = 0);

    void loadDocument(Document *doc);
    int getViewportWidth() const;
    int getCharacterWidthPixels() const;

private:
    void paintEvent(QPaintEvent *event);

    // To have wrapping work, the resize event will be forwarded to the loaded
    // document
    void resizeEvent(QResizeEvent *evt);

    QFont m_monospaceFont;
    int m_characterWidthPixels;
    Document *m_document;
    std::unique_ptr<ScrollBar>    m_verticalScrollBar;
};

#endif // CUSTOMCODEEDIT_H
