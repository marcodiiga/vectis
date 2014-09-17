#include "customcodeedit.h"

#include <QScrollBar>
#include <QDebug>

CustomCodeEdit::CustomCodeEdit(QWidget *parent) :
    QTextEdit(parent)
{
    m_scrollTimer = new QTimer(this);
    //connect(m_scrollTimer, SIGNAL(timeout()), this, SLOT(scrollToStep()));
    connect(this, SIGNAL(textChanged()), this, SLOT(updateScrollBar()));
}

void CustomCodeEdit::updateScrollBar() {
    this->verticalScrollBar()->update();
}

/*
void CustomCodeEdit::scrollContentsBy ( int dx, int dy ) {
    qDebug() << "asked scroll di (" << dx << "," << dy << ")";

    m_scrollDv.dx = dx;
    m_scrollDv.dy = dy;
    //m_scrollTimer->start(1000);
    QTextEdit::scrollContentsBy(dx,dy-50);
}
*/

void CustomCodeEdit::scrollToStep() {
    /*qDebug() << "Timer Scroll di (" << m_scrollDv.dx << "," << m_scrollDv.dy << ")";
    if(m_scrollDv.dx > 0)
        m_scrollDv.dx--;
    else if(m_scrollDv.dx < 0)
        m_scrollDv.dx++;
    if(m_scrollDv.dy > 0)
        m_scrollDv.dy--;
    else if(m_scrollDv.dy < 0)
        m_scrollDv.dy++;
    QTextEdit::scrollContentsBy(m_scrollDv.dx, m_scrollDv.dy);
    if(m_scrollDv.dx == 0 && m_scrollDv.dy == 0)
        m_scrollTimer->stop();
    update();*/
}
