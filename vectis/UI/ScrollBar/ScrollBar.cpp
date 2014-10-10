#include <UI/ScrollBar/ScrollBar.h>
#include <QtCore/qmath.h>
#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include <QTextBlock>
#include <QDebug> // TODO: rimuovi quando il resize è terminato

// Un filtro per gli eventi PageDown/Up per evitare il problema dello spostamento del caret che provoca una errata
// animazione di scroll
PgKeyEater::PgKeyEater ( ScrollBar *scrollBar ) :
    m_scrollBar( scrollBar ) {
}

bool PgKeyEater::eventFilter ( QObject *obj, QEvent *event ) {
    if ( event->type() == QEvent::KeyPress ) { // L'unico evento da gestire è il PgDown/Up
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        // Mangia completamente il pageup/pagedown; verrà gestito dalla scrollbar verticale (TODO: evitare questo per orizzontali)
        if( keyEvent->key() == Qt::Key_PageUp ) {
            m_scrollBar->m_scrollAnim->setProperty( "pageStepEvent", 1 ); // Segnala che l'evento era un PgUp
            // Imposta la nuova posizione manualmente, dato che questo evento sarà mangiato interamente
            m_scrollBar->setSliderPosition( m_scrollBar->sliderPosition() - m_scrollBar->pageStep() );
            return true; // Evento completamente gestito, interrompi l'handling per le sottoclassi
        } else if ( keyEvent->key() == Qt::Key_PageDown ) {
            m_scrollBar->m_scrollAnim->setProperty( "pageStepEvent", 2 ); // PgDown
            m_scrollBar->setSliderPosition( m_scrollBar->sliderPosition() + m_scrollBar->pageStep() ); // Idem sopra
            return true;
        }
    }
    // Per altri eventi: standard event processing
    return QObject::eventFilter( obj, event );
}

ScrollBar::ScrollBar ( QTextEdit * parent ) :
    QScrollBar(parent),
    m_parent(parent),
    m_textLineHeight(1), // Altezza di ogni riga, dipende dal font usato (anche se è sempre monospaced)
    m_internalLineCount(1), // Quante righe ci sono nel documento realmente
    m_sliderIsBeingDragged(false)
{
    Q_ASSERT( parent );

    // WA_OpaquePaintEvent specifica che ridisegneremo il controllo ogni volta che ce n'è bisogno
    // senza intervento del sistema. WA_NoSystemBackground invece evita che il sistema disegni
    // il background (che comunque gestiremo noi)
    setAttribute( Qt::WA_OpaquePaintEvent, false );
    setAttribute( Qt::WA_NoSystemBackground, true );

    // L'unico modo per rilevare quando il line wrapping inserisce delle righe è tramite documentSizeChanged()
    connect( m_parent->document()->documentLayout(), SIGNAL(documentSizeChanged(const QSizeF&)),
            this, SLOT(documentSizeChanged(const QSizeF&)) );
    // Handling dei segnali scroll mouse / page down-up / trascinamento (tracking)
    connect(this, SIGNAL( actionTriggered(int)), this, SLOT(actionTriggered(int)));
    connect(this, SIGNAL(sliderPressed()), this, SLOT(sliderPressed()));
    connect(this, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));

    // Un timer per una animazione di scorrimento più gradevole
    m_scrollAnim.reset( new QPropertyAnimation(this, "value") );
    // Un filtro tastiera per annullare il pageup/down e gestirlo internamente (è necessario poichè
    // altrimenti non si può evitare che il caret, che viene spostato con pagedown/up, cambi il valore di dove
    // punta la scrollbar e alla prossima animazione ci sia uno spostamento immediato sbagliato)
    m_pgKeyEater.reset( new PgKeyEater(this) ); // TODOcpp14: usa make_unique per ogni smart pointer
    // Installa il PageDown/PageUp key eater per il controllo parent
    m_parent->installEventFilter( m_pgKeyEater.get() );
    // Muove il caret alla fine dell'animazione di scroll (property pageStepEvent aiuta a non farlo per il mouse scroll)
    connect( m_scrollAnim.get(), SIGNAL(finished()), this, SLOT(moveParentCaret()) );
}

ScrollBar::~ScrollBar() {
    m_parent->removeEventFilter(m_pgKeyEater.get());
}

// Eventi di tracking (l'utente sta muovendo il caret a mano)
void ScrollBar::sliderReleased () {
    m_sliderIsBeingDragged = false;
}
void ScrollBar::sliderPressed () {
    m_sliderIsBeingDragged = true;
}

// Sposta il caret (cursore) del parent alla posizione raggiunta dalla scrollbar. SOLTANTO quando l'animazione
// è terminata (così non ci sono intoppi nella scorrevolezza). Utile quando è stato chiamato un PageUp/PageDown.
// Un mousescroll non causa movimento del caret e quindi non viene gestito (pageStepEvent rimane 0 in quel caso)
void ScrollBar::moveParentCaret() {
    int pageValue = m_scrollAnim->property("pageStepEvent").toInt();
    if(pageValue > 0) { // PageDown o PageUp è stato premuto
        QTextCursor cursor = m_parent->textCursor();
        // Riavvolge il cursore all'inizio del documento e poi lo posiziona alla riga dove doveva essere posizionato
        cursor.movePosition( QTextCursor::Start, QTextCursor::MoveAnchor );
        cursor.movePosition( QTextCursor::NextBlock, QTextCursor::MoveAnchor,
                             value() / m_textLineHeight); // Ricorda che il valore dello slider è indice_riga * altezza_riga
        m_parent->setTextCursor(cursor);
        m_scrollAnim->setProperty("pageStepEvent", 0); // Aiuta a non resettare il caret per un mouse scroll (non serve)
    }
}

// Slot invocato ogni volta che lo slider è mosso (scrolling, pageup/down, dragging (tracking))
// Notare che:
//   "When the signal is emitted, the sliderPosition has been adjusted according to the action, but the value has not
//    yet been propagated (meaning the valueChanged() signal was not yet emitted)"
void ScrollBar::actionTriggered ( int action ) {
    if(action == QAbstractSlider::SliderNoAction || m_sliderIsBeingDragged) // Nessuna animazione in tracking
        return;

    // Se una animazione è già in corso interrompila: i valori saranno aggiornati e una nuova animazione ripartirà
    if(m_scrollAnim->state() == QAbstractAnimation::State::Running)
        m_scrollAnim->stop();

    m_scrollAnim->setDuration(120);
    m_scrollAnim->setStartValue(value());
    m_scrollAnim->setEndValue(sliderPosition());
    m_scrollAnim->start();
}

// Quando il controllo viene resizato, si aggiorna anche il numero massimo di righe che possiamo visualizzare
// entro la view
void ScrollBar::resizeEvent ( QResizeEvent * event ) {
   qDebug() << "TODO: resizeEvent per la ScrollBar, è utile il setMaximum qua? Se basta il docSizeChange rimuovilo";
   // setMaximum( m_internalLineCount * m_textLineHeight );
    //qDebug() << "resizeEvent: maximum aggiornato a: " << maximum();
    //qDebug() << "m_maxNumLines is now " << m_maxNumLines;
    //qDebug() << "textLine.height() is  " << m_textLineHeight;
    QScrollBar::resizeEvent(event);
}

// Ogni volta che c'è un cambiamento nello slider (e.g. valore/range/orientamento/etc..) il numero di righe massime
// deve essere ridimensionato per aggiungere delle righe "vuote" virtuali (leggi sotto)
void ScrollBar::sliderChange ( SliderChange change ) {
    // Per poter simulare delle "righe vuote virtuali" alla fine e permettere di scrollare
    // l'ultima riga fino all'inizio della view è necessario rilevare quando cambia il valore dello
    // slider (e.g. sto scrollando o aggiungo/tolgo righe oppure wrappo) e aumentare il massimo dove si può scrollare.
    // Non posso fare un semplice m_internalLineCount = (m_parent->document()->lineCount() - 1);
    // perchè in wrap mode non è sempre valido e crea problemi
    setMaximum( (m_internalLineCount - 1) * m_textLineHeight );
    // qDebug() << "sliderValueChange dopo setMaximum ricevuto e value == " << value();
    QAbstractSlider::sliderChange(change);
}

// Emesso quando il documento cambia size, è l'unico modo per rilevare il numero delle linee del documento con il wrapping
void ScrollBar::documentSizeChanged(const QSizeF & newSize) {

    // Informazioni utili da sapere:
    // - La gerarchia usata per trovare il parent QPlainTextEdit widget è:
    //     QScrollBar >parent> qt_scrollarea_vcontainer >parent> QPlainTextEdit
    // - Se non inseriamo frames o tabelle, blocks == lines

    // Calcola la height di una riga qualsiasi (la prima che è sempre presente)
    QTextBlock block = m_parent->document()->findBlockByNumber( 0 );
    QTextLayout *layout = block.layout(); // Layout di una riga
    QTextLine textLine = layout->lineAt( 0 );
    m_textLineHeight = textLine.height();
    // Aggiorna il numero massimo di righe visibili nel controllo testo, questo può essere stato cambiato
    m_maxViewVisibleLines = qFloor( qreal( m_parent->height() ) / m_textLineHeight );

    // Calcola il numero reale di righe del documento
    m_internalLineCount = int( newSize.height() / m_textLineHeight );
    // Aggiorna anche il maximum consentito per permettere di scrollare l'ultima riga fino all'inizio della view
    setMaximum( (m_internalLineCount - 1)* m_textLineHeight );
    // qDebug() << "m_textLineHeight "  << m_textLineHeight << " m_maxNumLines " << m_maxViewVisibleLines << " m_internalLineCount "
    //   << m_internalLineCount;
}

// L'evento più importante del controllo: il repaint.
// L'equazione fondamentale per il repainting è:
//  lunghezza_dello_slider = altezza_massima_slider * (quante_righe_posso_visualizzare_nella_view / righe_totali_nel_documento)
void ScrollBar::paintEvent ( QPaintEvent* ) {

    QPainter p( this );

    // >> ---------------------------------------------------------------------
    //    Calcolo della posizione, lunghezza e area di disegno dello slider
    // --------------------------------------------------------------------  <<

    // Le extraBottomLines sono righe virtuali per far scrollare l'ultima riga del testo fino a rimanere da sola in alto nella
    // view. Quindi sono il numero massimo di righe che la view può visualizzare - 1 (esclusa la riga che voglio)
    int extraBottomLines = (m_maxViewVisibleLines - 1);

    // Dato che maximum() è SEMPRE maggiore di value() (il numero di linee del controllo è sempre
    // maggiore o uguale della prima riga visualizzata dalla view), posso esprimere il rapporto come
    //  posizione_iniziale_slider = altezza_view * (riga_dove_si_trova_la_view / max_righe_documento)
    float viewRelativePos = float(m_maxViewVisibleLines) * (float(value()) / float(maximum() + (extraBottomLines*m_textLineHeight)));

    // e ora trova la posizione assoluta nella rect del controllo, la proporzione è
    //  rect().height() : x = m_maxViewVisibleLines : viewRelativePos
    float rectAbsPos = (float(rect().height()) * viewRelativePos) / float(m_maxViewVisibleLines);

    // qDebug() << "maxNumLines is " << maxNumLines << " and viewRelativePos is = " << viewRelativePos <<
    //            " rectAbsPos = " << rectAbsPos;

    // e infine calcolo la lunghezza della rect dello slider includendo le extraBottomLines
    int lenSlider = int( float(rect().height()) * (float(m_maxViewVisibleLines) / float(m_internalLineCount + extraBottomLines)) );

    // Imposta un minimo di lunghezza per lo slider (quando ci sono tantissime righe)
    if( lenSlider < 15 )
        lenSlider = 15;

    // Evita che lo slider possa, per errori di roundoff, essere disegnato fuori dal rettangolo della scrollbar
    if( rectAbsPos + lenSlider > rect().height() )
        rectAbsPos -= ( rectAbsPos + lenSlider ) - rect().height();

    // Questa è infine l'area di disegno per lo slider
    QRect rcSlider(0, rectAbsPos, rect().width() - 1, lenSlider );
    // p.fillRect( rcSlider, QColor( 55, 4, 255, 100 ) );


    // >> ------------------------
    //    Disegno dello slider
    // -----------------------  <<

    // Disegna una linea di separazione di 1 px
    QPen lp( QColor( 29, 29, 29 ) );
    p.setPen(lp);
    p.drawLine( rect().left(), rect().top(), rect().left(), rect().bottom() );

    // Leggero gradiente di sfondo da sx a dx
    QLinearGradient bkGrad( rect().topLeft(), rect().topRight() );
    bkGrad.setColorAt( 0, QColor(33, 33, 33) );
    bkGrad.setColorAt( 1, QColor(50, 50, 50) );
    QRect rc = rect();
    rc.setLeft( rc.left()+1 );
    p.fillRect( rc, bkGrad );

    // Disegno dello slider con un rounded-rectangle
    // rcSlider è la hitbox, ma per il drawing ne prendiamo solo una sottosezione in larghezza
    QRect rcSliderSubsection( rcSlider);
    rcSliderSubsection.setX( rcSliderSubsection.x()+3 );
    rcSliderSubsection.setWidth( rcSliderSubsection.width()-2 );
    p.setRenderHint( QPainter::Antialiasing );
    QPainterPath path;
    path.setFillRule( Qt::WindingFill ); // Riempimento per closed-shapes
    path.addRoundedRect( rcSliderSubsection, 4, 4 );

    // Seleziona un brush a gradiente per il riempimento dello slider
    QLinearGradient fillGrad( rect().topLeft(), rect().topRight() );
    fillGrad.setColorAt( 0, QColor(88, 88, 88) );
    fillGrad.setColorAt( 1, QColor(64, 64, 64) );
    QBrush gradFill( fillGrad );
    p.setBrush( gradFill );

    // Disegna finalmente lo slider
    p.drawPath( path.simplified() /* Unisce tutti gli eventuali segmenti e ottiene un solo path */ );

    // QScrollBar::paintEvent(event); // Nessun paintEvent della baseclass, abbiamo gestito completamente il ridisegno
}
