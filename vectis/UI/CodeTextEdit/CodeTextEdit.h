#ifndef CUSTOMCODEEDIT_H
#define CUSTOMCODEEDIT_H
#include <UI/ScrollBar/ScrollBar.h>
#include <QAbstractScrollArea>
#include <memory>

// The code and text edit control (everything gets rendered to it)
class CodeTextEdit : public QAbstractScrollArea {
    Q_OBJECT
public:
    explicit CodeTextEdit(QWidget *parent = 0);

    void paintEvent(QPaintEvent *);

private:
    std::unique_ptr<ScrollBar>    m_verticalScrollBar;
};

#endif // CUSTOMCODEEDIT_H
