#ifndef TABSBAR_H
#define TABSBAR_H

#include <QWidget>

#define TAB_MAXIMUM_WIDTH 150
#define TAB_INTERSECTION_DELTA 20

class Tab { // Questa classe rappresenta una tab del controllo
public:
    //TODO: il titolo
    std::string m_title;
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

    QWidget *m_parent;
    std::list<Tab> m_tabs; // Il vettore delle tabs
    int m_selectedTabIndex; // L'index della tab selezionata. -1 significa "nessuna"
};

#endif // TABSBAR_H
