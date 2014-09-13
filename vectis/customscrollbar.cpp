#include "customscrollbar.h"
#include <QPainter>
#include <QTextBlock>
#include <QTextLayout>
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
    QSize sz = event->size();
    sz.setWidth(sz.width()-4);
    resize(sz);
    move(static_cast<QWidget*>(parent())->width() - width(), 1);
    qDebug() << event->oldSize() << " and " ;
    QScrollBar::resizeEvent(event);
}

void CustomScrollBar::paintEvent( QPaintEvent * event ) {

    QPainter p( this );
    QRect rc( 0, 0, rect().width() - 1, rect().height() - 1 );

    // Draw any scroll background - nota che per la trasparenza devi specificare come si blenda col background
    p.setCompositionMode (QPainter::CompositionMode_Source);
    p.fillRect( rc, QColor( 255, 255, 255, 50 ) );
    p.setCompositionMode (QPainter::CompositionMode_SourceOver);


    // Cerco il numero di righe visibili per il controllo testo resizato così com'è (22)

    // Hierarchy used to find the parent QPlainTextEdit widget
    // QScrollBar >parent> qt_scrollarea_vcontainer >parent> QPlainTextEdit
    // As long as we don't insert frames or tables, blocks == lines
    QTextBlock block = m_parent->document()->findBlockByNumber(0);
    QTextLayout *layout = block.layout(); // Layout di una riga
    QTextLine textLine = layout->lineAt(0);
    int maxNumLines = floor(qreal(m_parent->height()) / textLine.height());
    //int additionalEmptyBottomLines = maxNumLines - 1;
    int currentLinesNum = m_parent->document()->blockCount();
    int extraBottomLines = maxNumLines - 1;

    // Calcolo le linee PRIMA della view che scorre sul documento e le linee DOPO la view

    int beforeWeight = value();
    int afterWeight = ((currentLinesNum + (extraBottomLines)) - beforeWeight) - maxNumLines;

    //if(currentLinesNum <= maxNumLines)
    //    extraBottomLines = abs(maxNumLines - ((maxNumLines - 1) + currentLinesNum));
    qDebug() << "Before il peso è: " << beforeWeight << " after invece è " << afterWeight;


    // Cerco la posizione centrale dello slider usando i due valori come pesi


    float viewRelativePos = float(maxNumLines) -
            (float(maxNumLines)+float(afterWeight)*0.5f - 0-float(beforeWeight)*0.5f)/2.0f;

    // TODO: se questo valore è minore di zero, piazzalo a zero
    // e quanto più si avvicina a maxNumLines, tanto meno deve essere incrementato. Dev'essere un limite!
    if(viewRelativePos < 0)
        viewRelativePos = 0;

    // rect().height() : x = maxNumLines : viewRelativePos
    float rectAbsPos = (float(rect().height())*viewRelativePos)/float(maxNumLines);

    qDebug() << "maxNumLines is " << maxNumLines << " and viewRelativePos is = " << viewRelativePos <<
                " rectAbsPos = " << rectAbsPos;
    QRect rcSlider(0, rectAbsPos, rect().width() - 1, 5 );
    p.fillRect( rcSlider, QColor( 55, 4, 255, 100 ) );

    /*





    //max-min : x = maxnumlines : value()

    float sliderSize = 0.5f;
    if(currentLinesNum <= maxNumLines)
        sliderSize = 1.0f / (float(currentLinesNum + maxNumLines - 1) / float(maxNumLines));

    float weightBefore = float((maximum()-minimum())*value()) / float(maxNumLines);
    sliderSize -= weightBefore;

    //qDebug() << "currentLinesNum = " << currentLinesNum << " maxNumLines = " << maxNumLines <<
    //            " weightBefore = " << weightBefore << " sliderSize = " << sliderSize << "\n";

    float pos = 0;
    // value() : max()-min() = x : control_height()
    QRect rcSlider(0, pos, rect().width() - 1, pos + (sliderSize*rect().height()));
    p.fillRect( rcSlider, QColor( 55, 4, 255, 100 ) );


    qDebug() << "maximum()-minimum() = " << maximum()-minimum() << " value() = " << value() <<
                " rect().height() = " << rect().height() << " pos = " << pos << "\n";

*/




    //QTextLine line = layout->lineAt(0);
    //qDebug() << line.height();
    //qDebug() << "control has rect: " << rc << "\n";
    //qDebug() << "control has lines: " << static_cast<QPlainTextEdit*>(this->parent())->document()->blockCount() <<
    //            " maximum " << maximum() << " minimum " << minimum() << " pos " << pos;
    //QRect rcSlider(0,pos, rect().width() - 1, pos+pageStep());
    //p.fillRect( rcSlider, QColor( 55, 4, 255, 100 ) );


/*    QPainter painter(this);


        QStyleOptionSlider option;
        initStyleOption(&option);

        option.subControls = QStyle::SC_All;


        style()->drawComplexControl(QStyle::CC_ScrollBar, &option, &painter, this);
*/
    //QScrollBar::paintEvent(event);
}

