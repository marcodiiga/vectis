#include "customscrollbar.h"
#include <QPainter>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextLayout>
#include <QDebug>
#include <QStyleOptionSlider>

CustomScrollBar::CustomScrollBar(QWidget *parent) :
    QScrollBar(parent)
{
    Q_ASSERT( parent );

    //setAttribute( Qt::WA_TranslucentBackground );
    setAttribute( Qt::WA_OpaquePaintEvent, false );
    setStyle(new CustomStyle());
}


void CustomScrollBar::paintEvent( QPaintEvent * event ) {


    /*
    QPainter p( this );
    QRect rc( 0, 0, rect().width() - 1, rect().height() - 1 );

    // Draw any scroll background
    p.setCompositionMode (QPainter::CompositionMode_Source);
    p.fillRect( rc, QColor( 255, 255, 255, 50 ) );
    p.setCompositionMode (QPainter::CompositionMode_SourceOver);

    int pos = 0;
    if(maximum()-minimum() > 0)
        pos = (value()*rect().height())/(maximum()-minimum());
    // value() : max()-min() = x : control_height()

    // QScrollBar >parent> qt_scrollarea_vcontainer >parent> QPlainTextEdit
    QTextBlock block = static_cast<QPlainTextEdit*>(this->parent()->parent())->document()->findBlockByNumber(1);
    QTextLayout *layout = block.layout();
    //QTextLine line = layout->lineAt(0);
    //qDebug() << line.height();
    qDebug() << "control has rect: " << rc << "\n";
    //qDebug() << "control has lines: " << static_cast<QPlainTextEdit*>(this->parent())->document()->blockCount() <<
    //            " maximum " << maximum() << " minimum " << minimum() << " pos " << pos;
    QRect rcSlider(0,pos, rect().width() - 1, pos+pageStep());
    p.fillRect( rcSlider, QColor( 55, 4, 255, 100 ) );
*/

    QPainter painter(this);


        QStyleOptionSlider option;
        initStyleOption(&option);

        option.subControls = QStyle::SC_All;


        style()->drawComplexControl(QStyle::CC_ScrollBar, &option, &painter, this);

    //QScrollBar::paintEvent(event);
}

