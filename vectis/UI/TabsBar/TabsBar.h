#ifndef TABSBAR_H
#define TABSBAR_H

#include <QWidget>
#include <memory>

class Tab { // Questa classe rappresenta una tab del controllo
public:
    //TODO: il titolo
    std::string m_title;
};

class TabsBar : public QWidget { // Questa classe rappresenta l'intero controllo
    Q_OBJECT
public:
    explicit TabsBar( QWidget *parent = 0 );
    QPainterPath drawTabInsideRect(QPainter& p, QRect& tabRect , bool selected ,
                                   const QPainterPath* sxTabRect = 0, const QPainterPath* dxTabRect = 0);
private:

    void paintEvent ( QPaintEvent* );

    QWidget *m_parent;
    std::list<std::unique_ptr<Tab>> m_tabs; // Il vettore delle tabs
    size_t m_selectedTabIndex; // L'index della tab selezionata. -1 significa "nessuna"
};

#endif // TABSBAR_H
