#include <UI/TabsBar/TabsBar.h>
#include <QPainter>

#include <QtGlobal>
#include <QDebug>
#include <QStyleOption>
#include <QApplication>

TabsBar::TabsBar( QWidget *parent )
    : m_parent(parent),
      m_draggingInProgress(false),
      m_slideAnimation(this)
{
    Q_ASSERT( parent );

    // WA_OpaquePaintEvent specifica che ridisegneremo il controllo ogni volta che ce n'è bisogno
    // senza intervento del sistema.
    setAttribute( Qt::WA_OpaquePaintEvent, false );
    //qInstallMessageHandler(crashMessageOutput);
    setStyleSheet("QWidget { background-color: rgb(22,23,19); }");

    //DEBUG
    //DEBUG
    // cazzeggia qui per provare le tab
    m_tabs.push_back(Tab());
    m_tabs.push_back(Tab());
    m_tabs.push_back(Tab());
    m_tabs.push_back(Tab());
    //m_tabs.push_back(Tab());
    //m_tabs.push_back(Tab());
    //m_tabs.push_back(Tab());
    m_selectedTabIndex = 3;
    //DEBUG
}

// Disegna una tab selezionata o meno dentro un rect. Se si conoscono tab a destra e sinistra, consente di
// migliorarne l'aspetto inserendo una sfumatura
QPainterPath TabsBar::drawTabInsideRect( QPainter& p, const QRect& tabRect, bool selected,
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
    const QPen grayPen( QColor(60, 61, 56) ), blackPen( QColor(11, 11, 10) ), intersectionPen( QColor(70,70,67) );
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
            standardTabRect.setWidth( tabWidth );
            //qDebug() << standardTabRect.x() << "," << standardTabRect.y() << "," <<
            //            standardTabRect.width() << "," << standardTabRect.height();

            if(i == m_selectedTabIndex - 1) { // La prima non ha bisogno di "overlapping" con nessuna
                temp = drawTabInsideRect( p, standardTabRect, false );
                leftOfSelected = temp;
            }
            else
                temp = drawTabInsideRect( p, standardTabRect, false, &temp );
            m_tabs[i].m_region = temp;
            //p.setPen(QColor(255 - i*20,0,0));
            //p.drawRect(standardTabRect);
        }
        // Disegna le tab a dx della selezionata (se ci sono)
        for( int i = (int)m_tabs.size()-1; i > m_selectedTabIndex; --i ) {
            int x = 5 + i * tabWidth;
            x -= TAB_INTERSECTION_DELTA * i;
            standardTabRect.setX( x );
            standardTabRect.setWidth( tabWidth );
            //qDebug() << standardTabRect.x() << "," << standardTabRect.y() << "," <<
            //            standardTabRect.width() << "," << standardTabRect.height();

            if(i == m_tabs.size()-1) // La prima non ha bisogno di "overlapping" con nessuna
                temp = drawTabInsideRect( p, standardTabRect, false );
            else
                temp = drawTabInsideRect( p, standardTabRect, false, &temp );
            if( i == m_selectedTabIndex+1 )
                rightOfSelected = temp;
            m_tabs[i].m_region = temp;
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
        }

        standardTabRect.setX( x );
        standardTabRect.setWidth( tabWidth );
        m_tabs[m_selectedTabIndex].m_region = drawTabInsideRect( p, standardTabRect, true,
                          (m_selectedTabIndex > 0 ? &leftOfSelected : 0),
                          (m_selectedTabIndex < m_tabs.size()-1 ? &rightOfSelected : 0) );
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

    // Calcola la distanza negativa o positiva dal punto di inizio del tracking (sarà l'offset di quanto
    // dovrà essere spostata la QRect per disegnare la tab che stiamo trascinando)
    m_XTrackingDistance = evt->pos().x() - m_dragStartPosition.x();

    qDebug() << "mouseMoveEvent is dragging, m_XTrackingDistance(" << m_XTrackingDistance << "), tabWidth(" << tabWidth << "),"
             << "m_dragStartPosition.x(" << m_dragStartPosition.x() << ")";

    // Se abbiamo raggiunto la rect di un'altra tab, "prendiamo" il suo index
    // In Chrome grossomodo è: se c'è una tab nella direzione in cui stiamo andando e abbiamo raggiunto la metà
    // nella sua direzione: spostala. Poi se continuo altra metà e non succede niente. E poi si ricomincia.
    // quindi tabWidth / 2 è il "criterio" da memorizzare
    if( abs(m_XTrackingDistance) > tabWidth / 2 ) {
        // Effettuiamo uno swap se c'è almeno una tab in quella direzione
        if( m_XTrackingDistance > 0 ) {
            if( m_tabs.size()-1 > m_selectedTabIndex ) {
                // Swap del contenuto del vettore delle tabs e aggiorno il nuovo indice
                //qDebug() << "Swap della corrente (index: " << m_selectedTabIndex << ") con tab index: " << m_selectedTabIndex+1;
                std::swap(m_tabs[m_selectedTabIndex], m_tabs[m_selectedTabIndex+1]);
                ++m_selectedTabIndex;
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
                m_XTrackingDistance -= tabWidth;
                //qDebug() << "m_XTrackingDistance -= tabWidth => m_XTrackingDistance(" << m_XTrackingDistance << ")";
                m_dragStartPosition.setX(m_dragStartPosition.x() + tabWidth);
                //qDebug() << "m_dragStartPosition.x(" << m_dragStartPosition.x() << ")";
            }
        } else {
            if( m_selectedTabIndex > 0 )
                // Swap del contenuto del vettore delle tabs e aggiorno il nuovo indice
                qDebug() << "Swap della corrente (index: " << m_selectedTabIndex << ") con tab index: " << m_selectedTabIndex-1;
                std::swap(m_tabs[m_selectedTabIndex], m_tabs[m_selectedTabIndex-1]);
                --m_selectedTabIndex;
                // Stesso ragionamento (inverso) del caso sopra
                m_XTrackingDistance += tabWidth;
                qDebug() << "m_XTrackingDistance += tabWidth => m_XTrackingDistance(" << m_XTrackingDistance << ")";
                m_dragStartPosition.setX(m_dragStartPosition.x() - tabWidth);
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
    //m_draggingInProgress = false;

    // Anima il "ritorno" alla posizione corretta, i.e. diminuisce a zero il XTrackingDistance
    m_slideAnimation.setDuration(1000);
    m_slideAnimation.setStartValue(m_XTrackingDistance);
    m_slideAnimation.setEndValue(0);
    m_slideAnimation.start();
}



SlideToPositionAnimation::SlideToPositionAnimation( TabsBar *parent ) :
    m_parent(parent)
{
    connect(this, SIGNAL(finished()), SLOT(animationHasFinished()));
}

// Metodo chiamato ad ogni variazione del valore di interpolazione, deve assicurarsi di aggiornare
// l'XTrackingDistance di ogni tab
void SlideToPositionAnimation::updateCurrentValue(const QVariant &value) {
    // Aggiorna m_XTrackingDistance
    //qDebug() << "updateCurrentValue(" << value.toInt() << ")";
    m_parent->m_XTrackingDistance = value.toInt();
    m_parent->repaint();
}

void SlideToPositionAnimation::animationHasFinished() {
    m_parent->m_draggingInProgress = false;
}
