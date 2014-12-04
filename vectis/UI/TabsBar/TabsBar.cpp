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
}

void TabsBar::paintEvent ( QPaintEvent* ) {
    QStyleOption opt; // Permette il setting del background via styleSheet
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    p.setRenderHint(QPainter::Antialiasing);

    // La barra orizzontale che si interrompe solo sulla tab selezionata corrente, è di due pixel
    // e ha due colori:
    //
    //  -- nero rgb(11,11,10)   -- bcol1
    //  -- grigio rgb(60,61,56) -- gcol2
    const QColor outerBlackCol(11, 11, 10);
    const QColor innerGrayCol(60, 61, 56);

    // La tab attualmente selezionata (TODO: può essere nulla)
    QRect selectedTabRect(5, 5, 150 /* Width */, rect().bottom() - 5);

    // Disegna la barra orizzontale in basso che si collega al CodeTextEdit;
    // questa corre per tutto il code editor *tranne* che per la selected tab
    const QPen grayPen( innerGrayCol ), blackPen( outerBlackCol );
    p.setPen(grayPen);
    p.drawLine( rect().left(), rect().bottom(), selectedTabRect.left(), rect().bottom());
    p.drawLine( selectedTabRect.right(), rect().bottom(), rect().right(), rect().bottom());
    p.setPen(blackPen);
    p.drawLine( rect().left(), rect().bottom() - 1, selectedTabRect.left(), rect().bottom() - 1);
    p.drawLine( selectedTabRect.right(), rect().bottom() - 1, rect().right(), rect().bottom() - 1);

    p.setPen(QColor(255,0,0)); // Debug
    //p.drawRect(selectedTabRect); // Debug

    // Disegna due curve di bezièr e una linea orizzontale per la tab
    //  _______________
    // |0;0  c2  /     |h;0
    // |       _/      |
    // |     _/        |     Quadrato di contenimento della curva sx
    // |    /          |
    // |___/____c1_____|
    // 0;h              h;h
    QPainterPath tabPath;
    const QPointF c1(0.52f, 0.97f), c2(0.63f, -0.04f);
    const int h = selectedTabRect.height();
    tabPath.moveTo(selectedTabRect.x(), selectedTabRect.y() + selectedTabRect.height() + 1); // QRect ha bordi di 1 px, compenso
    qDebug() << selectedTabRect.x() << " " << selectedTabRect.y() + selectedTabRect.height() + 1;
    tabPath.cubicTo(selectedTabRect.x() + h * c1.x(), selectedTabRect.y() + h * c1.y(),
                 selectedTabRect.x() + h * c2.x(), selectedTabRect.y() + h * c2.y(),
                 selectedTabRect.x() + h, selectedTabRect.y()); // Destination point
    qDebug() << selectedTabRect.x() + h << " " << selectedTabRect.y();

    int dxStartBez = selectedTabRect.x() + selectedTabRect.width() - h;
    tabPath.lineTo(dxStartBez, selectedTabRect.y()); // Larghezza della tab

    // Disegna la bezièr dx
    tabPath.cubicTo(dxStartBez + (h - h * c2.x()), selectedTabRect.y() + h * c2.y(),
                    dxStartBez + (h - h * c1.x()), selectedTabRect.y() + h * c1.y(),
                    selectedTabRect.x() + selectedTabRect.width(), selectedTabRect.y() + selectedTabRect.height() + 1);

    // Riempie la tabPath con un gradiente di sfondo
    QLinearGradient selectedTabGradient( selectedTabRect.topLeft(), selectedTabRect.bottomLeft() ); // Direzione verticale
    selectedTabGradient.setColorAt( 1, QColor(39, 40, 34) ); // Colore finale è lo sfondo della code text box
    selectedTabGradient.setColorAt( 0, QColor(52, 53, 47) ); // Colore iniziale (in alto) è più chiaro
    QBrush selectedTabBrushFill( selectedTabGradient );
    p.setBrush( selectedTabBrushFill );
    p.fillPath( tabPath, selectedTabBrushFill );

    // Una volta riempito (fillato) lo sfondo, si può disegnare sopra un bordo grigio ed
    // una path attaccata alla prima come un bordo nero traslata di 2 in alto
    QPainterPath blackOuterTabPath = tabPath.translated(0, -2);
    p.setPen(blackPen);
    p.drawPath(blackOuterTabPath);
    p.setPen(grayPen);
    p.drawPath(tabPath);

    // TODO: usa QPainterPath::intersects per mouse hit test
}
