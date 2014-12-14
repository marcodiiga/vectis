#ifndef TABSBAR_H
#define TABSBAR_H

#include <QWidget>
#include <QMouseEvent>
#include <QVariantAnimation>
#include <memory>

#define TAB_MAXIMUM_WIDTH 150
#define TAB_INTERSECTION_DELTA 20

class Tab { // Questa classe rappresenta una tab del controllo
public:
    Tab(QString title) : m_title(title) {}

    QString m_title;
    QPainterPath m_region;

    // Per animare l'effetto "scroll verso la posizione di equilibrio" è necessario avere
    // una rect e un offset. La posizione di equilibrio è sempre quella dove l'offset è zero.
    QRect m_rect;
    int m_offset = 0;
};

class TabsBar;

// Questa classe implementa una interpolazione verso le posizioni di equilibrio delle tab,
// ovvero l'effetto "scorrimento" fluido
class SlideToPositionAnimation: public QVariantAnimation {
    Q_OBJECT
public:
    SlideToPositionAnimation( TabsBar& parent, size_t associatedTabIndex );

    void updateCurrentValue(const QVariant &value) override;

private:
    TabsBar& m_parent;
    size_t m_associatedTabIndex;

private slots:
    void animationHasFinished();
};

class TabsBar : public QWidget { // Questa classe rappresenta l'intero controllo
    Q_OBJECT
public:
    explicit TabsBar( QWidget *parent = 0 );
    QPainterPath drawTabInsideRect(QPainter& p, const QRect& tabRect , bool selected , QString text = "",
                                   const QPainterPath* sxTabRect = 0, const QPainterPath* dxTabRect = 0);
    void drawGrayHorizontalBar( QPainter& p, const QColor innerGrayCol );
private:

    friend class SlideToPositionAnimation;

    void paintEvent ( QPaintEvent* );
    void mousePressEvent( QMouseEvent* evt );
    void mouseMoveEvent( QMouseEvent* evt );
    void mouseReleaseEvent( QMouseEvent* evt );

    QWidget *m_parent;
    std::vector<Tab> m_tabs; // Il vettore delle tabs
    int m_selectedTabIndex; // L'index della tab selezionata. -1 significa "nessuna"

    bool m_draggingInProgress;
    QPoint m_dragStartPosition;
    int m_selectionStartIndex;
    int m_XTrackingDistance; // La distanza dall'inizio del dragging per una tab, negativo o positivo
    int tabWidth; // La metà di questa distanza è da superare per consentire lo swap di una tab con un'altra
    //SlideToPositionAnimation m_slideAnimation;
    std::vector<std::unique_ptr<SlideToPositionAnimation>> m_interpolators; // Un vettore di interpolatori. QObjects non
                                                        // sono copiabili per motivi di memory management, quindi è neces-
                                                        // sario utilizzare dei puntatori alla loro memoria per copiarli
};

#endif // TABSBAR_H
