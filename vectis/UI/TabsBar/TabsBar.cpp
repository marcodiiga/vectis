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
    //setAttribute( Qt::WA_OpaquePaintEvent, false );
    //qInstallMessageHandler(crashMessageOutput);
    setStyleSheet("QWidget { background-color: rgb(22,23,19); }");
}

void TabsBar::paintEvent ( QPaintEvent* ) {
    QStyleOption opt; // Permette il setting del background via styleSheet
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    // Disegna la barra orizzontale in basso che si collega al CodeTextEdit
    QPen hp1( QColor( 60, 61, 56 ) ), hp2( QColor( 11, 11, 10 ) );
    p.setPen(hp1);
    //qDebug() << rect();
    p.drawLine( rect().left(), rect().bottom(), rect().right(), rect().bottom() );
    p.setPen(hp2);
    p.drawLine( rect().left(), rect().bottom() - 1, rect().right(), rect().bottom() - 1 );

    // test di disegno di una tab a 0;0 - 50;31
    QPainterPath path;
    path.moveTo(0, 31); // Mi muovo in basso a sx
    path.cubicTo(20,31, 10,0, 30,0);
    path.lineTo(30,31);
    //path.lineTo(0,31);
    p.setPen(QColor(255,255,0));
    //p.setBrush(QColor(122, 163, 39));
    p.setRenderHint(QPainter::Antialiasing);
    p.drawPath(path);
    // usa QPainterPath::intersects per mouse hit test
}
