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
    m_tabs.push_back(std::make_unique<Tab>());
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

    // TODO: DISEGNA QUI LE TAB -NON- SELEZIONATE, POI DISEGNA SOPRA LA LINEA ORIZZONTALE E INFINE QUELLA SELEZIONATA

    // DEBUG - La tab attualmente selezionata (TODO: può essere nulla)
    QRect unSelectedTabRect(5, 5, 150 /* Width */, rect().bottom() - 5);

    QPainterPath tab = drawTabInsideRect(p, unSelectedTabRect, false);

    QRect selectedTabRect(5+unSelectedTabRect.width() - 20, 5, 150 /* Width */, rect().bottom() - 5);

    drawTabInsideRect(p, selectedTabRect, true, &tab, 0);

    //p.setPen(QColor(255,0,0)); // Debug bordo tab
    //p.drawRect(selectedTabRect); // Debug bordo tab

    // Disegna la tab selezionata sopra a ogni altro controllo o bordo
    //drawTabInsideRect(p, unSelectedTabRect, true);

    // TODO: usa QPainterPath::intersects per mouse hit test
}
