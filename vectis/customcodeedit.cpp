#include "customcodeedit.h"

#include <QScrollBar>
#include <QDebug>

CustomCodeEdit::CustomCodeEdit(QWidget *parent) :
    QTextEdit(parent)
{
}

void CustomCodeEdit::scrollContentsBy(int dx, int dy) {
    qDebug() << dx << dy;
    //TODO
}

