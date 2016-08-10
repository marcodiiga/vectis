#ifndef CUSTOMCODEEDIT_H
#define CUSTOMCODEEDIT_H
#include <UI/CodeTextEdit/Document.h>
#include <UI/ScrollBar/ScrollBar.h>
#include <QPlainTextEdit>


// The code and text edit control (everything gets rendered to it)
class CodeTextEdit : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit CodeTextEdit(QWidget *parent = 0);

    QFont getMonospaceFont() const;
    void getScreenShot(QPixmap& map) const;
    QSizeF getDocumentDimensions() const;

private:
    QFont m_monospaceFont;
};

#endif // CUSTOMCODEEDIT_H
