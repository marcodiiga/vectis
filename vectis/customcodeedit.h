#ifndef CUSTOMCODEEDIT_H
#define CUSTOMCODEEDIT_H

#include <QTextEdit>
#include <QTimer>

class CustomCodeEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit CustomCodeEdit(QWidget *parent = 0);

private:
    //void scrollContentsBy ( int dx, int dy );

    QTimer *m_scrollTimer;
    struct scrollDv { int dx; int dy; } m_scrollDv;

private slots:
    void scrollToStep();
    void updateScrollBar();

signals:

public slots:

};

#endif // CUSTOMCODEEDIT_H
