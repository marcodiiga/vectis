#include <UI/CodeTextEdit/CodeTextEdit.h>
#include <QPainter>
#include <QResizeEvent>
#include <QRegExp>
#include <QScrollBar>
#include <QTextBlock>
#include <QLabel>
#include <QHBoxLayout>
#include <algorithm>

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
                            padding: 0px; /* Also eliminate padding (needed to avoid QScrollBar spaces) */ \
                            selection-background-color: #49483E; \
                            selection-color: none; \
                          }" );

  // Disable highlighted text brush (i.e. use an empty brush to paint over)
  QPalette p = this->palette();
  p.setBrush(QPalette::HighlightedText, QBrush());
  this->setPalette(p);

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
  margin-left: 3px;
  margin-right: 3px;
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


  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding));
  QLabel *minimap = new QLabel(this);
  minimap->setFixedWidth(150);
  minimap->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  minimap->setAlignment(Qt::AlignRight);
  layout->addWidget(minimap);

  minimap->setStyleSheet("QWidget { \
                         border-style: outset; \
                         border-width: 1px; \
//                         border-color: beige; \
                     }");


  connect(this, &QPlainTextEdit::textChanged, this, [minimap, this]() {
      QSizeF dim = getDocumentDimensions();
      QPixmap document(dim.width(), dim.height());
      document.fill(Qt::transparent);
      getScreenShot(document);
      if(!document.isNull())
        minimap->setPixmap(document.scaled(150, 700, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
  });

}

QFont CodeTextEdit::getMonospaceFont() const {
  return m_monospaceFont;
}

QSizeF CodeTextEdit::getDocumentDimensions() const {
//  auto block = document()->firstBlock();
//  qreal height = 0;
//  qreal width = 0;
//  while (block.isValid()) {
//      QRectF r = blockBoundingRect(block);
//      height += r.height();
//      width = std::max(width, r.width());
//      block = block.next();
//  }

  auto line_height = QFontMetrics(getMonospaceFont()).height();
  return QSizeF(document()->size().width(), document()->size().height() * line_height);
}

void CodeTextEdit::getScreenShot(QPixmap& map) const
{
    QPainter painter(&map);

    int offset = 0;
    auto block = document()->firstBlock();

    while (block.isValid()) {

        QRectF r = blockBoundingRect(block);
        QTextLayout *layout = block.layout();

        if (!block.isVisible()) {

            offset += r.height();
            block = block.next();
            continue;

        } else {
            layout->draw(&painter, QPoint(0, offset));
        }

        offset += r.height();

        block = block.next();
    }
}
