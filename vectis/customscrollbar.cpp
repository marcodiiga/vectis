#include "customscrollbar.h"
#include <QPainter>
#include <QTextBlock>
#include <QTextLayout>
#include <QtCore/qmath.h>
#include <QDebug>
#include <QStyleOptionSlider>

CustomScrollBar::CustomScrollBar(QPlainTextEdit *parent) :
    QScrollBar(parent),
    m_parent(parent)
{
    Q_ASSERT( parent );

    //setAttribute( Qt::WA_TranslucentBackground );
    setAttribute( Qt::WA_OpaquePaintEvent, false );
   // setStyle(new CustomStyle());
}

void CustomScrollBar::resizeEvent ( QResizeEvent * event ) {
    // Rende la scrollbar più piccola di 3 units e la posiziona attaccata al lato destro
    QSize sz = event->size();
    sz.setWidth(sz.width());
    resize(sz);
    move(static_cast<QWidget*>(parent())->width() - width(), 1);

    // Ricalcolo il numero di righe visibili per il controllo testo resizato così com'è

    // Hierarchy used to find the parent QPlainTextEdit widget
    // QScrollBar >parent> qt_scrollarea_vcontainer >parent> QPlainTextEdit
    // As long as we don't insert frames or tables, blocks == lines
    QTextBlock block = m_parent->document()->findBlockByNumber(0);
    QTextLayout *layout = block.layout(); // Layout di una riga
    QTextLine textLine = layout->lineAt(0);

    m_maxNumLines = qFloor(qreal(m_parent->height()) / textLine.height());
    qDebug() << "m_maxNumLines is now " << m_maxNumLines;

    QScrollBar::resizeEvent(event);
}

void CustomScrollBar::paintEvent( QPaintEvent * event ) {

    QPainter p( this );
    QRect rc( 0, 0, rect().width() - 1, rect().height() - 1 );

    // Draw any scroll background - nota che per la trasparenza devi specificare come si blenda col background
    p.setCompositionMode (QPainter::CompositionMode_Source);
    p.fillRect( rc, QColor( 255, 255, 255, 50 ) );
    p.setCompositionMode (QPainter::CompositionMode_SourceOver);

    // Calcolo la posizione dello slider

    int extraBottomLines = m_maxNumLines - 1; // Righe extra per scrollare il testo fino a visualizzare solo una riga
                                              // in alto

    // Dato che maximum() è SEMPRE maggiore di value() (il numero di linee del controllo è sempre
    // maggiore o uguale della prima riga visualizzata dalla view), posso esprimere il rapporto come
    // posizione_iniziale_slider = altezza_view * (riga_view / max_righe)
    float viewRelativePos = float(m_maxNumLines) * float(value()) / float(maximum() + extraBottomLines);

    // e ora trova la posizione assoluta nella rect del controllo
    // rect().height() : x = maxNumLines : viewRelativePos
    float rectAbsPos = (float(rect().height())*viewRelativePos)/float(m_maxNumLines);

    //qDebug() << "maxNumLines is " << maxNumLines << " and viewRelativePos is = " << viewRelativePos <<
    //            " rectAbsPos = " << rectAbsPos;

    // e ora calcolo la lunghezza della rect dello slider
    int currentLinesNum = m_parent->document()->blockCount();
    int lenSlider = int(float(rect().height()) * (float(m_maxNumLines) / float(currentLinesNum + extraBottomLines)));


    // imposta un minimo di lunghezza per lo slider ed evita che vada disegnato fuori
    if(lenSlider < 15)
        lenSlider = 15;

    if(rectAbsPos + lenSlider > rect().height())
        rectAbsPos -= (rectAbsPos + lenSlider) - rect().height();

    qDebug() << lenSlider;

    QRect rcSlider(0, rectAbsPos, rect().width() - 1, lenSlider );
    p.fillRect( rcSlider, QColor( 55, 4, 255, 100 ) );


/*    QPainter painter(this);


        QStyleOptionSlider option;
        initStyleOption(&option);

        option.subControls = QStyle::SC_All;


        style()->drawComplexControl(QStyle::CC_ScrollBar, &option, &painter, this);
*/
    //QScrollBar::paintEvent(event);
}

