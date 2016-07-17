#ifndef CUSTOMCODEEDIT_H
#define CUSTOMCODEEDIT_H
#include <UI/CodeTextEdit/Document.h>
#include <UI/ScrollBar/ScrollBar.h>
#include <QTextEdit>


// The code and text edit control (everything gets rendered to it)
class CodeTextEdit : public QTextEdit {
    Q_OBJECT
public:
    explicit CodeTextEdit(QWidget *parent = 0);

    QFont m_monospaceFont;
};

#endif // CUSTOMCODEEDIT_H
