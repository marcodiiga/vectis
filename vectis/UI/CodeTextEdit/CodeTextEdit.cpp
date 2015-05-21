#include <UI/CodeTextEdit/CodeTextEdit.h>
#include <QPainter>
#include <QResizeEvent>

#include <QDebug>

CodeTextEdit::CodeTextEdit(QWidget *parent) :
  QAbstractScrollArea(parent),
  m_document(nullptr)
{

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

  // Set a font to use in this control
  // Consolas is installed by default on every Windows system, but not Linux. On Linux the
  // preferred one is Monospace. Anyway Qt's matching engine will try to find either or a
  // replacement monospace font
#ifdef _WIN32
  m_monospaceFont.setFamily( "Consolas" );
  m_monospaceFont.setPixelSize(12);
#else
  m_monospaceFont.setFamily( "Monospace" );
#endif
  m_monospaceFont.setStyleHint( QFont::Monospace );
  setFont( m_monospaceFont );

  // Stores the width of a single character in pixels with the given font (cache this value for
  // every document to use it)
  m_characterWidthPixels = fontMetrics().width('A');
}

void CodeTextEdit::loadDocument(Document *doc) {
  m_document = doc;
  // TODO: update the view
}

int CodeTextEdit::getViewportWidth() const {
  return this->viewport()->width();
}

int CodeTextEdit::getCharacterWidthPixels() const {
  return this->m_characterWidthPixels;
}

void CodeTextEdit::paintEvent (QPaintEvent *event) {
  QPainter painter(viewport());

  // Draw control background (this blends with the TabsBar selected tab's bottom color)
  const QBrush backgroundBrush(QColor(39, 40, 34));
  painter.setBrush(backgroundBrush);
  painter.fillRect(this->rect(), backgroundBrush);

  // DEBUG drawing code

  painter.setPen(QPen(Qt::white)); // TODO These colors suck. Find something better.
  auto setColor = [&painter](Style s) {
    switch(s) {
    case Comment: {
      painter.setPen(QPen(Qt::gray));
    } break;
    case Keyword: {
      painter.setPen(QPen(Qt::blue));
    } break;
    case QuotedString: {
      painter.setPen(QPen(Qt::red));
    } break;
    case Identifier: {
      painter.setPen(QPen(Qt::green));
    } break;
    case FunctionCall: {
      painter.setPen(QPen(Qt::cyan));
    } break;
    case Literal: {
      painter.setPen(QPen(Qt::darkRed));
    } break;
    default: {
      painter.setPen(QPen(Qt::white));
    } break;
    };
  };


  // ps. to relink the scrollbar:
  // 1) documentSizeChanged should be emitted every time the document has a different number of editor lines
  // 2) scrolling is just about drawing offsets, have the offsets set by the scrollbar be reflected here
  // 3) if you want a minimap, draw everything. But do NOT draw everything each time. We just need to render the lines that are in sight




  QPointF startpoint(5, 20);
  size_t documentRelativePos = 0;
  size_t lineRelativePos = 0;
  auto& styleIt = m_document->m_styleDb.styleSegment.begin();
  auto& styleEnd = m_document->m_styleDb.styleSegment.end();
  size_t nextDestination = -1;


  auto calculateNextDestination = [&]() {
    // We can have 2 cases here:
    // 1) Our position hasn't still reached a style segment (apply regular style and continue)
    // 2) Our position is exactly on the start of a style segment (apply segment style and continue)
    // If there are no other segments, use a regular style and continue till the end of the lines

    if (styleIt == styleEnd) { // No other segments
      nextDestination = -1;
      setColor( Normal );
      return;
    }

    if(styleIt->start > documentRelativePos) { // Case 1
      setColor( Normal );
      nextDestination = styleIt->start;
    } else if (styleIt->start == documentRelativePos) { // Case 2
      setColor( styleIt->style );
      nextDestination = styleIt->start + styleIt->count;
      ++styleIt; // This makes sure our document relative position is never ahead of a style segment
    }
  };

  // First time we don't have a destination set, just find one (if there's any)
  calculateNextDestination();

  for(auto pl : m_document->m_physicalLines) {

    auto el = pl.m_editorLines[0]; // DEBUG - assume only one editorLine

    do {
      startpoint.setX( 5 + lineRelativePos * m_characterWidthPixels );

      // If we don't have a destination OR we can't reach it within our line, just draw the entire line and continue
      if (nextDestination == -1 ||
          nextDestination > documentRelativePos + (el.m_characters.size() - lineRelativePos)) {

        // Multiple lines will have to be rendered, just render this till the end and continue

        int charsRendered = 0;
        if (el.m_characters.size() > 0) { // Empty lines must be skipped
          QString ts(el.m_characters.data() + lineRelativePos, static_cast<int>(el.m_characters.size() - lineRelativePos));
          painter.drawText(startpoint, ts);
          charsRendered = ts.size();
        }

        lineRelativePos = 0; // Next editor line will just start from the beginning
        documentRelativePos += charsRendered + 1 /* Plus a newline */;

        break; // Go and fetch a new line for the next cycle
      } else {

        // We can reach the goal within this line

        int charsRendered = 0;
        if (el.m_characters.size() > 0) { // Empty lines must be skipped
          QString ts(el.m_characters.data() + lineRelativePos, static_cast<int>(nextDestination - documentRelativePos));
          painter.drawText(startpoint, ts);
          charsRendered = ts.size();
        }

        bool addNewLine = false; // Check if this goal also exhausted the current line entirely
        if(nextDestination - documentRelativePos + lineRelativePos == el.m_characters.size()) {
          addNewLine = true;
          lineRelativePos = 0; // Next editor line will just start from the beginning
        } else
          lineRelativePos += charsRendered;

        documentRelativePos += charsRendered + (addNewLine ? 1 : 0); // Just add a newline if we also reached this line's end

        calculateNextDestination(); // Need a new goal

        if (addNewLine)
          break; // Go fetch a new line
      }

    } while(true);

    // Move the rendering cursor (carriage-return)
    startpoint.setY(startpoint.y() + fontMetrics().height());
  }

  QAbstractScrollArea::paintEvent(event);
}
void CodeTextEdit::resizeEvent (QResizeEvent *evt) {
  if (m_document != nullptr)
    m_document->setWrapWidth(evt->size().width());
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
