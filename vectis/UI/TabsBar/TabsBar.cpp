#include <UI/TabsBar/TabsBar.h>
#include <QPainter>

#include <QtGlobal>
#include <QDebug>
#include <QStyleOption>

TabsBar::TabsBar( QWidget *parent )
    : m_parent(parent)
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
    m_tabs.push_back(Tab());
    m_tabs.push_back(Tab());
    m_tabs.push_back(Tab());
    m_selectedTabIndex = m_tabs.size();
    //DEBUG
}

// Disegna una tab selezionata o meno dentro un rect
QPainterPath TabsBar::drawTabInsideRect( QPainter& p, QRect& tabRect, bool selected,
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
    const QPen grayPen( QColor(60, 61, 56) ), blackPen( QColor(11, 11, 10) ), intersectionPen( QColor(56,56,53) );
    QPainterPath blackOuterTabPath = tabPath.translated(0, -1);
    QPainterPath bottomLine; // Evita che il contorno abbia imperfezioni
    bottomLine.moveTo(blackOuterTabPath.boundingRect().left(), blackOuterTabPath.boundingRect().bottom()-1);
    blackOuterTabPath.lineTo(blackOuterTabPath.boundingRect().right(), blackOuterTabPath.boundingRect().bottom()-1);
    blackOuterTabPath.subtracted(blackOuterTabPath);
    p.setPen(blackPen);
    p.drawPath(blackOuterTabPath);
    if( sxTabRect != 0 ) {
        QPainterPath sxRemnant = blackOuterTabPath.intersected(*sxTabRect);
        p.setPen(intersectionPen);
        p.drawPath(sxRemnant);
    }
    if( dxTabRect != 0 ) {
        QPainterPath dxRemnant = blackOuterTabPath.intersected(*dxTabRect);
        p.setPen(intersectionPen);
        p.drawPath(dxRemnant);
    }
    p.setPen(grayPen);
    p.drawPath(tabPath);

    if(selected == false) { // Le tab unselected hanno una leggera ulteriore sfumatura in basso
        p.setRenderHint(QPainter::Antialiasing, false);
        p.setRenderHint(QPainter::HighQualityAntialiasing, false);
        p.setPen(QPen(QColor(60,61,56)));
        p.drawLine(tabRect.left() + tabRect.height() / 4, tabRect.bottom(),
                   tabRect.right() - tabRect.height() / 4, tabRect.bottom());
    }
    return tabPath;
}

void TabsBar::paintEvent ( QPaintEvent* ) {
    QStyleOption opt; // Permette il setting del background via styleSheet
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);


    // La barra orizzontale che si interrompe solo sulla tab selezionata corrente, è di due pixel
    // e ha due colori:
    //
    //  -- nero rgb(11,11,10)   -- bcol1
    //  -- grigio rgb(60,61,56) -- gcol2
    const QColor outerBlackCol(11, 11, 10);
    const QColor innerGrayCol(60, 61, 56);

    p.setRenderHint(QPainter::Antialiasing, false);
    p.setRenderHint(QPainter::HighQualityAntialiasing, false);

    // Disegna la prima barra orizzontale in basso che si collega al CodeTextEdit;
    // questa corre per tutto il code editor *tranne* che per la selected tab
    const QPen grayPen( innerGrayCol ), blackPen( outerBlackCol );
    p.setPen(blackPen);
    p.drawLine( rect().left(), rect().bottom() - 1, rect().right(), rect().bottom() - 1);
    // e disegna anche la seconda barra grigia orizzontale
    p.setPen(grayPen);
    p.drawLine(rect().left(), rect().bottom(), rect().right(), rect().bottom());

    // Disegna le tab secondo un preciso ordine:
    // -> prima quelle NON selezionate a destra e a sinistra della selezionata, partendo da quella più vicina
    // alla selezionata per poi andare verso le esterne
    // -> poi le linee orizzontali (che vanno sopra alle non selezionate)
    // -> poi alla fine la selezionata

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
    int tabWidth = (rect().width() - (5+20 /* bordo sx 5 px, bordo dx 20 px */)) / (int)m_tabs.size();
    int tabHeight = rect().bottom() - 5;
    tabWidth += (TAB_INTERSECTION_DELTA * (int)(m_tabs.size()-1)) / (int)m_tabs.size();
    // Clamping del risultato ai valori max e min
    if(tabWidth > TAB_MAXIMUM_WIDTH)
        tabWidth = TAB_MAXIMUM_WIDTH;
    if(tabWidth < 2*tabHeight) // Almeno lo spazio per disegnare le curve di bezièr
        tabWidth = 2*tabHeight;
    QRect standardTabRect(5, 5, tabWidth /* Width */, tabHeight);

    // Disegna prima le tab a sx della selezionata (se ci sono)
    {
        QPainterPath temp;
        for(int i = m_selectedTabIndex - 1; i >= 0; --i) {
            int x = 5 + i*tabWidth;
            x -= TAB_INTERSECTION_DELTA*i;
            standardTabRect.setX(x);
            standardTabRect.setWidth(tabWidth);
            qDebug() << standardTabRect.x() << "," << standardTabRect.y() << "," <<
                        standardTabRect.width() << "," << standardTabRect.height();

            if(i == m_selectedTabIndex - 1)
                temp = drawTabInsideRect(p, standardTabRect, false);
            else
                temp = drawTabInsideRect(p, standardTabRect, false, &temp);
            //p.setPen(QColor(255 - i*20,0,0));
            //p.drawRect(standardTabRect);
        }
    }
    // DEBUG - La tab attualmente selezionata (TODO: può essere nulla)
    //QRect unSelectedTabRect(5, 5, 150 /* Width */, rect().bottom() - 5);

    //QPainterPath tab = drawTabInsideRect(p, unSelectedTabRect, false);

    //QRect selectedTabRect(5+unSelectedTabRect.width() - 20, 5, 150 /* Width */, rect().bottom() - 5);

    //drawTabInsideRect(p, selectedTabRect, true, &tab, 0);

    //p.setPen(QColor(255,0,0)); // Debug bordo tab
    //p.drawRect(selectedTabRect); // Debug bordo tab

    // Disegna la tab selezionata sopra a ogni altro controllo o bordo
    //drawTabInsideRect(p, unSelectedTabRect, true);

    // TODO: usa QPainterPath::intersects per mouse hit test
}
