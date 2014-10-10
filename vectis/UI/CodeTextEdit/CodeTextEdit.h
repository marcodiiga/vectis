#ifndef CUSTOMCODEEDIT_H
#define CUSTOMCODEEDIT_H

#include <QTextEdit>

class CodeTextEdit : public QTextEdit {
    Q_OBJECT
public:
    explicit CodeTextEdit(QWidget *parent = 0);
};

#endif // CUSTOMCODEEDIT_H
