#ifndef TABSBAR_H
#define TABSBAR_H

#include <QWidget>
#include <QMouseEvent>

#define TAB_MAXIMUM_WIDTH 150
#define TAB_INTERSECTION_DELTA 20

class Tab { // Questa classe rappresenta una tab del controllo
public:
    //TODO: il titolo
    std::string m_title;
    QPainterPath m_region;
};

class TabsBar : public QWidget { // Questa classe rappresenta l'intero controllo
    Q_OBJECT
public:
    explicit TabsBar( QWidget *parent = 0 );
    QPainterPath drawTabInsideRect(QPainter& p, const QRect& tabRect , bool selected ,
                                   const QPainterPath* sxTabRect = 0, const QPainterPath* dxTabRect = 0);
    void drawGrayHorizontalBar( QPainter& p, const QColor innerGrayCol );
private:

    void paintEvent ( QPaintEvent* );
    void mousePressEvent( QMouseEvent* evt );
    void mouseMoveEvent( QMouseEvent* evt );
    void mouseReleaseEvent( QMouseEvent* evt );

    QWidget *m_parent;
    std::vector<Tab> m_tabs; // Il vettore delle tabs
    int m_selectedTabIndex; // L'index della tab selezionata. -1 significa "nessuna"

    bool m_draggingInProgress;
    QPoint m_dragStartPosition;
    int m_XTrackingDistance; // La distanza dall'inizio del dragging per una tab, negativo o positivo
};

#endif // TABSBAR_H
