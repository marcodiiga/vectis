#ifndef CUSTOMSCROLLBAR_H
#define CUSTOMSCROLLBAR_H

#include <QScrollBar>
#include <QTextEdit>
#include <QPropertyAnimation>

class ScrollBar;
// This class "eats" PageUp/PageDown events from the QTextEdit widget to avoid interrupting the
// animation due to the caret being moved that modifies value()
class PgKeyEater : public QObject {
    Q_OBJECT

    explicit PgKeyEater ( ScrollBar *scrollBar ); // Private constructor
    ScrollBar *m_scrollBar;

    friend class ScrollBar; // Only ScrollBar can build this kind of filter
protected:
    bool eventFilter ( QObject *obj, QEvent *event );
};

class ScrollBar : public QScrollBar {
    Q_OBJECT
public:
    explicit ScrollBar( QAbstractScrollArea *parent = 0 );
    ~ScrollBar();

private:
    void paintEvent ( QPaintEvent* );
    void resizeEvent ( QResizeEvent * event );
    void sliderChange ( SliderChange change );
    void mousePressEvent ( QMouseEvent * e );
    void mouseReleaseEvent (QMouseEvent *);
    void mouseMoveEvent ( QMouseEvent *e );

    QAbstractScrollArea *m_parent;
    int   m_maxViewVisibleLines; // Lines that the current view of the text control can visualize
    qreal m_textLineHeight;
    int   m_internalLineCount; // Real lines of the text control (not multiplied by lineHeight)
    QPropertyAnimation m_scrollAnim;
    bool  m_sliderIsBeingDragged;
    int   m_mouseTrackingStartPoint;
    int   m_mouseTrackingStartValue;
    QPainterPath m_sliderPath;
    PgKeyEater m_pgKeyEater;

    friend class PgKeyEater; // Filter needs access to m_scrollAnim to indicate whether at the end of the
                             // animation the caret will have to be moved (PageUp/Down only)

private slots: // Consult the respective definitions for a thoroughly documentation of these methods,
               // they're not part of an external interface
    void documentSizeChanged ( const QSizeF & newSize, const qreal lineHeight );
    //void sliderPressed ();
    //void sliderReleased ();
    void actionTriggered ( int action );
    void moveParentCaret ();

signals:
    void sliderValueChanged( int value );
};

#endif // CUSTOMSCROLLBAR_H
