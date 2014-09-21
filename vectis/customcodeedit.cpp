#include "customcodeedit.h"

#include <QScrollBar>
#include <QDebug>

CustomCodeEdit::CustomCodeEdit(QWidget *parent) :
    QTextEdit(parent)
{
    // Implementa un timer per lo scroll smooth
    m_scrollTimer.reset(new QTimer(this)); // C++11 non ha make_unique, sarà aggiunto a C++14
    //connect(m_scrollTimer.get(), SIGNAL(timeout()), this, SLOT(timerScroll()));
}

// Implementa uno step di animazione per lo scroll
void CustomCodeEdit::timerScroll() {
    QPoint valueToReach = m_scrollTimer->property("valueToReach").toPoint();
    if(valueToReach == QPoint(0,0)) { // Ciclo completato
        m_scrollTimer->stop();
        qDebug() << "scrollTimer has finished!";
        return;
    }
    QPoint increments(0,0);
    if(valueToReach.x() != 0) {
        if(valueToReach.x() > 0)
            increments.setX(-1);
        else
            increments.setX(1);
    }
    if(valueToReach.y() != 0) {
        if(valueToReach.y() > 0)
            increments.setY(-1);
        else
            increments.setY(1);
    }
    m_scrollTimer->setProperty("valueToReach", valueToReach + increments);
    qDebug() << "scrollTimer has calculated increments of " << increments;
    QTextEdit::scrollContentsBy(increments.x(),increments.y());
}

void CustomCodeEdit::scrollContentsBy(int dx, int dy) {
    // Invece di chiamare subito QTextEdit::scrollContentsBy(dx,dy);
    // implementa uno smooth scrolling con un timer
    //m_scrollTimer->setProperty("valueToReach", QPoint(dx,dy));
    //qDebug() << "starting timer to reach " << QPoint(dx,dy);
    //m_scrollTimer->start(1000);
    QTextEdit::scrollContentsBy(dx,dy);
}

