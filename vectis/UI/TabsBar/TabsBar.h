#ifndef TABSBAR_H
#define TABSBAR_H

#include <QWidget>
#include <QMouseEvent>
#include <QVariantAnimation>
#include <memory>

#define TAB_MAXIMUM_WIDTH 150
#define TAB_INTERSECTION_DELTA 20

class Tab { // This class represents a tab in the control
public:
    Tab(QString title) : m_title(title) {}

    QString m_title;
    QPainterPath m_region;

    // To animate the "scroll towards the equilibrium position" effect, it is necessary having
    // a rect and an offset. The equilibrium position has always offset zero.
    QRect m_rect;
    int m_Xoffset = 0;
    int m_Yoffset = 0; // There is also a vertical offset for the entering/exiting tab animation
};

class TabsBar;

// This class implements an interpolation towards the tabs equilibrium positions, i.e. the
// fluid scrolling effect
class SlideToPositionAnimation: public QVariantAnimation {
    Q_OBJECT
public:
    SlideToPositionAnimation(TabsBar& parent, size_t associatedTabIndex, bool isHorizontalOffset );

    void updateCurrentValue(const QVariant &value) override;

private:
    TabsBar& m_parent;
    size_t m_associatedTabIndex;
    bool m_isHorizontalOffset;

private slots:
    void animationHasFinished();
};

class TabsBar : public QWidget { // This class represents the entire control
    Q_OBJECT
public:
    explicit TabsBar( QWidget *parent = 0 );
    void insertTab(const QString text);

private:
    QPainterPath drawTabInsideRect(QPainter& p, const QRect& tabRect , bool selected , QString text = "");
    void drawGrayHorizontalBar( QPainter& p, const QColor innerGrayCol );

    friend class SlideToPositionAnimation;

    void paintEvent ( QPaintEvent* );
    void mousePressEvent( QMouseEvent* evt );
    void mouseMoveEvent( QMouseEvent* evt );
    void mouseReleaseEvent( QMouseEvent* evt );

    QWidget *m_parent;
    std::vector<Tab> m_tabs; // The tabs vector
    int m_selectedTabIndex; // Index of selected tab. -1 means "no one"

    bool m_draggingInProgress;
    QPoint m_dragStartPosition;
    int m_selectionStartIndex;
    int m_XTrackingDistance; // Distance from the beginning of the tracking for a tab, negative or positive
    int tabWidth; // Half of this distance has to be surpassed to allow swapping a tab with another

    // Two horizontal and vertical vector of interpolators. QObjects aren't copyable due to memory management
    // issues, thus it is necessary to use pointers to their memory to move them in different cells
    std::vector<std::unique_ptr<SlideToPositionAnimation>> m_XInterpolators;
    std::vector<std::unique_ptr<SlideToPositionAnimation>> m_YInterpolators;
};

#endif // TABSBAR_H
