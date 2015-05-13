#include <UI/CodeTextEdit/CodeTextEdit.h>
#include <QPainter>
#include <QFile>

CodeTextEdit::CodeTextEdit(QWidget *parent) :
    QAbstractScrollArea(parent) {

    Q_ASSERT(parent);

    // WA_OpaquePaintEvent specifies that we'll redraw the control every time it is needed without
    // any system intervention
    // WA_NoSystemBackground avoids the system to draw the background (we'll handle it as well)
    setAttribute( Qt::WA_OpaquePaintEvent, true );
    setAttribute( Qt::WA_NoSystemBackground, true );
    setFrameShape( QFrame::NoFrame ); // No widget border allowed (otherwise there would be a separation
                                      // line that doesn't allow this control to blend in with tabs)

    // Create the vertical scrollbar and set it as "always on"
    m_verticalScrollBar = std::make_unique<ScrollBar>( this );
    this->setVerticalScrollBar( m_verticalScrollBar.get() );
    setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
}

// Loads a text or code file from a path. Returns true on success
bool CodeTextEdit::loadFile (QString file) {
//    QFile f(file);
//    if (f.open(QFile::Text) == false)
//        return false;
//        QTextStream in(&f);
//        qDebug() << f.size() << in.readAll();
    return false;
}


void CodeTextEdit::paintEvent (QPaintEvent *event) {
    QPainter painter(viewport());

    // Draw control background (this blends with the TabsBar selected tab's bottom color)
    const QBrush backgroundBrush(QColor(39, 40, 34));
    painter.setBrush(backgroundBrush);
    painter.fillRect(this->rect(), backgroundBrush);

    QAbstractScrollArea::paintEvent(event);
}

/*

Design for the code editor (main component for code editing)
------------------------------------------------------------



// We can't directly subclass a QPlainTextEdit, we need to render everything in the control.
class Control : public QAbstractScrollArea {
  paintEvent() {
    QPainter painter(this->viewport());
    
    foreach visible line (only draw a subset of the file)
      DrawLine()
  }
  DrawLine() {
    // There will be multiple phases for drawing, e.g. background then edges, then text..
    draw every word in the line according to its formatting (a lexer might read keywords and the
      style might be set afterwards) with clipping (nowrap) or without clipping (wrap)
  }
};








*/
