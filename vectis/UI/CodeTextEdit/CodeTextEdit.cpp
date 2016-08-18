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
#include <QApplication>
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
    /*this->setStyleSheet("QLabel { \
                             border-style: outset; \
                             border-width: 1px; \
                             border-color: red; \
                           } \
    ");*/
    //setMouseTracking(true);
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

  void mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    m_dragging = false;
  }

  void enterEvent(QEvent *event) {
    Q_UNUSED(event);
    m_hovering = true;
    draw_viewport_placeholder();
  }

  void leaveEvent(QEvent *event) {
    Q_UNUSED(event);
    if (m_hovering || m_dragging) {
      draw_document_pixmap(); // Restore (i.e. clear placeholder)
      m_hovering = false;
      m_dragging = false;
      //m_running_delta += m_last_valid_delta;
      //qDebug() << "[SAVED RUNNING DELTA]";
    }
  }

  void draw_viewport_placeholder() {
    if (!m_hovering)
      return;
    int viewport_y = m_parent->getVScrollbarPos();
    int viewport_height = m_parent->viewport()->height();

    QPixmap new_pixmap = this->pixmap()->copy();
    QPainter paint(&new_pixmap);

    auto dim = m_parent->getDocumentDimensions();
    // dim.width() : MiniMap::WIDTH = viewport_height : scaled_viewport_height
    float scaled_viewport_height = ((float)MiniMap::WIDTH * viewport_height) / dim.width();
    // dim.width() : MiniMap::WIDTH = viewport_y : scaled_viewport_y
    float scaled_viewport_y = ((float)MiniMap::WIDTH * viewport_y) / dim.width();

    QPixmap rectangle(MiniMap::WIDTH, scaled_viewport_height);
    rectangle.fill(QColor(50, 50, 50, 160));

    auto offset = -m_document_pixmap_offset;

    float scaled_doc_height = ((float)MiniMap::WIDTH * dim.height()) / dim.width();
    if (scaled_doc_height < this->height())
      offset = 0; // Do not move the pixmap if it can safely be drawn into our area

    // Draw the viewport area placeholder    
    paint.drawPixmap(0, scaled_viewport_y + offset, MiniMap::WIDTH, scaled_viewport_height, rectangle);
    QLabel::setPixmap(new_pixmap);
  }

  void mouseMoveEvent(QMouseEvent *ev) {
    if (!m_dragging)
      return;

    auto dim = m_parent->getDocumentDimensions();
    auto viewport_height = m_parent->viewport()->height();
    auto scaled_viewport_height = ((float)MiniMap::WIDTH * viewport_height) / dim.width();


    // Delta from original mouse position
    float delta = ev->pos().y() - m_start_dragging_pos.y();
    float percentage = delta / (this->height() - scaled_viewport_height);
    percentage = clamp(percentage, -1.f, 1.f);

    auto line_height = QFontMetrics(m_parent->getMonospaceFont()).height();

    // Add a scroll-acceleration factor if the document is very small
    float factor = 1.f;
    float scaled_doc_height = ((float)MiniMap::WIDTH * dim.height()) / dim.width();
    if (scaled_doc_height < this->height())
      factor = this->height() / scaled_doc_height; // Do not move the pixmap if it can safely be drawn into our area

    auto document_delta = ((dim.height() - m_parent->viewport()->height()) / line_height) * percentage * factor;
    // Scaled delta to minimap height
    // dim.width() : MiniMap::WIDTH = dim.height() : scaled_delta
    //auto scaled_delta = (MiniMap::WIDTH * dim.height()) / dim.width();
    // Additional delta to have the document scrolled entirely in the minimap's height area
    // delta : viewport_height = document_pos : document_height
    //auto additional_delta = (delta * dim.height()) / viewport_height;
\
    int new_scrollbar_value = m_start_dragging_scrollbar_pos + document_delta;
    if (!m_parent->verticalScrollBar()->isVisible() ||
        new_scrollbar_value < m_parent->verticalScrollBar()->minimum() ||
        new_scrollbar_value > m_parent->verticalScrollBar()->maximum())
      return;

    //qDebug() << new_scrollbar_value;

    //qDebug() << m_parent->verticalScrollBar()->value();

    //if (new_scrollbar_value < m_parent->verticalScrollBar()->minimum())
      //m_document_pixmap_offset = 0;
    //else if (new_scrollbar_value > m_parent->verticalScrollBar()->maximum()) {
     // float scaled_doc_height = ((float)MiniMap::WIDTH * dim.height()) / dim.width();
      //m_document_pixmap_offset = scaled_doc_height - this->height();
    //} else {
      m_parent->verticalScrollBar()->setValue(new_scrollbar_value);

      updatePixmapOffsetFromScrollbar(percentage);
      //qDebug() << "Last m_document_pixmap_offset: " << m_document_pixmap_offset << " with scaled_viewport_y = " <<
      //            scaled_viewport_y << " and m_running_delta = "  << m_running_delta;
   // }

      //qDebug() << m_document_pixmap_offset;

    draw_document_pixmap(); // Restore (i.e. clear placeholder)
    draw_viewport_placeholder(); // Apply new placeholder at mouse position
  }

//  void reset_highlighter(QString extension) {
//    m_highlighter = getSyntaxHighlighterFromExtension(extension);
//    m_highlighter->setParent(this);
//  }

  void updatePixmapOffsetFromScrollbar(float percentage = 0.f /* additional percentage as supplied by a mouse offset */) {
    // Update map and placeholder deltas
    auto dim = m_parent->getDocumentDimensions();
    auto viewport_height = m_parent->viewport()->height();
    auto scaled_viewport_height = ((float)MiniMap::WIDTH * viewport_height) / dim.width();
    float viewport_y = m_parent->getVScrollbarPos();
    float scaled_viewport_y = ((float)MiniMap::WIDTH * viewport_y) / dim.width();

    float m_running_delta = (
                        // The percentage of the scrollbar itself (i.e. position where we were)
                        (m_start_dragging_scrollbar_pos / m_parent->verticalScrollBar()->maximum())
                        + percentage // Mouse delta percentage in the total scrollable area
                       ) * (this->height() - scaled_viewport_height);

//      qDebug() << "((float)m_start_dragging_scrollbar_pos / m_parent->verticalScrollBar()->maximum()): "
//               << ((float)m_start_dragging_scrollbar_pos / m_parent->verticalScrollBar()->maximum()) << "\n"
//               << "percentage:" << percentage << "\n"
//               << "(this->height()- scaled_viewport_height):" << (this->height()- scaled_viewport_height) << "\n";

    //int scaled_document_delta = (MiniMap::WIDTH * document_delta) / dim.width();
    m_document_pixmap_offset = scaled_viewport_y - m_running_delta;

  }

  void setPixmap(const QPixmap& pixmap) {

    m_document_pixmap = pixmap.copy();

    draw_document_pixmap();
  }

  void draw_document_pixmap() { // Draws document pixmap with the specified offset (and no hover rectangle)

    QPixmap pix(m_document_pixmap.width(), m_document_pixmap.height());
    pix.fill(Qt::transparent);
    QPainter paint(&pix);

    // Draw the document pixmap
    //qDebug() << "-m_document_pixmap_offset = " << -m_document_pixmap_offset;

    auto offset = -m_document_pixmap_offset;

    auto dim = m_parent->getDocumentDimensions();
    float scaled_doc_height = ((float)MiniMap::WIDTH * dim.height()) / dim.width();
    if (scaled_doc_height < this->height())
      offset = 0; // Do not move the pixmap if it can safely be drawn into our area

    paint.drawPixmap(0, offset, pix.width(), pix.height(), m_document_pixmap);

    QLabel::setPixmap(pix);
  }

  void clear_document_pixmap() { // Cleanup
    QPixmap pix(MiniMap::WIDTH, 20);
    pix.fill(Qt::transparent);
    QLabel::setPixmap(pix);
    m_document_pixmap = pix;
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
  //float m_running_delta = 0.f;
  //float m_last_valid_delta = 0.f;
  QPixmap m_old_pixmap_before_hovering;
  bool m_hovering = false;
  bool m_dragging = false;
  QPoint m_start_dragging_pos;
  float m_start_dragging_scrollbar_pos = 0.f;
  QPixmap m_map;
  QVector<QTextBlock> m_blocks;
  QVector<QPixmap> m_blocks_pixmaps;


  CodeTextEdit *m_parent = nullptr;
  //QPlainTextEdit m_plain_text_edit;
  //std::unique_ptr<QSyntaxHighlighter> m_highlighter;

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
                          } \
  " );
  setViewportMargins(0, 0, MiniMap::WIDTH, 0); // Prevent text from going under the minimap
  setAcceptDrops(false);
  viewport()->setAttribute(Qt::WA_KeyCompression, true);

  // Disable highlighted text brush (i.e. use an empty brush to paint over)
  QPalette p = this->palette();
  p.setBrush(QPalette::HighlightedText, QBrush());
  p.setColor(QPalette::Text, Qt::white);
  p.setColor(QPalette::WindowText, Qt::white);
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

  // Handler for scrollbar scroll (i.e. update the minimap as well)
  connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [&](int) {
    //qDebug() << "value changed";
    if (!m_minimap->m_dragging) {
      m_minimap->m_start_dragging_scrollbar_pos = verticalScrollBar()->value();
      m_minimap->updatePixmapOffsetFromScrollbar();
      m_minimap->draw_document_pixmap();
      if (m_minimap->m_hovering)
        m_minimap->draw_viewport_placeholder();
    }
  });

  // Install filter to reroute events we're interested in
  m_minimap->installEventFilter(this);
  this->installEventFilter(this);

  // Callback for too-fast-typing delay
  connect(&m_regenerate_minimap_delay, &QTimer::timeout, this, [&]() {
    this->regenerateMiniMap();
  });
  m_regenerate_minimap_delay.setInterval(200);
  m_regenerate_minimap_delay.setSingleShot(true);
}

bool CodeTextEdit::eventFilter(QObject *target, QEvent *event) {
  if(target == m_minimap ) {
      if (event->type() == QEvent::Wheel) {
        this->wheelEvent((QWheelEvent*)event);
        return true;
      } else {
        event->ignore();
        return false;
      }
  }
  if (target == this && event->type() == QEvent::KeyPress) {
    QKeyEvent* e = static_cast<QKeyEvent*>(event);
    if (e->key() == Qt::Key_Tab && e->modifiers() == Qt::NoModifier) { // Tab -> 4 spaces
      QKeyEvent *spaces_event = new QKeyEvent ( QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, "    ", false, 0 );
      QApplication::postEvent (this, spaces_event);
      return true;
    }
  }

  return QPlainTextEdit::eventFilter(target, event);
}

void CodeTextEdit::setDocument(QTextDocument *document, int scrollbar_pos) {

  this->setEnabled(true);

  m_last_document_modification = QDateTime::currentDateTime();

  QPlainTextEdit::setDocument(document); // Continue with base class handling

  this->verticalScrollBar()->setValue(scrollbar_pos);

  m_first_time_redraw = true;

  connect(this->document(), &QTextDocument::contentsChanged, this, [&]() {

    // TODO: this might as well be moved into another thread. For now, it isn't much of a big deal.
    if (m_first_time_redraw == false && m_last_document_modification.msecsTo(QDateTime::currentDateTime()) < 200) {
      // We're receiving lots of document modifications in a limited amount of time,
      // slow down with the minimap regeneration
      qDebug() << "SLOW DOWN!\n";
      m_regenerate_minimap_delay.start();
      m_last_document_modification = QDateTime::currentDateTime();
    } else {
      m_regenerate_minimap_delay.stop();
      qDebug() << "Regenerating..\n";
      this->regenerateMiniMap();
      m_first_time_redraw = false;
      m_last_document_modification = QDateTime::currentDateTime();
    }

  });
}

void CodeTextEdit::unloadDocument() {

    //delete this->document();
  this->document()->disconnect(this);
  QPlainTextEdit::setDocument(nullptr);
  m_minimap->clear_document_pixmap();
  this->setEnabled(false);
}

void CodeTextEdit::regenerateMiniMap() {
  QSizeF dim = getDocumentDimensions();
  QRect rect(0, 0, dim.width(), dim.height());
  //qDebug() << rect;

  QPixmap pixmap(rect.width(), rect.height());
  pixmap.fill(Qt::transparent);

  QRegion region(rect);
  //this->render(&pixmap, QPoint(0, 0), region);
  renderDocument(pixmap);
  //pixmap.save("test.png");

  Qt::AspectRatioMode aspect_ratio_mode = Qt::KeepAspectRatioByExpanding;
  // KeepAspectRatioByExpanding keeps the width scaled and expands on the height, but this only
  // works if the document_width / minimap_width ratio is greater than document_height / minimap_height,
  // otherwise the height is kept scaled and the width is expanded (not what we want). Therefore we
  // switch to a KeepAspectRatio (both width and height are resized uniformly) when we're sure we can
  // fit the width (adjusted with adjustSize anyway)
  if (dim.height() / m_minimap->height() <= dim.width() / MiniMap::WIDTH)
    aspect_ratio_mode = Qt::KeepAspectRatio;
  m_minimap->setPixmap(pixmap.scaled(MiniMap::WIDTH, m_minimap->height(), aspect_ratio_mode, Qt::SmoothTransformation));

  m_minimap->m_start_dragging_scrollbar_pos = verticalScrollBar()->value();
  m_minimap->updatePixmapOffsetFromScrollbar();
  m_minimap->draw_document_pixmap();
  if (m_minimap->m_hovering)
    m_minimap->draw_viewport_placeholder();
}

float CodeTextEdit::getVScrollbarPos() const {
  auto line_height = QFontMetrics(getMonospaceFont()).height();
  return ((float)this->verticalScrollBar()->value() * line_height);
}

void CodeTextEdit::paintEvent(QPaintEvent *e) {
  QPlainTextEdit::paintEvent(e);
}

void CodeTextEdit::resizeEvent(QResizeEvent *e) {

  QPlainTextEdit::resizeEvent(e);

  m_regenerate_minimap_delay.stop();
  m_regenerate_minimap_delay.start();
}

QFont CodeTextEdit::getMonospaceFont() const {
  return m_monospaceFont;
}

QSizeF CodeTextEdit::getDocumentDimensions() const {

  auto line_height = QFontMetrics(getMonospaceFont()).height();
  //this->document()->adjustSize(); // document()->size().width() returns the viewport width (i.e. much wider than
                                  // the actual contained text)
  //auto ideal_width = document()->idealWidth();
  //auto char_width = QFontMetrics(getMonospaceFont()).width("A");
  //qDebug() << document()->size().width();
  auto width = document()->size().width();
  if (width < 200)
    width = 200;
  return QSizeF(width, document()->size().height() * line_height);
}

// Expensive - renders the entire document on the given pixmap
void CodeTextEdit::renderDocument(QPixmap& map) const
{
    QPainter painter(&map);

    int offset = 0;
    document()->adjustSize();
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
