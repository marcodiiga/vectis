#ifndef CUSTOMSCROLLBAR_H
#define CUSTOMSCROLLBAR_H

#include <QScrollBar>
#include <QTextEdit>
#include <QPropertyAnimation>
#include <memory>

class ScrollBar;
// Questa classe si occupa di "mangiare" gli eventi PageUp/PageDown da parte del QTextEdit widget per
// evitare il fastidioso interrompere dell'animazione da parte del caret mosso che modifica value()
class PgKeyEater : public QObject {
    Q_OBJECT

    explicit PgKeyEater ( ScrollBar *scrollBar ); // Costruttore privato
    ScrollBar *m_scrollBar;

    friend class ScrollBar; // Solo la ScrollBar può costruire questo tipo di filtro
protected:
    bool eventFilter ( QObject *obj, QEvent *event );
};

class ScrollBar : public QScrollBar {
    Q_OBJECT
public:
    ScrollBar( QTextEdit * parent = 0 );
    ~ScrollBar();

private:
    void paintEvent ( QPaintEvent* );
    void resizeEvent ( QResizeEvent * event );
    void sliderChange ( SliderChange change );

    QTextEdit * m_parent;
    int   m_maxViewVisibleLines; // Le righe che la view corrente del controllo testo può visualizzare
    qreal m_textLineHeight;
    int   m_internalLineCount; // Le righe reali del controllo testo (non è moltiplicato per lineHeight)
    std::unique_ptr<QPropertyAnimation> m_scrollAnim;
    bool  m_sliderIsBeingDragged;
    std::unique_ptr<PgKeyEater> m_pgKeyEater;

    friend class PgKeyEater; // Il filtro ha bisogno di accedere alla m_scrollAnim per indicare se alla fine
                             // dell'animazione il caret dovrà essere spostato o meno (PgUp/Down soltanto)

private slots: // Per la documentazione per ogni singolo metodo consultare le rispettive definizioni,
               // questi metodi non fanno parte di una interfaccia esterna
    void documentSizeChanged ( const QSizeF & newSize );
    void sliderPressed ();
    void sliderReleased ();
    void actionTriggered ( int action );
    void moveParentCaret ();
};

#endif // CUSTOMSCROLLBAR_H
