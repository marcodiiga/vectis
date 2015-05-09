#include <UI/TabsBar/TabsBar.h>
#include <QPainter>

#include <QtGlobal>
#include <QDebug>
#include <QStyleOption>
#include <QApplication>

TabsBar::TabsBar( QWidget *parent )
    : m_parent(parent),
      m_draggingInProgress(false),
      m_selectedTabIndex(-1)
{
    Q_ASSERT( parent );

    // WA_OpaquePaintEvent specifies that we'll redraw the control every time it is needed without
    // any system intervention
    setAttribute( Qt::WA_OpaquePaintEvent, false );
    //qInstallMessageHandler(crashMessageOutput);
    setStyleSheet("QWidget { background-color: rgb(22,23,19); }");

    QFont font("Verdana");
    font.setPixelSize(10);
    this->setFont(font);

    //DEBUG
    //DEBUG
    // Tryouts here for tabs

//    // TODO: make a method "insertNewTab"
    m_tabs.push_back(Tab("First tab"));
    m_interpolators.emplace_back(std::make_unique<SlideToPositionAnimation>(*this, m_tabs.size()-1));
    m_verticalInterpolators.emplace_back(std::make_unique<VerticalSlideAnimation>(*this, m_tabs.size()-1));

    m_tabs.push_back(Tab("Second tab"));
    m_interpolators.emplace_back(std::make_unique<SlideToPositionAnimation>(*this, m_tabs.size()-1));
    m_verticalInterpolators.emplace_back(std::make_unique<VerticalSlideAnimation>(*this, m_tabs.size()-1));

    m_tabs.push_back(Tab("Third tab"));
    m_interpolators.emplace_back(std::make_unique<SlideToPositionAnimation>(*this, m_tabs.size()-1));
    m_verticalInterpolators.emplace_back(std::make_unique<VerticalSlideAnimation>(*this, m_tabs.size()-1));

    m_tabs.push_back(Tab("Fourth tab"));
    m_interpolators.emplace_back(std::make_unique<SlideToPositionAnimation>(*this, m_tabs.size()-1));
    m_verticalInterpolators.emplace_back(std::make_unique<VerticalSlideAnimation>(*this, m_tabs.size()-1));



    m_selectedTabIndex = 1;
    //DEBUG
}

// Dynamically inserts a tab into the control by providing a "sliding" vertical animation
void TabsBar::insertTab(const QString text) {
    m_tabs.emplace_back(text);
    int newTabIndex = static_cast<int>(m_tabs.size() - 1);
    m_interpolators.emplace_back(std::make_unique<SlideToPositionAnimation>(*this, newTabIndex));

    m_verticalInterpolators.emplace_back(std::make_unique<VerticalSlideAnimation>(*this, newTabIndex));
    m_verticalInterpolators[newTabIndex]->setDuration(100);
    m_verticalInterpolators[newTabIndex]->setStartValue(35);
    m_verticalInterpolators[newTabIndex]->setEndValue(0);
    m_verticalInterpolators[newTabIndex]->start();

    m_selectedTabIndex = newTabIndex; // Also make it the new selected one
    repaint();
}

// Draw a tab (selected or not) into a given rect. If left and right tabs are known, it allows to have a nicer
// look by adding a nuance
QPainterPath TabsBar::drawTabInsideRect(QPainter& p, const QRect& tabRect, bool selected, QString text) {
    // Decide colors for a selected or unselected tab
    QColor topGradientColor, bottomGradientColor;
    if( selected == false ) { // Unselected
        topGradientColor = QColor(54, 54, 52);
        bottomGradientColor = QColor(72, 72, 69);
    } else {
        topGradientColor = QColor(52, 53, 47);
        bottomGradientColor = QColor(39, 40, 34);
    }
    // Draw two bezièr curves and a horizontal line for the tab
    //  _______________
    // |0;0  c2  /     |h;0
    // |       _/      |
    // |     _/        |     Containing square for the left curve
    // |    /          |
    // |___/____c1_____|
    // 0;h              h;h
    QPainterPath tabPath;
    p.setRenderHint(QPainter::Antialiasing);
    const QPointF c1(0.6f, 1.0f-0.01f), c2(0.53f, 1.0f-1.0f);
    const int h = tabRect.height();
    tabPath.moveTo(tabRect.x(), tabRect.y() + tabRect.height() + 1); // QRect has 1px borders, compensation
    //qDebug() << selectedTabRect.x() << " " << selectedTabRect.y() + selectedTabRect.height() + 1;
    tabPath.cubicTo(tabRect.x() + h * c1.x(), tabRect.y() + h * c1.y(),
                 tabRect.x() + h * c2.x(), tabRect.y() + h * c2.y(),
                 tabRect.x() + h, tabRect.y()); // Destination point
    //qDebug() << selectedTabRect.x() + h << " " << selectedTabRect.y();

    int dxStartBez = tabRect.x() + tabRect.width() - h;
    tabPath.lineTo(dxStartBez, tabRect.y()); // Tab width

    // Draw right bezièr
    tabPath.cubicTo(dxStartBez + (h - h * c2.x()), tabRect.y() + h * c2.y(),
                    dxStartBez + (h - h * c1.x()), tabRect.y() + h * c1.y(),
                    tabRect.x() + tabRect.width(), tabRect.y() + tabRect.height() + 1);

    // Fill the tabPath with a background gradient
    QLinearGradient tabGradient( tabRect.topLeft(), tabRect.bottomLeft() ); // Vertical direction
    tabGradient.setColorAt( 1, bottomGradientColor ); // Final color is the code text box background
    tabGradient.setColorAt( 0, topGradientColor ); // Initial color (at the top) is brighter
    QBrush tabBrushFill( tabGradient );
    p.setBrush( tabBrushFill );
    p.fillPath( tabPath, tabBrushFill );

    p.setRenderHint(QPainter::HighQualityAntialiasing);
    // Once filled the background, a gray border can be drawn above translated 1px upper
    const QPen grayPen( QColor(60, 61, 56) );
    const QPen darkerGrayPen ( QColor(50, 51, 46) );
    QPainterPath grayOuterTabPath = tabPath.translated(0, -1);
    if( selected == true )
        p.setPen( grayPen );
    else
        p.setPen( darkerGrayPen );
    p.drawPath( grayOuterTabPath );

    // Draw the tab's text in the subrectangle available
    if( selected == true )
        p.setPen( Qt::white );
    else
        p.setPen( QPen(QColor(193,193,191)) );
    QRectF textRect = tabRect;
    textRect.setX(tabRect.x() + h);
    textRect.setWidth(tabRect.width() - 2*h);
    textRect.setY(textRect.y() + 6);
    p.drawText(textRect,  Qt::AlignLeft, QString(text));

    return tabPath; // This will be used to fade the selected tab's borders, in case this tab was near the selected one
}

// Draw an horizontal bar to separate the control from the rest
void TabsBar::drawGrayHorizontalBar( QPainter& p , const QColor innerGrayCol ) {
    p.setRenderHint( QPainter::Antialiasing, false );
    p.setRenderHint( QPainter::HighQualityAntialiasing, false );

    // Draw the horizontal bar at the bottom linked to the CodeTextEdit;
    // this runs for all the code editor control *except* on the selected tab
    const QPen grayPen( innerGrayCol );
    p.setPen( grayPen );
    p.drawLine( rect().left(), rect().bottom(), rect().right(), rect().bottom() );
}

void TabsBar::paintEvent ( QPaintEvent* ) {

    Q_ASSERT( m_selectedTabIndex == -1 || // Nothing was selected or we're into a valid tab range
              ( m_selectedTabIndex != -1 && m_selectedTabIndex < m_tabs.size() ) );

    QStyleOption opt; // Allows background setting via styleSheet
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    const QColor innerGrayCol( 60, 61, 56 );

    if( m_selectedTabIndex == -1 ) {
        drawGrayHorizontalBar( p, innerGrayCol );
        return; // Nothing else has to be drawn
    }

    // Draws the tabs following a precise order:
    // -> first the ones NOT selected on the right and on the left of the selected one, starting from the closest to the
    // selected one and going outside
    // -> afterwards the gray horizontal line, it will be visible where there are NO tabs
    // -> finally the selected tab (that goes above everything else)
    //

    // Calculate the dimension of a tab according to the control's width and how many tabs are there to be drawn
    //
    //                   3w
    // |--------------------------------------| If the control has maximum dimension 3w (i.e. rect().width()-(bordi_vari)),
    // |     w      |     w      |     w      | and I have 3 tabs, maximum dimension is w per each. Anyway since tabs aren't
    // |            |            |            | laid out exactly one after another but there's an "intersection delta" between
    // |--------------------------------------| them (i.e. an intersection area), therefore it has to be noted that an unused
    // space is left at the end proportional to the number of tabs:
    //                                                                          |-----------------------------------------|
    // In the case here at the right, a 2d length space is left. This space     |         | d |        | d |        |
    // must be redistributed to the various tabs, thus                          ---------------
    //                  tabWidth = 3w / 3                                                 ------------------
    //                  tabWidth += ((3-1)*d)/3                                                   --------------  2d
    //                                                                                                               ------
    tabWidth = ( rect().width() - (5 + 20 /* sx border 5 px, dx border 20 px */) ) / (int)m_tabs.size();
    int tabHeight = rect().bottom() - 5;
    tabWidth += ( TAB_INTERSECTION_DELTA * (int)(m_tabs.size() - 1) ) / (int)m_tabs.size();
    // Clamp result to max and min values
    if( tabWidth > TAB_MAXIMUM_WIDTH )
        tabWidth = TAB_MAXIMUM_WIDTH;
    if( tabWidth < 2 * tabHeight ) // At least the space to draw bezièr curves
        tabWidth = 2 * tabHeight;
    QRect standardTabRect( 5, 5, tabWidth /* Width */, tabHeight );

    if( m_selectedTabIndex != -1 ) {

        QPainterPath temp, leftOfSelected, rightOfSelected;
        // This lambda takes care of calculating the right rect position for a tab with a given index
        // and of calling the drawing function
        auto calculatePositionAndDrawTab = [&](int i, bool leftTabs) {
          int x = 5 + i * tabWidth;
          x -= TAB_INTERSECTION_DELTA * i;
          standardTabRect.setX( x );
          // If we have a 'decreasing' offset, add it
          if(m_tabs[i].m_offset != 0) {
              if (leftTabs)
                standardTabRect.setX(standardTabRect.x() + m_tabs[i].m_offset);
              else
                standardTabRect.setX(standardTabRect.x() - m_tabs[i].m_offset);
          }
          if(m_tabs[i].m_verticalOffset != 0) {
              standardTabRect.setY(standardTabRect.y() + m_tabs[i].m_verticalOffset);
          }
          standardTabRect.setWidth( tabWidth );

          temp = drawTabInsideRect( p, standardTabRect, false, m_tabs[i].m_title );

          if (leftTabs) {
              if(i == m_selectedTabIndex - 1)
                  leftOfSelected = temp;
          } else {
              if(i == m_selectedTabIndex + 1)
                rightOfSelected = temp;
          }

          m_tabs[i].m_region = temp;

          // Debug code to visualize tab rects
          //p.setPen(QColor(255 - i*20,0,0));
          //p.drawRect(standardTabRect);
        };

        // Draw tabs at the left of the selected one
        for( int i = m_selectedTabIndex - 1; i >= 0; --i )
            calculatePositionAndDrawTab(i, true);

        // Draw tabs at the right of the selected one
        for( int i = (int)m_tabs.size()-1; i > m_selectedTabIndex; --i )
            calculatePositionAndDrawTab(i, false);

        drawGrayHorizontalBar( p, innerGrayCol );

        // Finally draw the selected one above all the others (obviously if there's at least one tab)
        int x = 5 + m_selectedTabIndex * tabWidth;
        x -= TAB_INTERSECTION_DELTA * m_selectedTabIndex;

        // Adjustment factors (negative or positive) in case we're being dragged
        if( m_draggingInProgress ) {
            x += m_XTrackingDistance; // The tab must remain where we were dragging it
        } else if(m_tabs[m_selectedTabIndex].m_offset != 0) {
            x += m_tabs[m_selectedTabIndex].m_offset;
        }
        if(m_tabs[m_selectedTabIndex].m_verticalOffset != 0) {
            standardTabRect.setY(standardTabRect.y() + m_tabs[m_selectedTabIndex].m_verticalOffset);
        }
        standardTabRect.setX( x );
        standardTabRect.setWidth( tabWidth );

        m_tabs[m_selectedTabIndex].m_region = drawTabInsideRect( p, standardTabRect, true, m_tabs[m_selectedTabIndex].m_title);
        m_tabs[m_selectedTabIndex].m_rect = standardTabRect;
    }
}

// This event deals with mouse click to change a selected tab
void TabsBar::mousePressEvent(QMouseEvent *evt) {
    if ( evt->button() == Qt::LeftButton ) {
        m_dragStartPosition = evt->pos();
        m_selectionStartIndex = m_selectedTabIndex;
        for( size_t i=0; i<m_tabs.size(); ++i ) {
            if(m_tabs[i].m_region.contains(m_dragStartPosition) == true) {
                m_selectedTabIndex = (int)i;
                repaint();
                // TODO: send a signal "changedSelectedTab" with the integer 'm_selectedTabIndex'
            }
        }
    }
    qDebug() << "mousePressEvent " << m_dragStartPosition;
}

// This event deals with tab dragging
void TabsBar::mouseMoveEvent( QMouseEvent *evt ) {
    if ( !(evt->buttons() & Qt::LeftButton) ) // The only button we deal with for tracking
        return;

    int mouseXPosition = evt->pos().x();

    // DO NOT allow a tab to be dragged outside of the control area [+5;width-20]
    int tabRelativeX = 5 + m_selectedTabIndex * tabWidth;
    tabRelativeX -= TAB_INTERSECTION_DELTA * m_selectedTabIndex;
    tabRelativeX = m_dragStartPosition.x() - tabRelativeX;
    //qDebug() << tabRelativeX;
    if( mouseXPosition < 5 + tabRelativeX )
        mouseXPosition = 5 + tabRelativeX;
    if( mouseXPosition > this->width() - 20 - (tabWidth - tabRelativeX) )
        mouseXPosition = this->width() - 20 - (tabWidth - tabRelativeX);

    // Calculate the negative or positive distance from the tracking starting point (that will be the offset of how much
    // the QRect will have to be moved to draw the tab we're dragging)
    m_XTrackingDistance = mouseXPosition - m_dragStartPosition.x();

    //qDebug() << "mouseMoveEvent is dragging, m_XTrackingDistance(" << m_XTrackingDistance << "), tabWidth(" << tabWidth << "),"
    //        << "m_dragStartPosition.x(" << m_dragStartPosition.x() << ")";

    // If we reached another tab's rect, "grab" its index. In Chrome more or less works like this: if there's a tab
    // in the direction which we're moving towards and we reached more than half our width: move it. If we continue another
    // half our width nothing happens. And then everything starts again.
    // Therefore tabWidth / 2 is the criterion to keep in mind
    if( abs(m_XTrackingDistance) > tabWidth / 2 ) {
        // Do a swap if there's at least one tab in that direction
        if( m_XTrackingDistance > 0 ) {
            if( m_tabs.size()-1 > m_selectedTabIndex ) {
                // Swap tab vector's content and update the new index
                setUpdatesEnabled(false);   // Disable paint() events UNTIL all updates are finished & the 'dethroned' tab has its
                                            // movement interpolator set

                //qDebug() << "Swap current tab (index: " << m_selectedTabIndex << ") with tab index: " << m_selectedTabIndex+1;
                std::swap(m_tabs[m_selectedTabIndex], m_tabs[m_selectedTabIndex+1]);
                //qDebug() << m_tabs[m_selectedTabIndex].m_rect;
                //qDebug() << m_tabs[m_selectedTabIndex+1].m_rect;

                // Once a tab exceeded by tabWidth/2 in a direction, swap with the first tab in that direction
                //
                //                  |-------|                      |-------|
                // (startDragPoint) |   1   | -------------------> |   2   |
                //                  |-------|                      |-------|
                //         XTrackingDistance just became greater than tabWidth / 2, time to swap!
                //
                // however also the starting point is moved forward
                //                  |-------|                      |-------|
                //                  |   2   | --(startDragPoint)-> |   1   |
                //                  |-------|                      |-------|
                //         XTrackingDistance -= tabWidth, now is negative and in fact tab 1 has moved left and hasn't still reached
                //         the equilibrium position in its new rectangle
                //
                // CAVEAT: increasing a place to the right also means compensating the TAB_INSERSECTION_DELTA (one less)
                m_XTrackingDistance = -(tabWidth / 2) + TAB_INTERSECTION_DELTA;
                //qDebug() << "m_XTrackingDistance -= tabWidth => m_XTrackingDistance(" << m_XTrackingDistance << ")";
                m_dragStartPosition.setX(m_dragStartPosition.x() + tabWidth - TAB_INTERSECTION_DELTA);
                //qDebug() << "m_dragStartPosition.x(" << m_dragStartPosition.x() << ")";

                // Starts the interpolator for the swap of the other tab (it must come back to its equilibrium position)
                int offset = tabWidth - TAB_INTERSECTION_DELTA;
                m_tabs[m_selectedTabIndex].m_offset = offset; // Must go to zero

                ++m_selectedTabIndex;
                m_interpolators[m_selectedTabIndex-1]->setDuration(200);
                m_interpolators[m_selectedTabIndex-1]->setStartValue(offset);
                m_interpolators[m_selectedTabIndex-1]->setEndValue(0);
                m_interpolators[m_selectedTabIndex-1]->start();
                setUpdatesEnabled(true);
            }
        } else {
            if( m_selectedTabIndex > 0 )
                // Swap the contents of the tab vector and update the new index
                setUpdatesEnabled(false);   // Disable paint() events UNTIL all updates are finished & the 'dethroned' tab has its
                                            // movement interpolator set
                //qDebug() << "Swap current tab (index: " << m_selectedTabIndex << ") with tab index: " << m_selectedTabIndex-1;
                std::swap(m_tabs[m_selectedTabIndex], m_tabs[m_selectedTabIndex-1]);

                // Same reasoning (inverted) as the case above
                m_XTrackingDistance = +(tabWidth / 2) - TAB_INTERSECTION_DELTA;
                m_dragStartPosition.setX(m_dragStartPosition.x() - tabWidth + TAB_INTERSECTION_DELTA);

                // Starts the interpolator for the swap of the other tab (it must come back to its equilibrium position)
                int offset = tabWidth - TAB_INTERSECTION_DELTA;
                m_tabs[m_selectedTabIndex].m_offset = offset; // Must go to zero

                --m_selectedTabIndex;
                m_interpolators[m_selectedTabIndex+1]->setDuration(200);
                m_interpolators[m_selectedTabIndex+1]->setStartValue(offset);
                m_interpolators[m_selectedTabIndex+1]->setEndValue(0);
                m_interpolators[m_selectedTabIndex+1]->start();
                setUpdatesEnabled(true);
        }
    }


    m_draggingInProgress = true;
    repaint();
}

// Signal the end of a tracking event
void TabsBar::mouseReleaseEvent(QMouseEvent *evt) {
    if( m_draggingInProgress == false )
        return;

    qDebug() << "Tracking ended";

    // Animate the "return" to the correct position, i.e. decreases the XTrackingDistance to zero
    m_tabs[m_selectedTabIndex].m_offset = m_XTrackingDistance; // Must go to zero
    m_interpolators[m_selectedTabIndex]->setDuration(200);
    m_interpolators[m_selectedTabIndex]->setStartValue(m_XTrackingDistance);
    m_interpolators[m_selectedTabIndex]->setEndValue(0);
    m_interpolators[m_selectedTabIndex]->start();

    m_draggingInProgress = false;
}



SlideToPositionAnimation::SlideToPositionAnimation(TabsBar& parent, size_t associatedTabIndex ) :
    m_parent(parent),
    m_associatedTabIndex(associatedTabIndex)
{
    connect(this, SIGNAL(finished()), SLOT(animationHasFinished()));
}

// This method is called at every variation of the interpolation value, it must make sure that
// each tab's offset is updated
void SlideToPositionAnimation::updateCurrentValue(const QVariant &value) {
    // Aggiorna m_XTrackingDistance
    //qDebug() << "updateCurrentValue(" << value.toInt() << ")";
    //m_parent->m_XTrackingDistance = value.toInt();
    //m_parent->repaint();

    // Update the offset for the associated tab
    m_parent.m_tabs[m_associatedTabIndex].m_offset = value.toInt();
    // repaint?
    m_parent.repaint();
}

void SlideToPositionAnimation::animationHasFinished() {
    //m_parent.m_draggingInProgress = false;
}

VerticalSlideAnimation::VerticalSlideAnimation(TabsBar& parent, size_t associatedTabIndex ) :
    m_parent(parent),
    m_associatedTabIndex(associatedTabIndex)
{
}

// This method is called at every variation of the interpolation value, it must make sure that
// each tab's offset is updated
void VerticalSlideAnimation::updateCurrentValue(const QVariant &value) {
    // Aggiorna m_XTrackingDistance
    //qDebug() << "updateCurrentValue(" << value.toInt() << ")";
    //m_parent->m_XTrackingDistance = value.toInt();
    //m_parent->repaint();

    // Update the offset for the associated tab
    m_parent.m_tabs[m_associatedTabIndex].m_verticalOffset = value.toInt();
    // repaint?
    m_parent.repaint();
}
