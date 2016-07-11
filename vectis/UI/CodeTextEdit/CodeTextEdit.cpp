#include <UI/CodeTextEdit/CodeTextEdit.h>
#include <QPainter>
#include <QResizeEvent>

#include <QDebug>
#include <QElapsedTimer>

CodeTextEdit::CodeTextEdit(QWidget *parent) :
  QAbstractScrollArea(parent),
  m_caretAlphaInterpolator(*this)
{

  Q_ASSERT(parent);

  // WA_OpaquePaintEvent specifies that we'll redraw the control every time it is needed without
  // any system intervention
  // WA_NoSystemBackground avoids the system to draw the background (we'll handle it as well)
  setAttribute( Qt::WA_OpaquePaintEvent, true );
  setAttribute( Qt::WA_NoSystemBackground, true );
  setFrameShape( QFrame::NoFrame ); // No widget border allowed (otherwise there would be a separation
                                    // line that doesn't allow this control to blend in with tabs)
  setStyleSheet( "QWidget { background-color: rgb(22,23,19);     \
                            padding: 0px; }" ); // Also eliminate padding (needed to avoid QScrollBar spaces)

  // Create the vertical scrollbar and set it as "always on"
  m_verticalScrollBar = std::make_unique<ScrollBar>( this );
  this->setVerticalScrollBar( m_verticalScrollBar.get() );
  this->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );

  // Default values for mouse wheel and pgUp/Down
  this->verticalScrollBar()->setSingleStep(2);
  this->verticalScrollBar()->setPageStep(65);

  // Set a font to use in this control
  // Consolas is installed by default on every Windows system, but not Linux. On Linux the
  // preferred one is Monospace. Anyway Qt's matching engine will try to find either or a
  // replacement monospace font
#ifdef _WIN32
  m_monospaceFont.setFamily( "Consolas" );
  m_monospaceFont.setPixelSize(14);  
#else
  m_monospaceFont.setFamily( "Monospace" );
  m_monospaceFont.setPixelSize(14);
#endif
  m_monospaceFont.setFixedPitch( true );
  m_monospaceFont.setStyleHint( QFont::Monospace );
  m_monospaceFont.setStyleStrategy( QFont::ForceIntegerMetrics ); // IMPORTANT! Otherwise sub-pixel hinting will
                                                                  // screw all of our integer font metrics calculations
  setFont( m_monospaceFont );

  // Stores the width of a single character in pixels with the given font (cache this value for
  // every document to use it)
  m_characterWidthPixels = fontMetrics().width('A');
  /*
   * Here is how the font metrics work
   *
   * ----------------------------------------- Top
   * _________________________________ Ascent
   *               C
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Baseline
   * _________________________________ Descent
   *
   * ----------------------------------------- Bottom
   * (leading)
   *
   * Notice that when drawText() is called with a position, that indicates the baseline
   * of a character, therefore we're also subtracting the descent() before rendering
   *
   */
  m_characterDescentPixels = fontMetrics().descent();

  // Configure the caret animation
  m_caretAlphaInterpolator.setStartValue(0);
  m_caretAlphaInterpolator.setDuration(2000);
  m_caretAlphaInterpolator.setEndValue(255);

  m_renderingThread = std::make_unique<RenderingThread>(*this);
  connect(m_renderingThread.get(), SIGNAL(documentSizeChangedFromThread(const QSizeF&, const qreal)),
          this, SLOT(documentSizeChangedFromThread(const QSizeF&, const qreal)));
}

void CodeTextEdit::unloadDocument() {
  if (m_renderingThread->isRunning() == true)
    m_renderingThread->wait(); // Wait for all drawing operations to finish

  m_documentMutex.lock();
  m_document = nullptr;
  m_documentPixmap.release();
  // WARNING: the rendering thread should NEVER be stopped while the CodeTextEdit control is running
  // [NO] m_renderingThread.release();
  m_documentMutex.unlock();
  emit documentSizeChanged( QSizeF(), fontMetrics().height(), 0 );
  repaint();
}

void CodeTextEdit::loadDocument(Document *doc, int VScrollbarPos) {

  if (m_renderingThread->isRunning() == true)
    m_renderingThread->wait(); // Wait for all drawing operations to finish


  m_documentMutex.lock();
  m_document = doc;

  // Impose our character width and descent on this document before rendering it
  m_document->m_characterWidthPixels = this->m_characterWidthPixels;
  m_document->m_characterDescentPixels = this->m_characterDescentPixels;

  // Save the scrollbar position if we have one
  m_document->m_storeSliderPos = VScrollbarPos;

  // Calculate the new document size
  m_document->recalculateDocumentLines();

  // Set a new pixmap for rendering this document ~ caveat: this is NOT the viewport dimension
  // since everything needs to be rendered, not just the viewport region
  m_documentPixmap = std::make_unique<QImage>(viewport()->width(), m_document->m_numberOfEditorLines *
                                              fontMetrics().height() + 20 /* Remember to compensate the offset */,
                                              QImage::Format_ARGB32_Premultiplied);
  m_documentMutex.unlock();

  m_messageQueueMutex.lock();
  m_documentUpdateMessages.emplace_back( viewport()->width(), viewport()->width(), m_document->m_numberOfEditorLines *
                                         fontMetrics().height() + 20 /* Remember to compensate the offset */);
  m_messageQueueMutex.unlock();

  if( m_renderingThread->isRunning() == false )
      m_renderingThread->start();  

//  QSizeF newSize;
//  newSize.setHeight( m_document->m_numberOfEditorLines );
//  newSize.setWidth ( m_document->m_maximumCharactersLine );

//  // Emit a documentSizeChanged signal. This will trigger scrollbars resizing
//  emit documentSizeChanged( newSize, fontMetrics().height() );

//  m_verticalScrollBar->setSliderPosition(0);

//  this->viewport()->repaint(); // Trigger a cache invalidation for the viewport (necessary)
}

int CodeTextEdit::getViewportWidth() const {
  return this->viewport()->width();
}

int CodeTextEdit::getCharacterWidthPixels() const {
  return this->m_characterWidthPixels;
}

// A click in the viewport area usually moves the caret around if there's a document loaded
void CodeTextEdit::mousePressEvent(QMouseEvent *evt) {
  if (!m_document)
    return;

  if ( evt->button() == Qt::LeftButton ) {
    // Find the cell where the click was performed (x;y)
    int y = m_sliderValue + evt->pos().y() / fontMetrics().height();
    int x = (evt->pos().x() - BITMAP_OFFSET_X) / m_characterWidthPixels;
    qDebug() << "Click in document detected: (" << x << ";" << y << ")";

    m_documentMutex.lock();
      m_document->setCursorPos(x, y);
    m_documentMutex.unlock();

    m_caretAlphaInterpolator.stop();
    m_caretAlphaInterpolator.setCurrentTime(m_caretAlphaInterpolator.duration());
    m_caretAlphaInterpolator.setDirection(QAbstractAnimation::Backward);
  }
}

// As the name suggests: render the entire document on the internal stored pixmap
void CodeTextEdit::renderDocumentOnPixmap() {

  //QElapsedTimer timer;
  //timer.start();

  m_backgroundBufferPixmap->fill(Qt::transparent); // Set the pixmap transparent

  QPainter painter(m_backgroundBufferPixmap.get()); // Draw into the pixmap
  painter.setFont(this->font()); // And grab the widget's monospace font

  // Drawing the background is not needed since it is already transparent (the background only
  // needs to be set on the viewport)
  // const QBrush backgroundBrush(QColor(39, 40, 34));
  // painter.setBrush(backgroundBrush);
  // painter.fillRect(m_documentPixmap->rect(), backgroundBrush);

  painter.setPen(QPen(Qt::white)); // A classic Monokai style
  auto setColor = [&painter](Style s) {
    switch(s) {
    case Comment: {
      painter.setPen(QPen(QColor(117,113,94))); // Gray-ish
    } break;
    case Keyword: {
      painter.setPen(QPen(QColor(249,38,114))); // Pink-ish
    } break;
    case QuotedString: {
      painter.setPen(QPen(QColor(230,219,88))); // Yellow-ish
    } break;
    case Identifier: {
      painter.setPen(QPen(QColor(166,226,46))); // Green-ish
    } break;
    case KeywordInnerScope:
    case FunctionCall: {
      painter.setPen(QPen(QColor(102,217,239))); // Light blue
    } break;
    case Literal: {
      painter.setPen(QPen(QColor(174,129,255))); // Purple-ish
    } break;
    default: {
      painter.setPen(QPen(Qt::white));
    } break;
    };
  };

  // Notice the usage of the character descent: we're making sure that every call to drawText()
  // specifies the bottom-left point of a character
  QPointF startpoint(BITMAP_OFFSET_X, BITMAP_OFFSET_Y - m_characterDescentPixels);
  size_t documentRelativePos = 0;
  size_t lineRelativePos = 0;
  auto styleIt = m_document->m_styleDb.styleSegment.begin();
  auto styleEnd = m_document->m_styleDb.styleSegment.end();
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

  // Implement the main rendering loop algorithm which renders characters segment by segment
  // on the viewport area
  for(auto& pl : m_document->m_physicalLines) {

    size_t editorLineIndex = 0; // This helps tracking the last EditorLine of a PhysicalLine
    for(auto& el : pl.m_editorLines) {
      ++editorLineIndex;

      do {
        startpoint.setX( BITMAP_OFFSET_X + lineRelativePos * m_characterWidthPixels );

        // If we don't have a destination OR we can't reach it within our line, just draw the entire line and continue
        if (nextDestination == size_t(-1) ||
            nextDestination > documentRelativePos + (el.m_characters.size() - lineRelativePos)) {

          // Multiple lines will have to be rendered, just render this till the end and continue

          int charsRendered = 0;
          if (el.m_characters.size() > 0) { // Empty lines must be skipped
            QString ts(el.m_characters.data() + lineRelativePos, static_cast<int>(el.m_characters.size() - lineRelativePos));
            painter.drawText(startpoint, ts);
            charsRendered = ts.size();
          }

          lineRelativePos = 0; // Next editor line will just start from the beginning
          documentRelativePos += charsRendered + /* Plus a newline if a physical line ended (NOT an EditorLine) */
              (editorLineIndex == pl.m_editorLines.size() ? 1 : 0);

          break; // Go and fetch a new line for the next cycle
        } else {

          // We can reach the goal within this line

          int charsRendered = 0;
          if (el.m_characters.size() > 0) { // Empty lines must be skipped
            QString ts(el.m_characters.data() + lineRelativePos, static_cast<int>(nextDestination - documentRelativePos));
            painter.drawText(startpoint, ts);
            charsRendered = ts.size();
          }

          bool goFetchNewLine = false; // If this goal also exhausted the current editor line, go fetch
                                       // another one
          bool addNewLine = false; // If this was the last editor line, also add a newline because it
                                   // corresponds to a new physical line starting
          if(nextDestination - documentRelativePos + lineRelativePos == el.m_characters.size()) {
            goFetchNewLine = true;

            // Do not allow EditorLine to insert a '\n'. They're virtual lines
            if (editorLineIndex == pl.m_editorLines.size())
              addNewLine = true;

            lineRelativePos = 0; // Next editor line will just start from the beginning
          } else
            lineRelativePos += charsRendered;

          documentRelativePos += charsRendered + (addNewLine ? 1 : 0); // Just add a newline if we also reached this line's
                                                                       // end AND a physical line ended, not an EditorLine

          calculateNextDestination(); // Need a new goal

          if( goFetchNewLine )
            break; // Go fetch a new editor line (possibly on another physical line),
                   // we exhausted this editor line
        }

      } while(true);

      // Move the rendering cursor (carriage-return)
      startpoint.setY (startpoint.y() + fontMetrics().height());
    }
  }

  //m_invalidatedPixmap = false; // QPixmap has been redrawn

  m_documentMutex.lock();
  m_documentPixmap.swap(m_backgroundBufferPixmap);
  m_documentMutex.unlock();
  //qDebug() << "Done rendering document lines in " << timer.elapsed() << " milliseconds";
}



void CodeTextEdit::paintEvent (QPaintEvent *) {

  QPainter view(viewport());

  // Draw control background (this blends with the TabsBar selected tab's bottom color)
  const QBrush backgroundBrush(QColor(39, 40, 34));
  view.setBrush(backgroundBrush);
  view.fillRect(rect(), backgroundBrush);

  //////////////////////////////////////////////////////////////////////
  // Draw the document
  //////////////////////////////////////////////////////////////////////

  m_documentMutex.lock();
  if (m_document != nullptr) {
    // Apply the offset and draw the pixmap on the viewport
    int scrollOffset = m_sliderValue * fontMetrics().height();
    QRectF pixmapRequestedRect(m_documentPixmap->rect().x(), m_documentPixmap->rect().y() + scrollOffset,
                               m_documentPixmap->rect().width(), viewport()->height());
    QRectF myViewRect = viewport()->rect();
    myViewRect.setWidth (pixmapRequestedRect.width());

    view.drawImage(myViewRect, *m_documentPixmap, pixmapRequestedRect);
    //view.drawPixmap (myViewRect, *m_documentPixmap, pixmapRequestedRect);

    // Draw the cursor if in sight
    auto cursorPos = m_document->m_viewportCursorPos;

    // Is the cursor in sight?
    auto firstViewVisibleLine = m_sliderValue;
    auto lastViewVisibleLine = firstViewVisibleLine + (myViewRect.height() / fontMetrics().height());
    if (cursorPos.y >= firstViewVisibleLine - 1 && cursorPos.y < lastViewVisibleLine + 1) {
      if (m_caretAlphaInterpolator.state() != QAbstractAnimation::Running)
        m_caretAlphaInterpolator.start();

      const QPen caretPen(QColor(255, 255, 255, m_caretAlpha));
      view.setPen(caretPen);

      const int characterHeightPixels = fontMetrics().height();
      auto viewRelativeTopStart = (cursorPos.y - firstViewVisibleLine /* Line view-relative where the caret is at */) * characterHeightPixels;
      view.drawLine(cursorPos.x * m_characterWidthPixels + BITMAP_OFFSET_X, // Bottom point of the line
                    viewRelativeTopStart + BITMAP_OFFSET_Y,
                    cursorPos.x * m_characterWidthPixels + BITMAP_OFFSET_X,
                    viewRelativeTopStart + BITMAP_OFFSET_Y - characterHeightPixels /* Caret length */);
    }
  } else {
    // No document
    m_caretAlphaInterpolator.stop();
  }

  m_documentMutex.unlock();
}
void CodeTextEdit::resizeEvent (QResizeEvent *evt) {

  if (m_document == nullptr)
    return;

  m_messageQueueMutex.lock();
    m_documentMutex.lock();
      // Post a resize message for the rendering thread
      m_documentUpdateMessages.emplace_back( evt->size().width(), viewport()->width(), m_document->m_numberOfEditorLines *
                                             fontMetrics().height() + BITMAP_OFFSET_Y /* Remember to compensate the offset */ );
      // Save the current slider position in the document
      m_document->m_storeSliderPos = m_sliderValue;
    m_documentMutex.unlock();
  m_messageQueueMutex.unlock();

  if( m_renderingThread->isRunning() == false )
      m_renderingThread->start();
}

void CodeTextEdit::verticalSliderValueChanged (int value) {
  // This method is called each time there's a change in the vertical slider and we need to refresh the view
  m_sliderValue = value;
  repaint();
}

void CodeTextEdit::documentSizeChangedFromThread(const QSizeF &newSize, const qreal lineHeight) {
  emit documentSizeChanged( newSize, lineHeight, m_document->m_storeSliderPos ); // Forward the signal to our QScrollBar

//  // Since the thread updated the document size entirely, we're not able to adjust the scroll rate (mouse wheel and pgUp/Down)
//  // to better fit the number of lines of the document

//  // A document of 100 lines should be scrolled by 3 lines per scroll
//  m_document->m_numberOfEditorLines
//  this->verticalScrollBar()->setSingleStep(5);
//  this->verticalScrollBar()->setPageStep(50);

  this->repaint();
}

///////////////////////////////////////////////
///            RenderingThread              ///
///////////////////////////////////////////////

void RenderingThread::run() {

  while(getFrontElement() == true) {
    m_cte.m_documentMutex.lock();
    m_cte.m_document->setWrapWidth(m_currentElement.wrapWidth);

    m_cte.m_backgroundBufferPixmap = std::make_unique<QImage>(m_currentElement.bufferWidth,
                                                              m_currentElement.bufferHeight,
                                                              QImage::Format_ARGB32_Premultiplied);

    m_cte.m_documentMutex.unlock();

    m_cte.renderDocumentOnPixmap();

    // Even if the document's size might not have changed, we still need to fire a documentSizeChanged
    // event since scrollbars use this also to calculate the maximum number of lines our viewport can display
    QSizeF newSize;
    qreal lineHeight = m_cte.fontMetrics().height();
    newSize.setHeight( m_cte.m_document->m_numberOfEditorLines );
    newSize.setWidth ( m_cte.m_document->m_maximumCharactersLine );

    // Emit a documentSizeChanged signal. This will trigger scrollbars 'maxViewableLines' calculations
    emit documentSizeChangedFromThread ( newSize, lineHeight );
  }
}

bool RenderingThread::getFrontElement() {
  bool elementFound = false;
  m_cte.m_messageQueueMutex.lock();
  if (m_cte.m_documentUpdateMessages.size() > 0) {
    m_currentElement = m_cte.m_documentUpdateMessages.back();
    m_cte.m_documentUpdateMessages.clear(); // We're ONLY interested in the front object
    elementFound = true;
  }
  m_cte.m_messageQueueMutex.unlock();
  return elementFound;
}

CaretBlinkingInterpolation::CaretBlinkingInterpolation(CodeTextEdit& parent) :
  m_parent(parent)
{
  connect (this, SIGNAL(finished()), SLOT(finished()));
  setLoopCount(1); // Run once, then reset it with a new direction
}

// Called at every variation of the interpolation value
void CaretBlinkingInterpolation::updateCurrentValue(const QVariant& value) {
  // Update the caret's state (notice that this might go forward or backward depending on Direction)
  m_parent.m_caretAlpha = value.toInt();
  m_parent.update(); // Do NOT call repaint() here since it might end in an infinite paint recursion
}

// Animation has finished, switch direction (from start to end or vice-versa)
void CaretBlinkingInterpolation::finished() {
  setDirection((direction() == QAbstractAnimation::Forward) ? QAbstractAnimation::Backward : QAbstractAnimation::Forward);
  start();
}
