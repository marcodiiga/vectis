#ifndef TABSBAR_H
#define TABSBAR_H

#include <QWidget>

class Tab { // Questa classe rappresenta una tab del controllo
public:
};

class TabsBar : public QWidget { // Questa classe rappresenta l'intero controllo
    Q_OBJECT
public:
    explicit TabsBar( QWidget *parent = 0 );
private:

    void paintEvent ( QPaintEvent* );

    QWidget *m_parent;
    std::vector<Tab> m_tabs; // Il vettore delle tabs
};

#endif // TABSBAR_H
