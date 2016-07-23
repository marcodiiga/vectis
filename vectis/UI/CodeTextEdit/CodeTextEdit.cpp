#include <UI/CodeTextEdit/CodeTextEdit.h>
#include <QPainter>
#include <QResizeEvent>
#include <QRegExp>
#include <QScrollBar>
#include <QDebug>
#include <QElapsedTimer>

CodeTextEdit::CodeTextEdit(QWidget *parent) :
  QPlainTextEdit(parent)
{

  Q_ASSERT(parent);

  // WA_OpaquePaintEvent specifies that we'll redraw the control every time it is needed without
  // any system intervention
  // WA_NoSystemBackground avoids the system to draw the background (we'll handle it as well)
  //setAttribute( Qt::WA_OpaquePaintEvent, true );
  setFrameShape( QFrame::NoFrame ); // No widget border allowed (otherwise there would be a separation
                                    // line that doesn't allow this control to blend in with tabs)
  setStyleSheet( "QWidget { background-color: #272822;     \
                            color : white;                       \
                            padding: 0px; }" ); // Also eliminate padding (needed to avoid QScrollBar spaces)

#ifdef _WIN32
  m_monospaceFont.setFamily( "Consolas" );
  m_monospaceFont.setPixelSize(15);
#else
  m_monospaceFont.setFamily( "Monospace" );
  m_monospaceFont.setPixelSize(15);
#endif
  m_monospaceFont.setFixedPitch( true );
  m_monospaceFont.setStyleHint( QFont::Monospace );

  setFont( m_monospaceFont );

  verticalScrollBar()->setStyleSheet(
R"(

QScrollBar:vertical {
  border: 0px;
  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgb(33,33,33), stop:1 rgb(55,55,55));
}

/* Style the handle as an ellipse with some left and right margin */
QScrollBar::handle:vertical {
  margin-left: 4px;
  margin-right: 4px;
  border-radius: 4px;
  background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgb(88,88,88), stop:1 rgb(64,64,64));
  min-height: 20px;
}

/* Bar background gradient */
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgb(33,33,33), stop:1 rgb(55,55,55));
}

/* Hide the buttons completely */
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
  border: none;
  background: none;
  height: 0px;
}

  )");
}
