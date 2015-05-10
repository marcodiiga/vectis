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

    insertTab("First tab with longest text", false);
    insertTab("Sec tab", false);
    insertTab("Third tab", false);



    m_selectedTabIndex = 1;
    //DEBUG
}

// Dynamically inserts a tab into the control by providing a "sliding" vertical animation
int TabsBar::insertTab(const QString text, bool animation) {
    // Create the tab object and its interpolators
    m_tabs.emplace_back(std::make_unique<Tab>(text, Tab::private_access()));
    int newTabIndex = static_cast<int>(m_tabs.size() - 1);
    m_XInterpolators.emplace_back(std::make_unique<SlideToPositionAnimation>(*this, newTabIndex, true,
                                                                             SlideToPositionAnimation::private_access()));
    m_YInterpolators.emplace_back(std::make_unique<SlideToPositionAnimation>(*this, newTabIndex, false,
                                                                             SlideToPositionAnimation::private_access()));

    // Assign a free id to the tab (this is not the tab index in the control)
    auto getFreeIdFromPool = [&](int tabIndex) {
      if (m_tabIdHoles.begin() != m_tabIdHoles.end()) { // If there's a hole in the pool set, grab it and fill it
        int id = *m_tabIdHoles.begin();
        m_tabIdHoles.erase(m_tabIdHoles.begin());
        m_tabId2tabIndexMap[id] = tabIndex;
        return id;
      } else { // else simply grab the next free id
        int id = static_cast<int>(m_tabId2tabIndexMap.size());
        m_tabId2tabIndexMap[id] = tabIndex;
        return id;
      }
    };
    int newId = getFreeIdFromPool(newTabIndex);
    m_tabs[newTabIndex]->m_uniqueId = newId;

    // If asked, start a Y interpolator with a slide-in animation
    if (animation) {
      m_YInterpolators[newTabIndex]->setDuration(100);
      m_YInterpolators[newTabIndex]->setStartValue(35);
      m_YInterpolators[newTabIndex]->setEndValue(0);
      m_YInterpolators[newTabIndex]->start();
    }

    m_selectedTabIndex = newTabIndex; // Also make it the new selected one
    repaint();

    return newId;
}

// Deletes a tab from the control by providing a "sliding" vertical animation
void TabsBar::deleteTab(int id, bool animation) {
  if(m_tabs.size() == 0)
    return; // No tabs to delete

  auto tabIdMapIterator = m_tabId2tabIndexMap.find(id);
  bool validId = tabIdMapIterator != m_tabId2tabIndexMap.end();
  Q_ASSERT(validId == true);

  int deleteTabIndex = tabIdMapIterator->second;
  int deleteTabId = id;

  // Create a deletion callback to be executed only when the animation has finished
  auto deletionCallback = [=] (TabsBar& tabsBar) {

    int delTabId = deleteTabId;
    int delTabIndex = deleteTabIndex; // Avoid MSVC problems with lambda value capture

    // Actually delete tab and interpolators
    tabsBar.m_XInterpolators.erase(tabsBar.m_XInterpolators.begin() + delTabIndex);
    tabsBar.m_YInterpolators.erase(tabsBar.m_YInterpolators.begin() + delTabIndex);
    tabsBar.m_tabs.erase(tabsBar.m_tabs.begin() + delTabIndex);

    // Updates all right interpolators to refer to the right position in the tabs vector and
    // updates all right tabs in the tabId2tabIndex map
    for (int i=delTabIndex; i<tabsBar.m_tabs.size(); ++i) {
      tabsBar.m_XInterpolators[i]->m_associatedTabIndex = i; // Assumes same size
      tabsBar.m_YInterpolators[i]->m_associatedTabIndex = i; // Assumes same size
      tabsBar.m_tabId2tabIndexMap[tabsBar.m_tabs[i]->getTabId()] = i;
    }

    // Release the tab id and put it into the holes vector (to be grabbed the next time)
    tabsBar.m_tabIdHoles.insert(delTabId);

    // If this was the selected tab, make sure to have another selected before repainting
    if (tabsBar.m_selectedTabIndex == delTabIndex) {
      if (tabsBar.m_tabs.size() == 0)
        tabsBar.m_selectedTabIndex = -1; // We deleted the only one in the control
      else { // Find a selection substitute
        // Just leave the id as it was if there was a right tab (it will take our selection)
        // (in case m_selectedTabIndex < m_tabs.size()), otherwise grab the leftmost one
        if (tabsBar.m_selectedTabIndex >= tabsBar.m_tabs.size())
          tabsBar.m_selectedTabIndex = static_cast<int>(tabsBar.m_tabs.size()) - 1;
      }
      // TODO: RELOAD DOCUMENT EVENT HERE
    } else { // Keep the selected one active
      // TODO: NO RELOAD DOCUMENT EVENT HERE
      if (delTabIndex  < tabsBar.m_selectedTabIndex)
        tabsBar.m_selectedTabIndex--;
      // No need to do anything if it was on the right
    }

    tabsBar.repaint();
  };

  if (animation == true) {
    // Start a Y interpolator with a slide-out animation and set a finish callback
    m_YInterpolators[deleteTabIndex]->finishCallback = deletionCallback;
    m_YInterpolators[deleteTabIndex]->setDuration(150);
    m_YInterpolators[deleteTabIndex]->setStartValue(0);
    m_YInterpolators[deleteTabIndex]->setEndValue(35);
    m_YInterpolators[deleteTabIndex]->start();
  } else {
    deletionCallback(*this); // Just call the callback
    repaint();
  }
}

// Returns the selected tab id (NOT index) - users only deal with unique ids
int TabsBar::getSelectedTabId () {
  if (m_selectedTabIndex != -1)
    return m_tabs[m_selectedTabIndex]->getTabId();
  else
    return -1;
}

// Recalculates the opacity mask m_textOpacityMask in case the width has changed
void TabsBar::recalculateOpacityMask(QRectF newTabRect) {
  if (m_textOpacityMask && newTabRect.width() == m_textOpacityMask->width())
    return; // Nothing has changed

  qDebug() << "Recalculating tab opacity mask (changed to" << newTabRect.width() << ")";
  // Recalculate the gradient opacity mask for the text that goes on the tabs
  m_textOpacityMask.reset(new QPixmap(newTabRect.width(),newTabRect.height()));
  m_textOpacityMask->fill(Qt::transparent);

  const QPoint start(0, 0);
  const QPoint end(m_textOpacityMask->width(), 0);
  QLinearGradient gradient(start, end);
  gradient.setColorAt(0.0, Qt::white);
  gradient.setColorAt(0.8, Qt::white);
  gradient.setColorAt(1.0, Qt::transparent);

  QPainter painter(m_textOpacityMask.get());
  painter.fillRect(m_textOpacityMask->rect(), gradient);
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

    // Calculate the subrectangle where to draw the text
    QRectF textRect = tabRect;
    textRect.setX(tabRect.x() + h);
    textRect.setWidth(tabRect.width() - 2*h -h/4 /* This last one is for the 'x' button */);
    textRect.setY(textRect.y() + 6);

    recalculateOpacityMask (textRect); // Recalculate the tab's text opacity mask in case width has changed

    // Off-screen renders the text plus opacity mask in a pixMap before actually drawing it into the control
    QPixmap pixMap(textRect.width(),textRect.height());
    pixMap.fill(Qt::transparent);

    QPainter textPixmapPainter(&pixMap);
    // Choose a different color if the tab is selected or unselected
    if( selected == true )
        textPixmapPainter.setPen( Qt::white );
    else
        textPixmapPainter.setPen( QPen(QColor(193,193,191)) );
    textPixmapPainter.drawText(pixMap.rect(), Qt::AlignLeft, QString(text)); // Draw the text on the off-screen pixmap

    // Destination (aka the pixmap with the text) is the output, its alpha is reduced by that of the source (the mask)
    textPixmapPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);

    // Draw the pre-cached alpha mask over the text rectangle to set its alpha channel
    textPixmapPainter.drawPixmap(0,0, m_textOpacityMask->width(), m_textOpacityMask->height(), *m_textOpacityMask.get());

    p.drawPixmap(textRect, pixMap, pixMap.rect()); // Finally draw the pixmap with the text drawn + the right alpha mask
                                                   // in the right textRect on the control

    return tabPath; // Notice: this might be used to fade the selected tab's borders or other graphic manipulations
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

        QPainterPath temp;
        // This lambda takes care of calculating the right rect position for a tab with a given index
        // and of calling the drawing function
        auto calculatePositionAndDrawTab = [&](int i, bool leftTabs) {
          int x = 5 + i * tabWidth;
          x -= TAB_INTERSECTION_DELTA * i;
          standardTabRect.setX( x );
          // If we have a 'decreasing' offset, add it
          if(m_tabs[i]->m_Xoffset != 0) {
              if (leftTabs)
                standardTabRect.setX(standardTabRect.x() + m_tabs[i]->m_Xoffset);
              else
                standardTabRect.setX(standardTabRect.x() - m_tabs[i]->m_Xoffset);
          }
          if(m_tabs[i]->m_Yoffset != 0) {
              standardTabRect.setY(standardTabRect.y() + m_tabs[i]->m_Yoffset);
              standardTabRect.setBottom(standardTabRect.bottom() + m_tabs[i]->m_Yoffset);
          }
          standardTabRect.setWidth( tabWidth );

          temp = drawTabInsideRect( p, standardTabRect, false, m_tabs[i]->m_title );

          m_tabs[i]->m_region = temp;

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
        } else if(m_tabs[m_selectedTabIndex]->m_Xoffset != 0) {
            x += m_tabs[m_selectedTabIndex]->m_Xoffset;
        }
        if(m_tabs[m_selectedTabIndex]->m_Yoffset != 0) {
            standardTabRect.setY(standardTabRect.y() + m_tabs[m_selectedTabIndex]->m_Yoffset);
            standardTabRect.setBottom(standardTabRect.bottom() + m_tabs[m_selectedTabIndex]->m_Yoffset);
        }
        standardTabRect.setX( x );
        standardTabRect.setWidth( tabWidth );

        m_tabs[m_selectedTabIndex]->m_region = drawTabInsideRect( p, standardTabRect, true, m_tabs[m_selectedTabIndex]->m_title);
        m_tabs[m_selectedTabIndex]->m_rect = standardTabRect;
    }
}

// This event deals with mouse click to change a selected tab
void TabsBar::mousePressEvent(QMouseEvent *evt) {
    if ( evt->button() == Qt::LeftButton ) {
        m_dragStartPosition = evt->pos();
        m_selectionStartIndex = m_selectedTabIndex;
        for( int i=0; i<static_cast<int>(m_tabs.size()); ++i ) {
            if(m_tabs[i]->m_region.contains(m_dragStartPosition) == true) {
                m_selectedTabIndex = i;
                repaint();
                // TODO: send a signal "changedSelectedTab" with the integer 'm_selectedTabIndex'
            }
        }
    }
    qDebug() << "mousePressEvent " << m_dragStartPosition;
}

// This event deals with tab dragging (tracking)
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
                m_tabId2tabIndexMap[m_tabs[m_selectedTabIndex]->getTabId()] = m_selectedTabIndex+1;
                m_tabId2tabIndexMap[m_tabs[m_selectedTabIndex+1]->getTabId()] = m_selectedTabIndex;
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
                m_tabs[m_selectedTabIndex]->m_Xoffset = offset; // Must go to zero

                ++m_selectedTabIndex;
                m_XInterpolators[m_selectedTabIndex-1]->setDuration(200);
                m_XInterpolators[m_selectedTabIndex-1]->setStartValue(offset);
                m_XInterpolators[m_selectedTabIndex-1]->setEndValue(0);
                m_XInterpolators[m_selectedTabIndex-1]->start();
                setUpdatesEnabled(true);
            }
        } else {
            if( m_selectedTabIndex > 0 )
                // Swap the contents of the tab vector and update the new index
                setUpdatesEnabled(false);   // Disable paint() events UNTIL all updates are finished & the 'dethroned' tab has its
                                            // movement interpolator set
                //qDebug() << "Swap current tab (index: " << m_selectedTabIndex << ") with tab index: " << m_selectedTabIndex-1;
                m_tabId2tabIndexMap[m_tabs[m_selectedTabIndex]->getTabId()] = m_selectedTabIndex-1;
                m_tabId2tabIndexMap[m_tabs[m_selectedTabIndex-1]->getTabId()] = m_selectedTabIndex;
                std::swap(m_tabs[m_selectedTabIndex], m_tabs[m_selectedTabIndex-1]);

                // Same reasoning (inverted) as the case above
                m_XTrackingDistance = +(tabWidth / 2) - TAB_INTERSECTION_DELTA;
                m_dragStartPosition.setX(m_dragStartPosition.x() - tabWidth + TAB_INTERSECTION_DELTA);

                // Starts the interpolator for the swap of the other tab (it must come back to its equilibrium position)
                int offset = tabWidth - TAB_INTERSECTION_DELTA;
                m_tabs[m_selectedTabIndex]->m_Xoffset = offset; // Must go to zero

                --m_selectedTabIndex;
                m_XInterpolators[m_selectedTabIndex+1]->setDuration(200);
                m_XInterpolators[m_selectedTabIndex+1]->setStartValue(offset);
                m_XInterpolators[m_selectedTabIndex+1]->setEndValue(0);
                m_XInterpolators[m_selectedTabIndex+1]->start();
                setUpdatesEnabled(true);
        }
    }


    m_draggingInProgress = true;
    repaint();
}

// Signal the end of a tracking event
void TabsBar::mouseReleaseEvent(QMouseEvent *) {
    if( m_draggingInProgress == false )
        return;

    qDebug() << "Tracking ended";

    // Animate the "return" to the correct position, i.e. decreases the XTrackingDistance to zero
    m_tabs[m_selectedTabIndex]->m_Xoffset = m_XTrackingDistance; // Must go to zero
    m_XInterpolators[m_selectedTabIndex]->setDuration(200);
    m_XInterpolators[m_selectedTabIndex]->setStartValue(m_XTrackingDistance);
    m_XInterpolators[m_selectedTabIndex]->setEndValue(0);
    m_XInterpolators[m_selectedTabIndex]->start();

    m_draggingInProgress = false;
}



SlideToPositionAnimation::SlideToPositionAnimation(TabsBar& parent, int associatedTabIndex , bool isHorizontalOffset) :
    m_parent(parent),
    m_associatedTabIndex(associatedTabIndex),
    m_isHorizontalOffset(isHorizontalOffset)
{
    connect(this, SIGNAL(finished()), SLOT(animationHasFinished()));
}

// This method is called at every variation of the interpolation value, it must make sure that
// each tab's offset is updated
void SlideToPositionAnimation::updateCurrentValue(const QVariant &value) {

    // Update the offset for the associated tab (either vertical or horizontal)
    if (m_isHorizontalOffset)
        m_parent.m_tabs[m_associatedTabIndex]->m_Xoffset = value.toInt();
    else
        m_parent.m_tabs[m_associatedTabIndex]->m_Yoffset = value.toInt();

    m_parent.repaint();
}

void SlideToPositionAnimation::animationHasFinished() {
    if (finishCallback) // operator(bool) indicates if this is a callable function
      finishCallback (m_parent);
}
