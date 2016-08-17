#include <UI/CodeTextEdit/CodeTextEdit.h>
#include <UI/Utils.h>
#include <QPainter>
#include <QResizeEvent>
#include <QRegExp>
#include <QScrollBar>
#include <QTextBlock>
#include <QLabel>
#include <QHBoxLayout>
#include <QSyntaxHighlighter>
#include <algorithm>

#include <QDebug>
#include <QElapsedTimer>

//class QTextDocumentSub : public QTextDocument {
//public:
//  QTextDocumentSub(QObject *parent = Q_NULLPTR) : QTextDocument(parent) {}

//  void QTextDocumentSub::drawContents(QPainter *p, const QRectF &rect = QRectF()) {
//    qDebug() << "Hello!!";
//  }
//};

class MiniMap : public QLabel {
public:
  MiniMap(CodeTextEdit *parent = nullptr) :
    m_parent(parent),
    QLabel(parent) {
    setFixedWidth(WIDTH);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setAlignment(Qt::AlignRight);
    this->setStyleSheet("QPlainTextEdit { \
                             border-style: outset; \
                             border-width: 1px; \
                             border-color: red; \
                           } \
    ");
    setMouseTracking(true);
    //setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  }

//  void setDocument(QTextDocument *document) {
//    m_plain_text_edit.setDocument(document);
//    connect(m_plain_text_edit.document(), &QTextDocument::contentsChanged, [&]() {
//      QSizeF dim = m_parent->getDocumentDimensions();
//      QPixmap shot = m_plain_text_edit.grab(QRect(0, 0, dim.width(), dim.height()));
//      this->setPixmap(shot);
//    });
//  }

  void mousePressEvent(QMouseEvent *ev) {
    if (m_hovering) {
      m_dragging = true;
      m_start_dragging_pos = ev->pos();

      auto line_height = QFontMetrics(m_parent->getMonospaceFont()).height();
      m_start_dragging_scrollbar_pos = m_parent->getVScrollbarPos() / line_height;
    } else
      m_dragging = false;
  }

  void mouseReleaseEvent(QMouseEvent *ev) {
    m_dragging = false;
  }

  void enterEvent(QEvent *event) {
    m_hovering = true;
    draw_viewport_placeholder();
  }

  void leaveEvent(QEvent *event) {
    if (m_hovering) {
      draw_document_pixmap(); // Restore (i.e. clear placeholder)
      m_hovering = false;
    }
  }

  void draw_viewport_placeholder() {
    if (!m_hovering)
      return;
    int viewport_y = m_parent->getVScrollbarPos();
    int viewport_height = m_parent->viewport()->height();
    QPixmap rectangle(MiniMap::WIDTH, viewport_height);
    rectangle.fill(QColor(50, 50, 50, 160));
    QPixmap new_pixmap = this->pixmap()->copy();
    QPainter paint(&new_pixmap);

    auto dim = m_parent->getDocumentDimensions();
    // dim.width() : MiniMap::WIDTH = viewport_height : scaled_viewport_height
    int scaled_viewport_height = (MiniMap::WIDTH * viewport_height) / dim.width();
    // dim.width() : MiniMap::WIDTH = viewport_y : scaled_viewport_y
    int scaled_viewport_y = (MiniMap::WIDTH * viewport_y) / dim.width();

    paint.drawPixmap(0, scaled_viewport_y - m_document_pixmap_offset, MiniMap::WIDTH, scaled_viewport_height, rectangle);
    QLabel::setPixmap(new_pixmap);
  }

  void mouseMoveEvent(QMouseEvent *ev) {
    if (!m_dragging)
      return;

    auto viewport_height = m_parent->viewport()->height();
    auto dim = m_parent->getDocumentDimensions();

    // Delta from original mouse position
    float delta = ev->pos().y() - m_start_dragging_pos.y();
    float percentage = delta / viewport_height;

    auto line_height = QFontMetrics(m_parent->getMonospaceFont()).height();
    auto document_delta = (dim.height() / line_height) * percentage;
    // Scaled delta to minimap height
    // dim.width() : MiniMap::WIDTH = dim.height() : scaled_delta
    //auto scaled_delta = (MiniMap::WIDTH * dim.height()) / dim.width();
    // Additional delta to have the document scrolled entirely in the minimap's height area
    // delta : viewport_height = document_pos : document_height
    //auto additional_delta = (delta * dim.height()) / viewport_height;

    qDebug() << percentage;
\
    m_parent->verticalScrollBar()->setValue(m_start_dragging_scrollbar_pos + document_delta);


    int scaled_document_delta = (MiniMap::WIDTH * document_delta) / dim.width();
    m_document_pixmap_offset = scaled_document_delta;

    draw_document_pixmap(); // Restore (i.e. clear placeholder)
    draw_viewport_placeholder(); // Apply new placeholder at mouse position
  }

  void reset_highlighter(QString extension) {
    m_highlighter = std::move(getSyntaxHighlighterFromExtension(extension));
  }

  void setPixmap(const QPixmap& pixmap) {

    m_document_pixmap = pixmap.copy();

    draw_document_pixmap();
  }

  void draw_document_pixmap() { // Draws document pixmap with the specified offset (and no hover rectangle)

    QPixmap pix(m_document_pixmap.width(), m_document_pixmap.height());
    pix.fill(Qt::transparent);
    QPainter paint(&pix);

    paint.drawPixmap(0, -m_document_pixmap_offset, pix.width(), pix.height(), m_document_pixmap);

    QLabel::setPixmap(pix);
  }

  QPixmap& map() {
    return m_map;
  }

  QVector<QTextBlock>& blocks() {
    return m_blocks;
  }

  QVector<QPixmap>& blocks_pixmaps() {
    return m_blocks_pixmaps;
  }

static constexpr const int WIDTH = 150;

  QPixmap m_document_pixmap; // Unmodified document pixmap without hover rectangles or translations
  float m_document_pixmap_offset = 0.f; // Y offset for the document pixmap to be shown
  QPixmap m_old_pixmap_before_hovering;
  bool m_hovering = false;
  bool m_dragging = false;
  QPoint m_start_dragging_pos;
  int m_start_dragging_scrollbar_pos;
  QPixmap m_map;
  QVector<QTextBlock> m_blocks;
  QVector<QPixmap> m_blocks_pixmaps;


  CodeTextEdit *m_parent = nullptr;
  QPlainTextEdit m_plain_text_edit;
  std::unique_ptr<QSyntaxHighlighter> m_highlighter;

};

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


  // |------------------------------|
  // |                              |
  // | <-- spacer --> <- minimap -> |
  // |                              |
  // |------------------------------|
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding));
  m_minimap = new MiniMap(this);
  layout->addWidget(m_minimap);

  //getScreenShot(m_minimap->map());



}

void CodeTextEdit::setDocument(QTextDocument *document) {

 // QTextDocumentSubdocument->clone()

  QPlainTextEdit::setDocument(document); // Continue with base class handling



//  auto seta = this->document()->clone(m_minimap);
//  QPlainTextDocumentLayout *layout = new QPlainTextDocumentLayout(seta);
//  seta->setDocumentLayout(layout);

//  QVariant syntax_highlighter = document->property("syntax_highlighter");
//  if (!syntax_highlighter.isNull()) {
//    m_minimap->reset_highlighter(syntax_highlighter.toString());
//  }
//  m_minimap->m_highlighter->setDocument(seta);
//  m_minimap->setDocument(seta);
//  QFont font;
//  font.setFamily( "Consolas" );
//  font.setPixelSize(5);
//  m_minimap->setFont(font);


//    QSizeF dim = getDocumentDimensions();
//    QPixmap document_pixmap(dim.width(), dim.height());
//    document_pixmap.fill(Qt::transparent);
//    this->render(&document_pixmap);
//    m_minimap->setPixmap(document_pixmap.scaled(150, 700, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));


  connect(this->document(), &QTextDocument::contentsChanged, this, [&]() {
    // Apply plain text layout and forward to minimap
    //auto new_document = this->document()->clone(m_minimap);
    //this->document()->documentLayout()->
    //QPlainTextDocumentLayout *layout = new QPlainTextDocumentLayout(new_document);
    //new_document->setDocumentLayout(layout);
    //auto document = this->document()->clone(m_minimap);
    //document->setDocumentLayout(this->document()->documentLayout());

    QSizeF dim = getDocumentDimensions();
    QRect rect(0, 0, dim.width(), dim.height());

    QPixmap pixmap(rect.width(), rect.height());
    pixmap.fill(Qt::transparent);

    QRegion region(rect);
    //this->render(&pixmap, QPoint(0, 0), region);
    renderDocument(pixmap);
    //pixmap.save("test.png");

    Qt::AspectRatioMode aspect_ratio_mode = Qt::KeepAspectRatioByExpanding;
    if (dim.height() / m_minimap->height() <= dim.width() / MiniMap::WIDTH) {
      qDebug() << "SWITCHING";
      aspect_ratio_mode = Qt::KeepAspectRatio;
    }
    m_minimap->setPixmap(pixmap.scaled(MiniMap::WIDTH, m_minimap->height(), aspect_ratio_mode, Qt::SmoothTransformation));

    //m_minimap->document()->setPlainText(this->document()->toPlainText());
  //  m_minimap->setDocument(this->document());
//    QFont font;
//    font.setFamily( "Consolas" );
//    font.setPixelSize(5);
//    m_minimap->setFont(font);

  });


//  QSizeF dim = getDocumentDimensions();
//  QPixmap document_pixmap(dim.width(), dim.height());
//  document_pixmap.fill(Qt::transparent);
//  renderDocument(document_pixmap);
//  if(!document_pixmap.isNull())
//    m_minimap->setPixmap(document_pixmap.scaled(150, 700, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));


//  QTextBlock block = document()->firstBlock();
//  block.layout()

//  connect(this->document(), &QTextDocument::contentsChanged , this, [&]() {


//    //this->document()->availableUndoSteps();
//    //qDebug() << "QAbstractTextDocumentLayout::documentSizeChanged";


//      QSizeF dim = getDocumentDimensions();
//      const int height = dim.height();
//      const int width = std::max((int)dim.width(), this->viewport()->width());
//      QPixmap document_pixmap(width, height);
//      document_pixmap.fill(Qt::transparent);
//      QPainter document_painter(&document_pixmap);

//      QVector<QTextBlock> new_blocks;
//      QVector<QPixmap> new_blocks_pixmaps;
//      bool different_blocks = false;
//      int render_y_offset = 0;
//      int j = 0; // Index into our stored blocks
//      for(auto i = 0; i < this->document()->blockCount(); ++i) {

//        QTextBlock& block = this->document()->findBlockByNumber(i);
//        QRectF block_br = blockBoundingRect(block);

//        if (j >= m_minimap->blocks().size() || m_minimap->blocks()[j] != block) {
//          // Different, repaint
//          QPixmap block_pixmap(block_br.width(), block_br.height());
//          block_pixmap.fill(Qt::transparent);
//          QPainter block_painter(&block_pixmap);

//          renderBlock(block_painter, block);

//          new_blocks.append(block);
//          new_blocks_pixmaps.append(block_pixmap);

//          document_painter.drawPixmap(0, render_y_offset, block_pixmap);

//          different_blocks = true;
//        } else {
//          // Valid index and block

//          new_blocks.append(m_minimap->blocks()[j]);
//          new_blocks_pixmaps.append(m_minimap->blocks_pixmaps()[j]);

//          document_painter.drawPixmap(0, render_y_offset, m_minimap->blocks_pixmaps()[j]);

//          ++j;
//        }
//        render_y_offset += block_br.height();
//      }

//      if (different_blocks) {
//        m_minimap->blocks().swap(new_blocks);
//        m_minimap->blocks_pixmaps().swap(new_blocks_pixmaps);
//      }

//      m_minimap->setPixmap(document_pixmap.scaled(MiniMap::WIDTH, dim.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

////       QPixmap document_pixmap(width, height);
//   //   if (m_minimap->map().width() != width || m_minimap->height() != height)
//  //      m_minimap->map().scaledToHeight(203, Qt::TransformationMode::)
//  //    m_minimap->map().fill(Qt::transparent);
////      renderDocument(document_pixmap);
////      //if(!document.isNull())
////      m_minimap->setPixmap(document_pixmap.scaled(MiniMap::WIDTH, dim.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
//  });



//  connect(this, &QPlainTextEdit::textChanged, this, [minimap, this]() {
//      QSizeF dim = getDocumentDimensions();
//      QPixmap document(dim.width(), dim.height());
//      document.fill(Qt::transparent);
//      getScreenShot(document);
//      if(!document.isNull())
//        minimap->setPixmap(document.scaled(150, 700, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
  //  });
}

int CodeTextEdit::getVScrollbarPos() const {
  auto line_height = QFontMetrics(getMonospaceFont()).height();
  return ((float)this->verticalScrollBar()->value() * line_height);
}

void CodeTextEdit::paintEvent(QPaintEvent *e)
{

  QPlainTextEdit::paintEvent(e);


  //pixmap.fill(Qt::transparent);




  //QPainter::restoreRedirected(viewport());

 // QSizeF dim = getDocumentDimensions();
 // QImage bitmap(dim.width(), dim.height(), QImage::Format_ARGB32);
 // bitmap.fill(Qt::transparent);
 // QPainter painter(&bitmap);
 // this->render(&painter, QPoint(), QRegion(), 0);
//this->grab()

//    QPixmap document_pixmap(dim.width(), dim.height());
//    document_pixmap.fill(Qt::red);
//    this->render(&document_pixmap);



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

// Expensive - renders the entire document on the given pixmap
void CodeTextEdit::renderDocument(QPixmap& map) const
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

void CodeTextEdit::renderBlock(QPainter& painter, const QTextBlock& block) const
{
  if (block.isValid()) {

      QRectF r = blockBoundingRect(block);
      QTextLayout *layout = block.layout();

      if (block.isVisible()) {

          layout->draw(&painter, QPoint(0, 0));
      }
  }
}
