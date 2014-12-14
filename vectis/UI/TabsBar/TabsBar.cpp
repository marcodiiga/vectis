#include <UI/TabsBar/TabsBar.h>
#include <QPainter>

#include <QtGlobal>
#include <QDebug>
#include <QStyleOption>
#include <QApplication>

TabsBar::TabsBar( QWidget *parent )
    : m_parent(parent),
      m_draggingInProgress(false)
{
    Q_ASSERT( parent );

    // WA_OpaquePaintEvent specifica che ridisegneremo il controllo ogni volta che ce n'è bisogno
    // senza intervento del sistema.
    setAttribute( Qt::WA_OpaquePaintEvent, false );
    //qInstallMessageHandler(crashMessageOutput);
    setStyleSheet("QWidget { background-color: rgb(22,23,19); }");

    QFont font("Verdana");
    font.setPixelSize(10);
    this->setFont(font);

    //DEBUG
    //DEBUG
    // cazzeggia qui per provare le tab

    // TODO: fai un metodo "insertNewTab"
    m_tabs.push_back(Tab("First tab"));
    m_interpolators.emplace_back(std::make_unique<SlideToPositionAnimation>(*this, m_tabs.size()-1));

    m_tabs.push_back(Tab("Second tab"));
    m_interpolators.emplace_back(std::make_unique<SlideToPositionAnimation>(*this, m_tabs.size()-1));

    m_tabs.push_back(Tab("Third tab"));
    m_interpolators.emplace_back(std::make_unique<SlideToPositionAnimation>(*this, m_tabs.size()-1));

    m_tabs.push_back(Tab("Fourth tab"));
    m_interpolators.emplace_back(std::make_unique<SlideToPositionAnimation>(*this, m_tabs.size()-1));

    //m_tabs.push_back(Tab());
    //m_tabs.push_back(Tab());
    //m_tabs.push_back(Tab());
    m_selectedTabIndex = 1;
    //DEBUG
}

// Disegna una tab selezionata o meno dentro un rect. Se si conoscono tab a destra e sinistra, consente di
// migliorarne l'aspetto inserendo una sfumatura
QPainterPath TabsBar::drawTabInsideRect(QPainter& p, const QRect& tabRect, bool selected, QString text,
                                         const QPainterPath* sxTabRect, const QPainterPath* dxTabRect ) {
    // Decide i colori per una tab selected o unselected
    QColor topGradientColor, bottomGradientColor;
    if( selected == false ) { // Unselected
        topGradientColor = QColor(54, 54, 52);
        bottomGradientColor = QColor(72, 72, 69);
    } else {
        topGradientColor = QColor(52, 53, 47);
        bottomGradientColor = QColor(39, 40, 34);
    }
    // Disegna due curve di bezièr e una linea orizzontale per la tab
    //  _______________
    // |0;0  c2  /     |h;0
    // |       _/      |
    // |     _/        |     Quadrato di contenimento della curva sx
    // |    /          |
    // |___/____c1_____|
    // 0;h              h;h
    QPainterPath tabPath;
    p.setRenderHint(QPainter::Antialiasing);
    const QPointF c1(0.6f, 1.0f-0.01f), c2(0.53f, 1.0f-1.0f);
    const int h = tabRect.height();
    tabPath.moveTo(tabRect.x(), tabRect.y() + tabRect.height() + 1); // QRect ha bordi di 1 px, compenso
    //qDebug() << selectedTabRect.x() << " " << selectedTabRect.y() + selectedTabRect.height() + 1;
    tabPath.cubicTo(tabRect.x() + h * c1.x(), tabRect.y() + h * c1.y(),
                 tabRect.x() + h * c2.x(), tabRect.y() + h * c2.y(),
                 tabRect.x() + h, tabRect.y()); // Destination point
    //qDebug() << selectedTabRect.x() + h << " " << selectedTabRect.y();

    int dxStartBez = tabRect.x() + tabRect.width() - h;
    tabPath.lineTo(dxStartBez, tabRect.y()); // Larghezza della tab

    // Disegna la bezièr dx
    tabPath.cubicTo(dxStartBez + (h - h * c2.x()), tabRect.y() + h * c2.y(),
                    dxStartBez + (h - h * c1.x()), tabRect.y() + h * c1.y(),
                    tabRect.x() + tabRect.width(), tabRect.y() + tabRect.height() + 1);

    // Riempie la tabPath con un gradiente di sfondo
    QLinearGradient tabGradient( tabRect.topLeft(), tabRect.bottomLeft() ); // Direzione verticale
    tabGradient.setColorAt( 1, bottomGradientColor ); // Colore finale è lo sfondo della code text box
    tabGradient.setColorAt( 0, topGradientColor ); // Colore iniziale (in alto) è più chiaro
    QBrush tabBrushFill( tabGradient );
    p.setBrush( tabBrushFill );
    p.fillPath( tabPath, tabBrushFill );

    p.setRenderHint(QPainter::SmoothPixmapTransform);
    // Una volta riempito (fillato) lo sfondo, si può disegnare sopra un bordo grigio ed
    // una path attaccata alla prima come un bordo nero traslata di 1 in alto
    const QPen grayPen( QColor(60, 61, 56) ), blackPen( QColor(11, 11, 10) ), intersectionPen( QColor(40,40,40) );
    QPainterPath blackOuterTabPath = tabPath.translated(0, -1);
    p.setPen( blackPen );
    p.drawPath( blackOuterTabPath );

    // Il bordo nero esterno va sfumato se ci sono delle tab di sfondo, altrimenti c'è uno stacco troppo netto
    p.setRenderHint( QPainter::Antialiasing, false );
    p.setRenderHint( QPainter::HighQualityAntialiasing, false );
    if( sxTabRect != 0 ) {
        QPainterPath sxRemnant = blackOuterTabPath.intersected( *sxTabRect );
        p.setPen( intersectionPen );
        p.drawPath( sxRemnant );
    }
    if( dxTabRect != 0 ) {
        QPainterPath dxRemnant = blackOuterTabPath.intersected( *dxTabRect );
        p.setPen( intersectionPen );
        p.drawPath( dxRemnant );
    }
    p.setPen( grayPen );
    p.drawPath( tabPath );

    if(selected == false) { // Le tab unselected hanno una ulteriore leggera sfumatura in basso
        p.setPen( QPen( QColor( 60,61,56 ) ) );
        p.drawLine( tabRect.left() + tabRect.height() / 4, tabRect.bottom(),
                    tabRect.right() - tabRect.height() / 4, tabRect.bottom() );
    }

    // Disegna il testo della tab nel sottorettangolo disponibile
    if( selected == true )
        p.setPen( Qt::white );
    else
        p.setPen( QPen(QColor(193,193,191)) );
    QRectF textRect = tabRect;
    textRect.setX(tabRect.x() + h);
    textRect.setWidth(tabRect.width() - 2*h);
    textRect.setY(textRect.y() + 6);
    p.drawText(textRect,  Qt::AlignLeft, QString(text));

    return tabPath; // Questo servirà per sfumare i bordi della selected, in caso questa tab sia ad essa vicina
}

// Disegna una barra orizzontale di separazione per il controllo
void TabsBar::drawGrayHorizontalBar( QPainter& p , const QColor innerGrayCol ) {
    p.setRenderHint( QPainter::Antialiasing, false );
    p.setRenderHint( QPainter::HighQualityAntialiasing, false );

    // Disegna la barra orizzontale in basso che si collega al CodeTextEdit;
    // questa corre per tutto il code editor *tranne* che per la selected tab
    const QPen grayPen( innerGrayCol );
    p.setPen( grayPen );
    p.drawLine( rect().left(), rect().bottom(), rect().right(), rect().bottom() );
}

void TabsBar::paintEvent ( QPaintEvent* ) {
    Q_ASSERT( m_selectedTabIndex == -1 || // Nulla è selezionato, oppure siamo entro i limiti del numero di tab
              ( m_selectedTabIndex != -1 && m_selectedTabIndex < m_tabs.size() ) );

    QStyleOption opt; // Permette il setting del background via styleSheet
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    const QColor innerGrayCol( 60, 61, 56 );

    if( m_selectedTabIndex == -1 ) {
        drawGrayHorizontalBar( p, innerGrayCol );
        return; // Nient'altro dev'essere disegnato
    }

    // Disegna le tab secondo un preciso ordine:
    // -> prima quelle NON selezionate a destra e a sinistra della selezionata, partendo da quella più vicina
    // alla selezionata per poi andare verso le esterne
    // -> poi la linea grigia orizzontale, sarà visibile dove NON ci sono tab
    // -> poi alla fine la selezionata (che va sopra a tutto)
    //

    // Calcola la dimensione di una tab a seconda della larghezza del controllo e di quante
    // ce ne sono da disegnare
    //
    //                   3w
    // |--------------------------------------| Se il controllo ha dimensione massima 3w (rect().width()-(bordi_vari)),
    // |     w      |     w      |     w      | ed ho 3 tab, la dimensione massima è w per ciascuna. Dato che però le tab
    // |            |            |            | non sono tenute esattamente una di seguito all'altra ma c'è un "intersection
    // |--------------------------------------| delta" fra di loro, cioè un'area nella quale queste tab si intersecano, allora
    // è necessario considerare che si lascia uno spazio inutilizzato alla fine proporzionalmente a quante tab ci sono:
    //                                                                          |-----------------------------------------|
    // Nel caso qui a destra si lascia uno spazio di 2d. Questo spazio deve     |         | d |        | d |        |
    // essere ridistribuito alle varie tab, quindi                              ---------------
    //                  tabWidth = 3w / 3                                                 ------------------
    //                  tabWidth += (3w - (3-1)*d)/3                                                   --------------  2d
    //                                                                                                               ------
    tabWidth = ( rect().width() - (5 + 20 /* bordo sx 5 px, bordo dx 20 px */) ) / (int)m_tabs.size();
    int tabHeight = rect().bottom() - 5;
    tabWidth += ( TAB_INTERSECTION_DELTA * (int)(m_tabs.size() - 1) ) / (int)m_tabs.size();
    // Clamping del risultato ai valori max e min
    if( tabWidth > TAB_MAXIMUM_WIDTH )
        tabWidth = TAB_MAXIMUM_WIDTH;
    if( tabWidth < 2 * tabHeight ) // Almeno lo spazio per disegnare le curve di bezièr
        tabWidth = 2 * tabHeight;
    QRect standardTabRect( 5, 5, tabWidth /* Width */, tabHeight );

    if( m_selectedTabIndex != -1 ) {
        // Disegna prima le tab a sx della selezionata (se ci sono)
        QPainterPath temp, leftOfSelected, rightOfSelected;
        for( int i = m_selectedTabIndex - 1; i >= 0; --i ) {
            int x = 5 + i * tabWidth;
            x -= TAB_INTERSECTION_DELTA * i;
            standardTabRect.setX( x );
            // Se abbiamo un offset in decreasing, aggiungicelo
            if(m_tabs[i].m_offset != 0) {
                standardTabRect.setX(standardTabRect.x() + m_tabs[i].m_offset);
            }
            standardTabRect.setWidth( tabWidth );

            if(i == m_selectedTabIndex - 1) { // La prima non ha bisogno di "overlapping" con nessuna
                temp = drawTabInsideRect( p, standardTabRect, false, m_tabs[i].m_title );
                leftOfSelected = temp;
            }
            else
                temp = drawTabInsideRect( p, standardTabRect, false, m_tabs[i].m_title, &temp );
            m_tabs[i].m_region = temp;
            //p.setPen(QColor(255 - i*20,0,0));
            //p.drawRect(standardTabRect);
        }
        // Disegna le tab a dx della selezionata (se ci sono)
        for( int i = (int)m_tabs.size()-1; i > m_selectedTabIndex; --i ) {
            int x = 5 + i * tabWidth;
            x -= TAB_INTERSECTION_DELTA * i;
            standardTabRect.setX( x );
            // Se abbiamo un offset in decreasing, aggiungicelo
            if(m_tabs[i].m_offset != 0) {
                standardTabRect.setX(standardTabRect.x() - m_tabs[i].m_offset);
            }
            standardTabRect.setWidth( tabWidth );
            //qDebug() << standardTabRect.x() << "," << standardTabRect.y() << "," <<
            //            standardTabRect.width() << "," << standardTabRect.height();

            if(i == m_tabs.size()-1) // La prima non ha bisogno di "overlapping" con nessuna
                temp = drawTabInsideRect( p, standardTabRect, false, m_tabs[i].m_title );
            else
                temp = drawTabInsideRect( p, standardTabRect, false, m_tabs[i].m_title, &temp );
            if( i == m_selectedTabIndex+1 )
                rightOfSelected = temp;
            m_tabs[i].m_region = temp;
            //m_tabs[i].m_rect = standardTabRect;
            //p.setPen(QColor(255 - i*20,0,0));
            //p.drawRect(standardTabRect);
        }

        drawGrayHorizontalBar( p, innerGrayCol );

        // Infine disegna la selezionata sopra a tutte le altre (c'è sempre se c'è almeno una tab)
        int x = 5 + m_selectedTabIndex * tabWidth;
        x -= TAB_INTERSECTION_DELTA * m_selectedTabIndex;

        // Fattori di aggiustamento (positivi o negativi) in caso siamo in dragging
        if( m_draggingInProgress ) {
            x += m_XTrackingDistance; // La tab deve rimanere dove la stavamo trascinando
        } else if(m_tabs[m_selectedTabIndex].m_offset != 0) {
            x += m_tabs[m_selectedTabIndex].m_offset;
        }
//qDebug() << "x is " << x;
        standardTabRect.setX( x );
        standardTabRect.setWidth( tabWidth );
        m_tabs[m_selectedTabIndex].m_region = drawTabInsideRect( p, standardTabRect, true, m_tabs[m_selectedTabIndex].m_title,
                          (m_selectedTabIndex > 0 ? &leftOfSelected : 0),
                          (m_selectedTabIndex < m_tabs.size()-1 ? &rightOfSelected : 0) );
        m_tabs[m_selectedTabIndex].m_rect = standardTabRect;
        //p.setPen(QColor(255 - 5*20,0,0));
        //p.drawRect(standardTabRect);
    }
}

// Si occupa del mouse click per cambiare una tab selezionata
void TabsBar::mousePressEvent(QMouseEvent *evt) {
    if ( evt->button() == Qt::LeftButton ) {
        m_dragStartPosition = evt->pos();
        m_selectionStartIndex = m_selectedTabIndex;
        for( size_t i=0; i<m_tabs.size(); ++i ) {
            if(m_tabs[i].m_region.contains(m_dragStartPosition) == true) {
                m_selectedTabIndex = (int)i;
                repaint();
                // TODO: inviare un signal "changedSelectedTab" con l'intero m_selectedTabIndex
            }
        }
    }
    qDebug() << "mousePressEvent " << m_dragStartPosition;
}

// Si occupa del dragging di una tab
void TabsBar::mouseMoveEvent( QMouseEvent *evt ) {
    if ( !(evt->buttons() & Qt::LeftButton) ) // L'unico tasto di cui ci occupiamo per il tracking
        return;

    int mouseXPosition = evt->pos().x();

    // NON permette di trascinare una tab fuori dall'area designata del controllo [+5;width-20]
    int tabRelativeX = 5 + m_selectedTabIndex * tabWidth;
    tabRelativeX -= TAB_INTERSECTION_DELTA * m_selectedTabIndex;
    tabRelativeX = m_dragStartPosition.x() - tabRelativeX;
    //qDebug() << tabRelativeX;
    if( mouseXPosition < 5 + tabRelativeX )
        mouseXPosition = 5 + tabRelativeX;
    if( mouseXPosition > this->width() - 20 - (tabWidth - tabRelativeX) )
        mouseXPosition = this->width() - 20 - (tabWidth - tabRelativeX);

    // Calcola la distanza negativa o positiva dal punto di inizio del tracking (sarà l'offset di quanto
    // dovrà essere spostata la QRect per disegnare la tab che stiamo trascinando)
    m_XTrackingDistance = mouseXPosition - m_dragStartPosition.x();

    //qDebug() << "mouseMoveEvent is dragging, m_XTrackingDistance(" << m_XTrackingDistance << "), tabWidth(" << tabWidth << "),"
    //        << "m_dragStartPosition.x(" << m_dragStartPosition.x() << ")";

    // Se abbiamo raggiunto la rect di un'altra tab, "prendiamo" il suo index
    // In Chrome grossomodo è: se c'è una tab nella direzione in cui stiamo andando e abbiamo raggiunto la metà
    // nella sua direzione: spostala. Poi se continuo altra metà e non succede niente. E poi si ricomincia.
    // quindi tabWidth / 2 è il "criterio" da memorizzare
    if( abs(m_XTrackingDistance) > tabWidth / 2 ) {
        // Effettuiamo uno swap se c'è almeno una tab in quella direzione
        if( m_XTrackingDistance > 0 ) {
            if( m_tabs.size()-1 > m_selectedTabIndex ) {
                // Swap del contenuto del vettore delle tabs e aggiorno il nuovo indice
                setUpdatesEnabled(false);   // Disabilita i paint() FINO al termine di tutti gli aggiornamenti & del set
                                            // dell'interpolatore per la tab "detronizzata".

                //qDebug() << "Swap della corrente (index: " << m_selectedTabIndex << ") con tab index: " << m_selectedTabIndex+1;
                std::swap(m_tabs[m_selectedTabIndex], m_tabs[m_selectedTabIndex+1]);
                //qDebug() << m_tabs[m_selectedTabIndex].m_rect;
                //qDebug() << m_tabs[m_selectedTabIndex+1].m_rect;

                // Una volta che una tab ha superato di tabWidth/2 in una direzione, si swappa con la prima tab in quella
                // direzione:
                //                  |-------|                      |-------|
                // (startDragPoint) |   1   | -------------------> |   2   |
                //                  |-------|                      |-------|
                //         XTrackingDistance è appena diventato maggiore di tabWidth / 2, è ora di swappare
                //
                // però anche il punto di inizio si sposta avanti
                //                  |-------|                      |-------|
                //                  |   2   | --(startDragPoint)-> |   1   |
                //                  |-------|                      |-------|
                //         XTrackingDistance -= tabWidth, ora è negativo e infatti la tab 1 è spostata a sinistra e
                //         non ha ancora raggiunto la posizione di equilibrio nel nuovo rettangolo
                //
                // NOTA BENE: aumentare un posto a dx significa anche compensare il TAB_INTERSECTION_DELTA (uno in meno)
                m_XTrackingDistance = -(tabWidth / 2) + TAB_INTERSECTION_DELTA;
                //qDebug() << "m_XTrackingDistance -= tabWidth => m_XTrackingDistance(" << m_XTrackingDistance << ")";
                m_dragStartPosition.setX(m_dragStartPosition.x() + tabWidth - TAB_INTERSECTION_DELTA);
                //qDebug() << "m_dragStartPosition.x(" << m_dragStartPosition.x() << ")";

                // Avvia l'interpolatore per lo swap dell'altra tab (deve tornare alla sua posizione di equilibrio)
                int offset = tabWidth - TAB_INTERSECTION_DELTA;
                m_tabs[m_selectedTabIndex].m_offset = offset; // Deve andare a zero

                ++m_selectedTabIndex;
                m_interpolators[m_selectedTabIndex-1]->setDuration(200);
                m_interpolators[m_selectedTabIndex-1]->setStartValue(offset);
                m_interpolators[m_selectedTabIndex-1]->setEndValue(0);
                m_interpolators[m_selectedTabIndex-1]->start();
                setUpdatesEnabled(true);
            }
        } else {
            if( m_selectedTabIndex > 0 )
                // Swap del contenuto del vettore delle tabs e aggiorno il nuovo indice
                setUpdatesEnabled(false);   // Disabilita i paint() FINO al termine di tutti gli aggiornamenti & del set
                                            // dell'interpolatore per la tab "detronizzata".
                //qDebug() << "Swap della corrente (index: " << m_selectedTabIndex << ") con tab index: " << m_selectedTabIndex-1;
                std::swap(m_tabs[m_selectedTabIndex], m_tabs[m_selectedTabIndex-1]);

                // Stesso ragionamento (inverso) del caso sopra
                m_XTrackingDistance = +(tabWidth / 2) - TAB_INTERSECTION_DELTA;
                m_dragStartPosition.setX(m_dragStartPosition.x() - tabWidth + TAB_INTERSECTION_DELTA);

                // Avvia l'interpolatore per lo swap dell'altra tab (deve tornare alla sua posizione di equilibrio)
                int offset = tabWidth - TAB_INTERSECTION_DELTA;
                m_tabs[m_selectedTabIndex].m_offset = offset; // Deve andare a zero

                --m_selectedTabIndex;
                m_interpolators[m_selectedTabIndex+1]->setDuration(200);
                m_interpolators[m_selectedTabIndex+1]->setStartValue(offset);
                m_interpolators[m_selectedTabIndex+1]->setEndValue(0);
                m_interpolators[m_selectedTabIndex+1]->start();
                setUpdatesEnabled(true);
        }
    }


    m_draggingInProgress = true;
    repaint();
}

// Segnala la fine di un tracking event
void TabsBar::mouseReleaseEvent(QMouseEvent *evt) {
    if( m_draggingInProgress == false )
        return;

    qDebug() << "Tracking ended";

    // Anima il "ritorno" alla posizione corretta, i.e. diminuisce a zero il XTrackingDistance
    m_tabs[m_selectedTabIndex].m_offset = m_XTrackingDistance; // Deve andare a zero
    m_interpolators[m_selectedTabIndex]->setDuration(200);
    m_interpolators[m_selectedTabIndex]->setStartValue(m_XTrackingDistance);
    m_interpolators[m_selectedTabIndex]->setEndValue(0);
    m_interpolators[m_selectedTabIndex]->start();

    m_draggingInProgress = false;
}



SlideToPositionAnimation::SlideToPositionAnimation(TabsBar& parent, size_t associatedTabIndex ) :
    m_parent(parent),
    m_associatedTabIndex(associatedTabIndex)
{
    connect(this, SIGNAL(finished()), SLOT(animationHasFinished()));
}

// Metodo chiamato ad ogni variazione del valore di interpolazione, deve assicurarsi di aggiornare
// l'offset di ogni tab
void SlideToPositionAnimation::updateCurrentValue(const QVariant &value) {
    // Aggiorna m_XTrackingDistance
    //qDebug() << "updateCurrentValue(" << value.toInt() << ")";
    //m_parent->m_XTrackingDistance = value.toInt();
    //m_parent->repaint();

    // Aggiorna l'offset per la tab associata
    m_parent.m_tabs[m_associatedTabIndex].m_offset = value.toInt();
    // repaint?
    m_parent.repaint();
}

void SlideToPositionAnimation::animationHasFinished() {
    //m_parent.m_draggingInProgress = false;
}
