#include <UI/TabsBar/TabsBar.h>
#include <QPainter>

#include <QtGlobal>
#include <QDebug>
#include <QStyleOption>


void crashMessageOutput(QtMsgType type, const QMessageLogContext &, const QString & str)
{
    switch (type)
    {
        case QtDebugMsg:
            qDebug() << "Debug: " << str;
            break;
        case QtWarningMsg:
            qDebug() << "Warning:" << str;
            break;
        case QtCriticalMsg:
            qDebug() << "Critical:" << str;
            break;
        case QtFatalMsg:
            qDebug() << "Fatal:" << str;
            abort();
    }
}

TabsBar::TabsBar( QWidget *parent )
    : m_parent(parent)
{
    Q_ASSERT( parent );

    // WA_OpaquePaintEvent specifica che ridisegneremo il controllo ogni volta che ce n'è bisogno
    // senza intervento del sistema.
    //setAttribute( Qt::WA_OpaquePaintEvent, false );
    qInstallMessageHandler(crashMessageOutput);
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
    p.drawLine( rect().left(), rect().bottom(), rect().right(), rect().bottom() );
    p.setPen(hp2);
    p.drawLine( rect().left(), rect().bottom() - 1, rect().right(), rect().bottom() - 1 );
}
