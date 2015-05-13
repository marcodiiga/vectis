#include <UI/CodeTextEdit/CodeTextEdit.h>

CodeTextEdit::CodeTextEdit(QWidget *parent) :
    QAbstractScrollArea(parent) {

    Q_ASSERT(parent);

    // WA_OpaquePaintEvent specifies that we'll redraw the control every time it is needed without
    // any system intervention
    setAttribute( Qt::WA_OpaquePaintEvent, false );

    // Create the vertical scrollbar and set it as "always on"
    m_verticalScrollBar = std::make_unique<ScrollBar>( this );
    this->setVerticalScrollBar( m_verticalScrollBar.get() );
    setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );

    //    m_customCodeEdit->verticalScrollBar()->setStyleSheet(QString("\
    //                                                                  QScrollBar:vertical {\
    //                                                                    width:15px;\
    //                                                                  }"));

}


void CodeTextEdit::paintEvent(QPaintEvent *) {
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
