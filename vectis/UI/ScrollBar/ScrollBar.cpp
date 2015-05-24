#include <UI/ScrollBar/ScrollBar.h>
#include <QtCore/qmath.h>
#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include <QTextBlock>
#include <QDebug> // TODO: remove when resize is finished

// A filter for PageDown/Up events to avoid the caret movement problem which causes a wrong scroll animation
PgKeyEater::PgKeyEater ( ScrollBar *scrollBar ) :
    m_scrollBar( scrollBar ) {
}

bool PgKeyEater::eventFilter ( QObject *obj, QEvent *event ) {
    if ( event->type() == QEvent::KeyPress ) { // The only event to be handled is the PgDown/Up
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        // Completely eat the pageup/pagedown; it will be handled by the vertical scrollbar (TODO: avoid this for horizontal scrollbars)
        if( keyEvent->key() == Qt::Key_PageUp ) {
            m_scrollBar->m_scrollAnim.setProperty( "pageStepEvent", 1 ); // Signal that the event was a PgUp
            // Set the new position manually since this will be eaten up completely
            m_scrollBar->setSliderPosition( m_scrollBar->sliderPosition() - m_scrollBar->pageStep() );
            return true; // Event was completely handled, interrupt handling for all subclasses
        } else if ( keyEvent->key() == Qt::Key_PageDown ) {
            m_scrollBar->m_scrollAnim.setProperty( "pageStepEvent", 2 ); // PgDown
            m_scrollBar->setSliderPosition( m_scrollBar->sliderPosition() + m_scrollBar->pageStep() ); // Ditto as above
            return true;
        }
    }
    // Other events: standard event processing
    return QObject::eventFilter( obj, event );
}

ScrollBar::ScrollBar (QAbstractScrollArea *parent ) :
    QScrollBar(parent),
    m_parent(parent),
    m_textLineHeight(1), // Height of every line, it depends on the font used (although it's always monospaced)
    m_internalLineCount(1), // How many lines are actually in the document
    m_sliderIsBeingDragged(false),
    m_scrollAnim(this, "value"), // A timer for a more pleasant scroll animation
    m_pgKeyEater(this) // A keyboard filter to cancel the pageup/down and handle it internally (it is necessary since
                       // otherwise there's no way to avoid the caret, moved with pagedown/up, changing the value where
                       // the scrollbar points to and later causing an immediate wrong movement at the next animation)
{
    Q_ASSERT( parent );

    // WA_OpaquePaintEvent specifies that we'll redraw the control every time it is needed without any system intervention.
    // WA_NoSystemBackground avoids the system to draw the background (we'll handle it as well)
    setAttribute( Qt::WA_OpaquePaintEvent, true );
    setAttribute( Qt::WA_NoSystemBackground, true );
    setStyleSheet(QString("QScrollBar:vertical { \
                              width:15px;        \
                           }"));

    // Only way to detect when line wrapping inserts additional lines is through documentSizeChanged()
    connect( m_parent, SIGNAL(documentSizeChanged(const QSizeF&, const qreal)),
            this, SLOT(documentSizeChanged(const QSizeF&, const qreal)) );

    // Handling signals scroll mouse / page down-up / dragging (tracking)
    connect(this, SIGNAL(actionTriggered(int)), this, SLOT(actionTriggered(int)));
    //connect(this, SIGNAL(sliderPressed()), this, SLOT(sliderPressed()));
    //connect(this, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));
    //connect(this, SIGNAL())

    // Install PageDown/PageUp key eater for the parent control
    m_parent->installEventFilter( &m_pgKeyEater );
    // Move caret at the end of the scroll animation (pageStepEvent property helps not to move it for mouse scroll)
    // connect( &m_scrollAnim, SIGNAL(finished()), this, SLOT(moveParentCaret()) );

    // Connect the parent "refresh view" slot with the slider's changes
    connect( this, SIGNAL(sliderValueChanged(int)), m_parent, SLOT(verticalSliderValueChanged(int)) );
}

ScrollBar::~ScrollBar() {
    m_parent->removeEventFilter(&m_pgKeyEater);
}

// Tracking events (user is manually moving the slider)
//void ScrollBar::sliderReleased () {
//    m_sliderIsBeingDragged = false;
//}
//void ScrollBar::sliderPressed () {
//    m_sliderIsBeingDragged = true;
//}

// Tracking events are imprecise if we manually draw the slider. Tracking the mouse click into the slider's area
// is way more accurate
void ScrollBar::mousePressEvent ( QMouseEvent *e ) {
  if( m_sliderIsBeingDragged == false && m_sliderPath.contains( e->pos() ) == true ) {
    m_sliderIsBeingDragged = true;
    m_mouseTrackingStartPoint = e->pos().y();
    m_mouseTrackingStartValue = sliderPosition();
  } else {
    // A click was detected but NOT on the slider. Move the slider at the click position
    int x = static_cast<int>( e->pos().y() * m_internalLineCount / this->rect().height() );
    setSliderPosition(x);
  }
}

// The logic here is the following: if we're tracking the slider, calculate the delta from the tracking start
// point and scale it not according to the scrollbar area rectangle, but to the number of lines of the document
// (at rect.y == 0 we're at line 0, at rect.y == max we're at line lastLine). Finally add this value to the
// slider position that was stored before the tracking (so that we're not modifying its position)
void ScrollBar::mouseMoveEvent ( QMouseEvent *e ) {
  if ( m_sliderIsBeingDragged == true ) {
    // It is important to note that even if the mouse did a complete document tracking (i.e. from line 0 to the last
    // one), its path's length would NOT be equal to the control rect().height() since the slider's length would have
    // to be subtracted. This can be easily visualized on a piece of paper. The right proportion is therefore:
    // x : m_internalLineCount = delta : (this->rect().height() - sliderLength)
    int delta = ( e->pos().y() - m_mouseTrackingStartPoint );
    int x = static_cast<int>( delta * m_internalLineCount / (this->rect().height() - m_lenSlider) );
    setSliderPosition(m_mouseTrackingStartValue + x);
  }
}

void ScrollBar::mouseReleaseEvent ( QMouseEvent * ) {
  if ( m_sliderIsBeingDragged == true ) {
    emit sliderReleased();
    m_sliderIsBeingDragged = false;
  }
}

// Move the caret (cursor) of the parent to the position reached by the scrollbar. Do this ONLY WHEN animation has
// terminated (so that the animation smoothness is not affected). Useful when a PageUp/PageDown was called.
// A mousescroll won't cause caret's movement and thus it is not handled here (pageStepEvent remains 0 in that case)
void ScrollBar::moveParentCaret() {
    int pageValue = m_scrollAnim.property("pageStepEvent").toInt();
    if(pageValue > 0) { // PageDown or PageUp was pressed

        /* TODO: find a way to provide a cursor.movePosition to the CodeTextEdit

        QTextCursor cursor = m_parent->textCursor();
        // Rewind the cursor to the beginning of the document and then position it at the line it had to be positioned
        cursor.movePosition( QTextCursor::Start, QTextCursor::MoveAnchor );
        cursor.movePosition( QTextCursor::NextBlock, QTextCursor::MoveAnchor,
                             value() / m_textLineHeight ); // Recall that the slider's value is line_index * line_height
        m_parent->setTextCursor(cursor);
        m_scrollAnim.setProperty("pageStepEvent", 0); // Helps not to reset the caret for a mouse scroll (it isn't needed)

        */
    }
}

// This slot is invoked every time the slider is moved (scrolling, pageup/down, dragging(tracking))
// Notice that:
//   "When the signal is emitted, the sliderPosition has been adjusted according to the action, but the value has not
//    yet been propagated (meaning the valueChanged() signal was not yet emitted)"
void ScrollBar::actionTriggered ( int action ) {
    if(action == QAbstractSlider::SliderNoAction || m_sliderIsBeingDragged) // No animation in tracking
        return;

    // If an animation is already in progress - interrupt it. Values will be updated and a new animation will start
    if(m_scrollAnim.state() == QAbstractAnimation::State::Running)
        m_scrollAnim.setEndValue(sliderPosition());
    else {
      m_scrollAnim.setStartValue(value());
      m_scrollAnim.setDuration(100);
      m_scrollAnim.setEndValue(sliderPosition());
      m_scrollAnim.start();
    }

}

// When the control is resized, the maximum number of lines we can display into the view is updated as well
void ScrollBar::resizeEvent ( QResizeEvent * event ) {
   // qDebug() << "TODO: resizeEvent for the ScrollBar, is it useful setMaximum here? If docSizeChange is enough, remove it";
   // setMaximum( m_internalLineCount * m_textLineHeight );
    //qDebug() << "resizeEvent: maximum updated to: " << maximum();
    //qDebug() << "m_maxNumLines is now " << m_maxNumLines;
    //qDebug() << "textLine.height() is  " << m_textLineHeight;
    QScrollBar::resizeEvent(event);
}

// Every time there's a change in the slider (e.g. value/range/orientation/etc..) the number of maximum lines
// must be redimensioned to add some "empty" virtual lines (read below)
void ScrollBar::sliderChange ( SliderChange change ) {
    // To simulate some "empty virtual lines" at the bottom and allow to scroll the last line till the
    // beginning of the view, it is necessary to detect when the slider value changes (e.g. I'm scrolling or
    // adding/removing lines, or perhaps I'm wrapping) and increase the maximum the control can be scrolled to.
    // This can't be as simple as doing m_internalLineCount = (m_parent->document()->lineCount() - 1);
    // because in wrap mode this isn't always valid and might create problems. A full 'recalculate' has to be triggered.
    setMaximum( m_internalLineCount - 1 );
    // qDebug() << "sliderValueChange after setMaximum received and set to == " << value();

    emit sliderValueChanged(value()); // Signal our parents that it's time to redraw the scene
    QAbstractSlider::sliderChange(change);
}

// Emitted when the document changes size, it is the only way to detect the number of lines in the document if wrapping is active
void ScrollBar::documentSizeChanged(const QSizeF & newSize, const qreal lineHeight) {

    // Useful information:
    // - The hierarchy used to find the parent QPlainTextEdit widget is:
    //   QScrollBar >parent> qt_scrollarea_vcontainer >parent> QPlainTextEdit
    // - If we don't insert frames or tables, blocks == lines

    // In case of a custom control, the lineHeight has to be manually provided. The code to relink a QPlainTextEdit is left
    // here for compatibility reasons

    // Calculate the height of a whatsoever line (the first is chosen since it is always present)
    // QTextBlock block = m_parent->document()->findBlockByNumber( 0 );
    // QTextLayout *layout = block.layout(); // Layout of a line
    // QTextLine textLine = layout->lineAt( 0 );

    m_textLineHeight = lineHeight; // textLine.height();
    // Update the maximum number of visible lines in the text control, this might have changed
    m_maxViewVisibleLines = qFloor( qreal( m_parent->viewport()->height() ) / m_textLineHeight );

    // Calculate the real number of lines in the document
    m_internalLineCount = int( newSize.height() );
    // Also update the maximum allowed to let the last line to be scrolled till the beginning of the view
    setMaximum( m_internalLineCount - 1 );
    // qDebug() << "m_textLineHeight "  << m_textLineHeight << " m_maxNumLines " << m_maxViewVisibleLines << " m_internalLineCount "
    //  << m_internalLineCount;
}

// The most important event in the control: repaint.
// The fundamental equation to repaint the scrollbar is:
//  slider_length = maximum_slider_height * (how_many_lines_I_can_display_in_the_view / total_number_of_lines_in_the_document)
void ScrollBar::paintEvent ( QPaintEvent* ) {

    QPainter p( this );

    // >> ---------------------------------------------------------------------
    //    Calculate the position, length and drawing area for the slider
    // --------------------------------------------------------------------  <<

    // extraBottomLines are virtual lines to let the last line of text be scrollable till it is left as the only one above in the view.
    // Thus they correspond to the maximum number of lines that the view can visualize - 1 (the one I want is excluded)
    int extraBottomLines = (m_maxViewVisibleLines - 1);

    // Since maximum() is ALWAYS greater than value() (the number of lines in the control is always greater
    // or equal to the line we're scrolled at), the position of the slider is:
    //   slider_position = view_height * (line_where_the_view_is_scrolled / total_number_of_lines_in_the_document)
    // in this case we calculate a "relative" position by using value() and maximum() which are relative to the control (not to the document)
    float viewRelativePos = float(m_maxViewVisibleLines) * (float(value()) / float(maximum() + (extraBottomLines)));

    // now find the absolute position in the control's rect, the proportion is:
    //  rect().height() : x = m_maxViewVisibleLines : viewRelativePos
    float rectAbsPos = (float(rect().height()) * viewRelativePos) / float(m_maxViewVisibleLines);

    // qDebug() << "maxNumLines is " << maxNumLines << " and viewRelativePos is = " << viewRelativePos <<
    //            " rectAbsPos = " << rectAbsPos;

    // Calculate the length of the slider's rect including extraBottomLines
    m_lenSlider = int( float(rect().height()) * (float(m_maxViewVisibleLines) / float(m_internalLineCount + extraBottomLines)) );

    // Set a mimumim length for the slider (when there are LOTS of lines)
    if( m_lenSlider < 15 )
        m_lenSlider = 15;

    // Prevents the slider to be drawn, due to roundoff errors, outside the scrollbar rectangle
    if( rectAbsPos + m_lenSlider > rect().height() )
        rectAbsPos -= ( rectAbsPos + m_lenSlider ) - rect().height();

    // This is finally the drawing area for the slider
    QRect rcSlider(0, rectAbsPos, rect().width() - 1, m_lenSlider );
    // p.fillRect( rcSlider, QColor( 55, 4, 255, 100 ) );

    // >> ------------------------
    //       Slider drawing
    // -----------------------  <<

    // A separation line of 1 px
    QPen lp( QColor( 29, 29, 29 ) );
    p.setPen(lp);
    p.drawLine( rect().left(), rect().top(), rect().left(), rect().bottom() );

    // Soft background gradient from sx to dx
    QLinearGradient bkGrad( rect().topLeft(), rect().topRight() );
    bkGrad.setColorAt( 0, QColor(33, 33, 33) );
    bkGrad.setColorAt( 1, QColor(50, 50, 50) );
    QRect rc = rect();
    rc.setLeft( rc.left()+1 );
    p.fillRect( rc, bkGrad );

    // Draws the slider with a rounded rectangle
    // rcSlider is the hitbox, but to draw it we only take a width subsection
    QRect rcSliderSubsection( rcSlider );
    rcSliderSubsection.setX( rcSliderSubsection.x()+3 );
    rcSliderSubsection.setWidth( rcSliderSubsection.width()-2 );
    p.setRenderHint( QPainter::Antialiasing );
    QPainterPath empty;
    m_sliderPath.swap(empty); // Cleans the current slider path
    m_sliderPath.setFillRule( Qt::WindingFill ); // Fill for closed-shapes
    m_sliderPath.addRoundedRect( rcSliderSubsection, 4, 4 );

    // Select a gradient brush to fill the slider
    QLinearGradient fillGrad( rect().topLeft(), rect().topRight() );
    fillGrad.setColorAt( 0, QColor(88, 88, 88) );
    fillGrad.setColorAt( 1, QColor(64, 64, 64) );
    QBrush gradFill( fillGrad );
    p.setBrush( gradFill );

    // Finally draw the slider
    p.drawPath( m_sliderPath.simplified() /* Join any segments and obtain a single path */ );

    // QScrollBar::paintEvent(event); // No base class paintEvent - we completely handled redrawing
}
