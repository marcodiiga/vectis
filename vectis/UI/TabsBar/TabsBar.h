#ifndef TABSBAR_H
#define TABSBAR_H

#include <QWidget>
#include <QMouseEvent>
#include <QVariantAnimation>
#include <memory>
#include <functional>
#include <set>

#define TAB_MAXIMUM_WIDTH 150
#define TAB_INTERSECTION_DELTA 20

class TabsBar;
class SlideToPositionAnimation;

class Tab { // This class represents a tab in the control
public:

    int getTabId() {
      return m_uniqueId;
    }

private:

    // Why are we using delegate constructors and declaring a private_access inner structure?
    //
    // We want to have Tab's private constructor accessible only to TabsBar and interpolators
    // but we also want to be able to construct objects in-place from TabsBar with
    //   tabs_vector.emplace_back("I'm a new tab");
    // The problem with the approach above is: emplace_back delegates construction of the
    // Tab object to std::allocator (construct()) and this is not a friend of Tab.
    //
    // To workaround this issue we can either supply a custom TabAllocator which, being an inner class,
    // as of [class.access.nest]/p1
    //
    //   "A nested class is a member and as such has the same access rights as any other member."
    //
    // would allow TabsBar to emplace_back Tabs by using the custom member allocator provided.
    //
    // Unfortunately this doesn't work on MSVC and thus we need a plan B: a public constructor
    //  Tab(Qstring, const private_access&)
    // that, since requires a private_access structure (privately declared), can only be called by
    // a friend class. Since the constructor (called by the allocator) then delegates the actual object
    // construction to another constructor (C++11 and above only), this will ensure there will be no
    // access problems.

    friend class TabsBar;
    friend class SlideToPositionAnimation;
    friend class CloseBtnInterpolator;

    Tab(QString title) : m_title(title) {}

    struct private_access {};

public:
    Tab (QString title, const private_access&) : // Only accessible to friend classes
      Tab(title)
    {}
private:

    QString m_title;
    QPainterPath m_region;
    QPainterPath m_closeBtnRegion;

    // Every tab is referred to with a unique id. Notice: this must *NOT* be confused with the
    // tab's index into the tab control (i.e. its position into the control)
    int m_uniqueId;

    // To animate the "scroll towards the equilibrium position" effect, it is necessary having
    // a rect and an offset. The equilibrium position has always offset zero.
    QRect m_rect;
    int m_Xoffset = 0;
    int m_Yoffset = 0; // There is also a vertical offset for the entering/exiting tab animation
};

// This class implements an interpolation towards the tabs equilibrium positions, i.e. the
// fluid scrolling effect
class SlideToPositionAnimation: public QVariantAnimation {
    Q_OBJECT

    SlideToPositionAnimation(TabsBar& parent, int associatedTabIndex, bool isHorizontalOffset );
    struct private_access {};
public:
    SlideToPositionAnimation (TabsBar& parent, int associatedTabIndex, bool isHorizontalOffset,
                              const private_access&) : // Only accessible to friend classes
      SlideToPositionAnimation(parent, associatedTabIndex, isHorizontalOffset)
    {}
private:

    friend class TabsBar;
    void updateCurrentValue(const QVariant &value) override;

    TabsBar& m_parent;
    int m_associatedTabIndex;
    bool m_isHorizontalOffset;
    std::function<void(TabsBar&)> finishCallback;

private slots:
    void animationHasFinished();
};

class TabsBar : public QWidget { // This class represents the entire control
    Q_OBJECT
public:
    explicit TabsBar( QWidget *parent = 0 );
    int insertTab(const QString text, bool animation = true);
    void deleteTab(int id, bool animation = true);
    int getSelectedTabId();

private:

    struct TabPaths {
        QPainterPath tabRegion;
        QPainterPath closeBtnRegion;
    };

    TabPaths drawTabInsideRect(QPainter& p, const QRect& tabRect , bool selected , QString text = "",
                               bool mouseHoveringXBtn = false);
    void drawGrayHorizontalBar( QPainter& p, const QColor innerGrayCol );

    friend class SlideToPositionAnimation;

    void paintEvent ( QPaintEvent* );
    void mousePressEvent( QMouseEvent* evt );
    void mouseMoveEvent( QMouseEvent* evt );
    void mouseReleaseEvent(QMouseEvent*);

    QWidget *m_parent;
    std::vector<std::unique_ptr<Tab>> m_tabs; // The tabs vector
    int m_selectedTabIndex; // Index of selected tab. -1 means "no one"

    // Nb. there are two different kind of indices:
    //  - Tab id -> this is unique for every tab and can never change
    //  - Tab index -> this is the position of the tab in the control vector and might be change (swap)
    // Users only deal with tab ids
    std::map<int, int> m_tabId2tabIndexMap; // The tab_id->control_position_index map for the tabs
    std::set<int> m_tabIdHoles; // The non-contiguous tab ids (left by deleted tabs)

    bool m_draggingInProgress;
    QPoint m_dragStartPosition;
    int m_selectionStartIndex;
    int m_XTrackingDistance; // Distance from the beginning of the tracking for a tab, negative or positive
    int tabWidth; // Half of this distance has to be surpassed to allow swapping a tab with another

    // Two horizontal and vertical vector of interpolators. QObjects aren't copyable due to memory management
    // issues, thus it is necessary to use pointers to their memory to move them in different cells
    std::vector<std::unique_ptr<SlideToPositionAnimation>> m_XInterpolators;
    std::vector<std::unique_ptr<SlideToPositionAnimation>> m_YInterpolators;
    int m_mouseHoveringCloseBtnTabIndex; // The index of the tab whose X button the mouse is hovering on (or -1)

    // Any time text is drawn on a tab, it has an opacity mask on it to fade it out before the 'x' button.
    // The opacity mask can be cached with this variable until the width of all the tabs changes
    std::unique_ptr<QPixmap> m_textOpacityMask;
    void recalculateOpacityMask(QRectF newTabRect);

    // Emit a signal of selection changed (notice that the parameters are tab IDs, not relative indices into the tabs bar).
    // oldTabIdIndex can be -1 if the old tab is no longer available (i.e. deleted) or if there were no one (a first tab
    // ever has been created)
    void emitSelectionHasChanged(int /* old tab index */ oldTabIdIndex, int /* new tab index */ newTabIdIndex);

signals:
    // As the name suggests, this might be useful to signal a view refresh.
    // Sends the id of the newly selected tab (and of the old one)
    void selectedTabHasChanged(int oldTabId, int newTabId);
    // Sends the id of the tab which has been asked to close down (it hasn't been closed yet)
    void tabWasRequestedToClose(int tabId);
};

#endif // TABSBAR_H
