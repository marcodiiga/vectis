#include "customscrollbar.h"
#include <QPainter>
#include <QTextBlock>
#include <QTextLayout>
#include <QtCore/qmath.h>
#include <QAbstractTextDocumentLayout>
#include <QDebug>
#include <QStyleOptionSlider>




CustomScrollBar::CustomScrollBar ( QTextEdit * parent ) :
    QScrollBar(parent),
    m_parent(parent),
    m_textLineHeight(1),
    m_internalLineCount(1),
    m_sliderIsBeingDragged(false)
{
    Q_ASSERT( parent );

    //setAttribute( Qt::WA_TranslucentBackground );
    setAttribute( Qt::WA_OpaquePaintEvent, false );

    connect(m_parent->document()->documentLayout(), SIGNAL(documentSizeChanged(const QSizeF&)),
            this, SLOT(documentSizeChanged(const QSizeF&)));
    connect(this, SIGNAL(actionTriggered(int)), this, SLOT(actionTriggered(int)));
    connect(this, SIGNAL(sliderPressed()), this, SLOT(sliderPressed()));
    connect(this, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));

    m_scrollAnim.reset(new QPropertyAnimation(this, "value"));
    m_pgKeyEater.reset(new PgKeyEater(this));

    // Installa il PageDown/PageUp key eater per il controllo parent
    m_parent->installEventFilter(m_pgKeyEater.get());
    connect(m_scrollAnim.get(), SIGNAL(finished()), this, SLOT(moveParentCaret()));
    //m_scrollTimer = new QTimer(this);
    //connect(m_scrollTimer, SIGNAL(timeout()), this, SLOT(timerScroll()));
    //connect(this, SIGNAL(valueChanged(int)), this, SLOT(scrollToValue(int)));


}

CustomScrollBar::~CustomScrollBar() {
    m_parent->removeEventFilter(m_pgKeyEater.get());
}

namespace {
    // Un piccolo template per ritornare -1 o 1 a seconda del segno di un valore
    template <typename T> int sgn(T val) {
        return (T(0) < val) - (val < T(0));
    }
    // Restituisce il clamped value di un intero
    int clamp(int val, int min, int max) {
        if(val < min)
            return min;
        if(val > max)
            return max;
        return val;
    }
}

// This happens when the slider is being dragged by the user
void CustomScrollBar::sliderReleased () {
    m_sliderIsBeingDragged = false;
}
void CustomScrollBar::sliderPressed () {
    m_sliderIsBeingDragged = true;
}

// Sposta il caret (cursore) del parent alla posizione raggiunta dalla scrollbar.
// Utile quando è stato chiamato un PageUp/PageDown
void CustomScrollBar::moveParentCaret() {
    int pageValue = m_scrollAnim->property("pageStepEvent").toInt();
    if(pageValue > 0) { // PageDown o PageUp è stato premuto
        QTextCursor cursor = m_parent->textCursor();
        qDebug() << "value() / m_textLineHeight " << value() / m_textLineHeight;
        cursor.movePosition( QTextCursor::Start, QTextCursor::MoveAnchor );
        cursor.movePosition( QTextCursor::NextBlock, QTextCursor::MoveAnchor, value() / m_textLineHeight);
        m_parent->setTextCursor(cursor);
        m_scrollAnim->setProperty("pageStepEvent", 0);
    }
}

// This happens whenever the slider is moved (mouse scrollbar / page down-up / dragging)
void CustomScrollBar::actionTriggered ( int action ) {
//    qDebug() << "actionTriggered received action << " << action << " m_sliderIsBeingDragged = " << m_sliderIsBeingDragged <<
//                ", al momento del segnale value mi risulta: " << value();
    if(action == QAbstractSlider::SliderNoAction || m_sliderIsBeingDragged)
        return;

    if(m_scrollAnim->state() == QAbstractAnimation::State::Running)
        m_scrollAnim->stop();
    // Se l'animazione è stata triggerata da un PageDown/PageUp sarà necessario muovere il caret alla fine
    //if(action == SliderPageStepAdd || action == SliderPageStepSub)
    //    m_scrollAnim->setProperty("pageStepEvent", true);
    m_scrollAnim->setDuration(120);
    m_scrollAnim->setStartValue(value());
    m_scrollAnim->setEndValue(sliderPosition());

    m_scrollAnim->start();
}

//    //disconnect(m_parent, SIGNAL())

//    // Lo slider si trova in value(), devo scrollarlo verso sliderPosition() in una serie di step
//    // quindi mi calcolo l'incremento da applicare ad ogni step
//    m_scrollTimer->setProperty("sliderDesiredPosition", sliderPosition());
//    m_scrollTimer->setProperty("ignoreSliderUpdate", true);
//    m_scrollTimer->setProperty("storedPosition", value());
//    int incrementToBeCovered = sliderPosition() - value();

//    // Nessun movimento immediato nè pianificato, il timer se ne occuperà
//    //setValue(value());
//    setSliderPosition(value());


//    int sliderIncrement = incrementToBeCovered / 15;
//    if(sliderIncrement == 0)
//        sliderIncrement = sgn(incrementToBeCovered);
//    m_scrollTimer->setProperty("sliderIncrement", sliderIncrement);
//    m_scrollTimer->setProperty("sliderValueRecorded", value());
//    //qDebug() << "scroll action detected - cur pos: " << value() << " desired pos: " << sliderPosition();

//    m_scrollTimer->start(50);
//}

//void CustomScrollBar::timerScroll() {
//    // L'area di scroll ha un range [0, maximum()] quindi il timer
//    // va terminato quando sfora questi limiti
//    int sliderDesiredPosition = m_scrollTimer->property("sliderDesiredPosition").toInt();
//    int sliderIncrement = m_scrollTimer->property("sliderIncrement").toInt();
//    qDebug() << "[TIMER] reaching " << sliderDesiredPosition;
//    //qDebug() << "value() " << value() << " sliderDesiredPosition " << sliderDesiredPosition << " maximum() " << maximum()
//     //        << " sliderIncrement: " << sliderIncrement;
//    if( (value() <= 0 && sliderDesiredPosition < 0) || // Slider al minimo e abbiamo chiesto fuori range
//        (value() >= maximum() && sliderDesiredPosition > maximum())) // Slider al massimo e abbiamo chiesto fuori range
//    {
//        m_scrollTimer->stop(); // Abbiamo raggiunto il punto che volevamo
//        setSliderPosition(value());
//        qDebug() << ">TIMER FINISHED<";
//    }
//    else {
//        // Se abbiamo raggiunto il range/il valore preciso il nostro compito è finito
//        if(sliderDesiredPosition + abs(sliderIncrement) >= value() &&
//                value() >= sliderDesiredPosition - abs(sliderIncrement)) {
//            m_scrollTimer->stop(); // Abbiamo raggiunto il punto che volevamo
//            m_scrollTimer->setProperty("ignoreSliderUpdate", false);
//            setValue(clamp(value(), 0, maximum()));
//            m_scrollTimer->setProperty("storedPosition", value());
//            setSliderPosition(value());
//            qDebug() << ">TIMER FINISHED - range reached - <";
//        } else {
//            //qDebug() << " incrementando value a: " << value()+sliderIncrement;
//            m_scrollTimer->setProperty("ignoreSliderUpdate", false);
//            setValue(value() + sliderIncrement);
//            m_scrollTimer->setProperty("storedPosition", value());
//            m_scrollTimer->setProperty("ignoreSliderUpdate", true);
//        }
//    }
//}

// Quando il controllo viene resizato, si aggiorna anche il numero massimo di righe che possiamo visualizzare
// entro la view
void CustomScrollBar::resizeEvent ( QResizeEvent * event ) {
   qDebug() << "TODO: resizeEvent per la ScrollBar, è utile il setMaximum qua? Se basta il docSizeChange rimuovilo";
   // setMaximum( m_internalLineCount * m_textLineHeight );
    //qDebug() << "resizeEvent: maximum aggiornato a: " << maximum();


    //qDebug() << "m_maxNumLines is now " << m_maxNumLines;

    //qDebug() << "textLine.height() is  " << m_textLineHeight;

    QScrollBar::resizeEvent(event);
}

void CustomScrollBar::sliderChange ( SliderChange change ) {


    /*if(m_scrollTimer->isActive()) {
        qDebug() << "sliderValueChange ricevuto ma bailing out immediatamente causa timer";
        int sliderValueRecorded = m_scrollTimer->property("sliderValueRecorded").toInt();
        setValue(sliderValueRecorded);
        return;
    }*/
    //qDebug() << "sliderValueChange ricevuto e value == " << value();

    // Per poter simulare delle "righe vuote virtuali" alla fine e permettere di scrollare
    // l'ultima riga fino all'inizio della view è necessario rilevare quando cambia il valore dello
    // slider (e.g. sto scrollando o aggiungo/tolgo righe oppure wrappo) e aumentare il massimo dove si può scrollare.
    // Non posso fare un semplice m_internalLineCount = (m_parent->document()->lineCount() - 1);
    // perchè non è sempre valido e in wrap mode crea problemi
    setMaximum( (m_internalLineCount - 1) * m_textLineHeight );
    //qDebug() << "sliderValueChange dopo setMaximum ricevuto e value == " << value();
    QAbstractSlider::sliderChange(change);

    //qDebug() << "sliderValueChange dopo base class call ricevuto e value == " << value();
}


// Emesso quando il documento cambia size, è l'unico modo per rilevare il numero delle linee del documento con il wrapping
void CustomScrollBar::documentSizeChanged(const QSizeF & newSize) {

    // Hierarchy used to find the parent QPlainTextEdit widget
    // QScrollBar >parent> qt_scrollarea_vcontainer >parent> QPlainTextEdit
    // As long as we don't insert frames or tables, blocks == lines
    QTextBlock block = m_parent->document()->findBlockByNumber(0);
    QTextLayout *layout = block.layout(); // Layout di una riga
    QTextLine textLine = layout->lineAt(0);
    m_textLineHeight = textLine.height();
    m_maxNumLines = qFloor(qreal(m_parent->height()) / m_textLineHeight);

    // Aggiorna anche il maximum per permettere di scrollare l'ultima riga fino all'inizio della view
    m_internalLineCount = int(newSize.height() / m_textLineHeight);
    setMaximum( (m_internalLineCount - 1)* m_textLineHeight );

    //qDebug() << "m_textLineHeight "  << m_textLineHeight << " m_maxNumLines " << m_maxNumLines << " m_internalLineCount " << m_internalLineCount;
}

void CustomScrollBar::paintEvent ( QPaintEvent * event ) {

    QPainter p( this );

    /*QPlainTextEdit ea(m_parent); // TODO: come cazzo si può rilevare il numero di righe se ho il word wrapping????
    int lineCount = ea.document()->lineCount();
    if(m_internalLineCount != lineCount) {
        qDebug() << lineCount;
        m_internalLineCount = lineCount;
        setMaximum( m_internalLineCount * m_textLineHeight );
    }*/

    // Draw any scroll background - nota che per la trasparenza devi specificare come si blenda col background
    //p.setCompositionMode (QPainter::CompositionMode_Source);
    //p.fillRect( rc, QColor( 255, 255, 255, 50 ) );
    //p.setCompositionMode (QPainter::CompositionMode_SourceOver);

    // Calcolo la posizione dello slider
    //qDebug() << "value : " << value() << "maximum() :" << maximum();

    int extraBottomLines = (m_maxNumLines - 1); // Righe extra per scrollare il testo fino a visualizzare solo una riga
                                                // in alto

    // Dato che maximum() è SEMPRE maggiore di value() (il numero di linee del controllo è sempre
    // maggiore o uguale della prima riga visualizzata dalla view), posso esprimere il rapporto come
    // posizione_iniziale_slider = altezza_view * (riga_view / max_righe)
    float viewRelativePos = float(m_maxNumLines) * (float(value()) / float(maximum() + (extraBottomLines*m_textLineHeight)));


    // e ora trova la posizione assoluta nella rect del controllo
    // rect().height() : x = maxNumLines : viewRelativePos
    float rectAbsPos = (float(rect().height()) * viewRelativePos) / float(m_maxNumLines);

    //qDebug() << value();

    //qDebug() << "maxNumLines is " << maxNumLines << " and viewRelativePos is = " << viewRelativePos <<
    //            " rectAbsPos = " << rectAbsPos;

    // e ora calcolo la lunghezza della rect dello slider
    int lenSlider = int(float(rect().height()) * (float(m_maxNumLines) / float(m_internalLineCount + extraBottomLines)));

    // imposta un minimo di lunghezza per lo slider ed evita che vada disegnato fuori
    if(lenSlider < 15)
        lenSlider = 15;

    if(rectAbsPos + lenSlider > rect().height())
        rectAbsPos -= (rectAbsPos + lenSlider) - rect().height();

    //qDebug() << lenSlider;

    // Disegno area dello slider
    QRect rcSlider(0, rectAbsPos, rect().width() - 1, lenSlider );
    //p.fillRect( rcSlider, QColor( 55, 4, 255, 100 ) );



    ////// Routines di disegno ///////

    // Disegna una linea di separazione di 1 px
    QPen lp(QColor(29,29,29));
    p.setPen(lp);
    p.drawLine(rect().left(), rect().top(), rect().left(), rect().bottom());

    // Leggero gradiente da sx a dx
    QLinearGradient bkGrad(rect().topLeft(), rect().topRight());
    bkGrad.setColorAt(0, QColor(33,33,33));
    bkGrad.setColorAt(1, QColor(50,50,50));
    QRect rc = rect();
    rc.setLeft(rc.left()+1);
    p.fillRect(rc, bkGrad);


    // Routine di disegno per lo slider
    // rcSlider è la hitbox, ma per il drawing ne prendiamo una sottosezione in larghezza
    QRect rcSliderSubsection(rcSlider);
    rcSliderSubsection.setX(rcSliderSubsection.x()+3);
    rcSliderSubsection.setWidth(rcSliderSubsection.width()-2);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.setFillRule( Qt::WindingFill );
    path.addRoundedRect( rcSliderSubsection, 4, 4 );

    // seleziona un brush a gradiente
    QLinearGradient fillGrad(rect().topLeft(), rect().topRight());
    fillGrad.setColorAt(0, QColor(88,88,88));
    fillGrad.setColorAt(1, QColor(64,64,64));
    QBrush gradFill(fillGrad);
    p.setBrush(gradFill);


    //path.addRect( QRect( 200, 50, 50, 50 ) ); // Top right corner not rounded
        //path.addRect( QRect( 50, 100, 50, 50 ) ); // Bottom left corner not rounded
    p.drawPath( path.simplified() ); // Only Top left & bottom right corner rounded


/*    QPainter painter(this);


        QStyleOptionSlider option;
        initStyleOption(&option);

        option.subControls = QStyle::SC_All;


        style()->drawComplexControl(QStyle::CC_ScrollBar, &option, &painter, this);
*/
    //QScrollBar::paintEvent(event);
}

PgKeyEater::PgKeyEater ( CustomScrollBar *scrollBar ) :
    m_scrollBar(scrollBar)
{
}

bool PgKeyEater::eventFilter ( QObject *obj, QEvent *event ) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        // Mangia il pageup/pagedown, verrà gestito dalla scrollbar verticale
        if(keyEvent->key() == Qt::Key_PageUp) {
            m_scrollBar->m_scrollAnim->setProperty("pageStepEvent", 1);
            m_scrollBar->setSliderPosition(m_scrollBar->sliderPosition() - m_scrollBar->pageStep());
            return true;
        } else if (keyEvent->key() == Qt::Key_PageDown) {
            m_scrollBar->m_scrollAnim->setProperty("pageStepEvent", 2);
            m_scrollBar->setSliderPosition(m_scrollBar->sliderPosition() + m_scrollBar->pageStep());
            return true;
        }
    }
    // standard event processing
    return QObject::eventFilter(obj, event);
}
