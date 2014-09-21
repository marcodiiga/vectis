#ifndef CUSTOMSCROLLBAR_H
#define CUSTOMSCROLLBAR_H

#include <QScrollBar>
#include <QTextEdit>
#include <QPropertyAnimation>
#include <memory>


#include <QCommonStyle>
 #include <QStyleOption>
#include <QDebug>
#include <QPainter>

class CustomStyle : public QCommonStyle
 {
     Q_OBJECT

 public:
     CustomStyle() {}
     ~CustomStyle() {}


    void drawComplexControl ( ComplexControl control, const QStyleOptionComplex * option,
                                              QPainter * painter, const QWidget * widget = 0 ) const {
        //const QStyleOptionSlider* sosl = qstyleoption_cast<const QStyleOptionSlider*>(option);

        switch (control) {
            case CC_ScrollBar: {

            // From style()->drawComplexControl(QStyle::CC_ScrollBar
            if (const QStyleOptionSlider *scrollbar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
                qDebug() << "drawing complex control";
                // Make a copy here and reset it for each primitive.
                QStyleOptionSlider newScrollbar = *scrollbar;
                State saveFlags = scrollbar->state;

                if (scrollbar->subControls & SC_ScrollBarSubLine) {
                    newScrollbar.state = saveFlags;
                    newScrollbar.rect = proxy()->subControlRect(control, &newScrollbar, SC_ScrollBarSubLine, widget);
                    if (newScrollbar.rect.isValid()) {
                        if (!(scrollbar->activeSubControls & SC_ScrollBarSubLine))
                            newScrollbar.state &= ~(State_Sunken | State_MouseOver);
                        proxy()->drawControl(CE_ScrollBarSubLine, &newScrollbar, painter, widget);
                    }
                }
                if (scrollbar->subControls & SC_ScrollBarAddLine) {
                    newScrollbar.rect = scrollbar->rect;
                    newScrollbar.state = saveFlags;
                    newScrollbar.rect = proxy()->subControlRect (control, &newScrollbar, SC_ScrollBarAddLine, widget);
                    if (newScrollbar.rect.isValid()) {
                        if (!(scrollbar->activeSubControls & SC_ScrollBarAddLine))
                            newScrollbar.state &= ~(State_Sunken | State_MouseOver);
                        proxy()->drawControl(CE_ScrollBarAddLine, &newScrollbar, painter, widget);
                    }
                }
                if (scrollbar->subControls & SC_ScrollBarSubPage) {
                    newScrollbar.rect = scrollbar->rect;
                    newScrollbar.state = saveFlags;
                    newScrollbar.rect = proxy()->subControlRect(control, &newScrollbar, SC_ScrollBarSubPage, widget);
                    if (newScrollbar.rect.isValid()) {
                        if (!(scrollbar->activeSubControls & SC_ScrollBarSubPage))
                            newScrollbar.state &= ~(State_Sunken | State_MouseOver);
                        proxy()->drawControl(CE_ScrollBarSubPage, &newScrollbar, painter, widget);
                    }
                }
                if (scrollbar->subControls & SC_ScrollBarAddPage) {

                    newScrollbar.rect = scrollbar->rect;
                    newScrollbar.state = saveFlags;

                    newScrollbar.rect = proxy()->subControlRect(control, &newScrollbar, SC_ScrollBarAddPage, widget);

                    if (newScrollbar.rect.isValid()) {

                        if (!(scrollbar->activeSubControls & SC_ScrollBarAddPage))
                            newScrollbar.state &= ~(State_Sunken | State_MouseOver);

                        proxy()->drawControl(CE_ScrollBarAddPage, &newScrollbar, painter, widget);
                    }
                }
                if (scrollbar->subControls & SC_ScrollBarFirst) {
                    newScrollbar.rect = scrollbar->rect;
                    newScrollbar.state = saveFlags;
                    newScrollbar.rect = proxy()->subControlRect(control, &newScrollbar, SC_ScrollBarFirst, widget);

                    if (newScrollbar.rect.isValid()) {

                        if (!(scrollbar->activeSubControls & SC_ScrollBarFirst))
                            newScrollbar.state &= ~(State_Sunken | State_MouseOver);
                        proxy()->drawControl(CE_ScrollBarFirst, &newScrollbar, painter, widget);
                    }
                }
                if (scrollbar->subControls & SC_ScrollBarLast) {
                    newScrollbar.rect = scrollbar->rect;
                    newScrollbar.state = saveFlags;
                    newScrollbar.rect = proxy()->subControlRect(control, &newScrollbar, SC_ScrollBarLast, widget);
                    if (newScrollbar.rect.isValid()) {
                        if (!(scrollbar->activeSubControls & SC_ScrollBarLast))
                            newScrollbar.state &= ~(State_Sunken | State_MouseOver);
                        proxy()->drawControl(CE_ScrollBarLast, &newScrollbar, painter, widget);
                    }
                }
                if (scrollbar->subControls & SC_ScrollBarSlider) {
                    newScrollbar.rect = scrollbar->rect;
                    newScrollbar.state = saveFlags;
                    newScrollbar.rect = proxy()->subControlRect(control, &newScrollbar, SC_ScrollBarSlider, widget);
                    if (newScrollbar.rect.isValid()) {
                        if (!(scrollbar->activeSubControls & SC_ScrollBarSlider))
                            newScrollbar.state &= ~(State_Sunken | State_MouseOver);
                        proxy()->drawControl(CE_ScrollBarSlider, &newScrollbar, painter, widget);

                        if (scrollbar->state & State_HasFocus) {
                            QStyleOptionFocusRect fropt;
                            fropt.QStyleOption::operator=(newScrollbar);
                            fropt.rect.setRect(newScrollbar.rect.x() + 2, newScrollbar.rect.y() + 2,
                                               newScrollbar.rect.width() - 5,
                                               newScrollbar.rect.height() - 5);
                            proxy()->drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
                        }
                    }
                }
            }
        }break;
        default:
            QCommonStyle::drawComplexControl(control, option, painter, widget);
        }
     }
 };

class CustomScrollBar;
// Questa classe si occupa di "mangiare" gli eventi PageUp/PageDown
// da parte del QTextEdit widget per evitare il fastidioso interrompere
// dell'animazione da parte del caret mosso che modifica value()
class PgKeyEater : public QObject
{
    Q_OBJECT

    friend class CustomScrollBar;
    explicit PgKeyEater ( CustomScrollBar *scrollBar );
    CustomScrollBar *m_scrollBar;

protected:
    bool eventFilter ( QObject *obj, QEvent *event );
};

class CustomScrollBar : public QScrollBar
{
    Q_OBJECT
public:
    CustomScrollBar(QTextEdit * parent = 0);
    ~CustomScrollBar();

    friend class PgKeyEater;

private:
    void paintEvent ( QPaintEvent * event );
    void resizeEvent ( QResizeEvent * event );
    void sliderChange ( SliderChange change );

    QTextEdit * m_parent;
    int   m_maxNumLines;
    qreal m_textLineHeight;
    int   m_internalLineCount;
    //QTimer * m_scrollTimer;
    std::unique_ptr<QPropertyAnimation> m_scrollAnim;
    bool  m_sliderIsBeingDragged;
    std::unique_ptr<PgKeyEater> m_pgKeyEater;

private slots:
    void documentSizeChanged ( const QSizeF & newSize );
    void sliderPressed ();
    void sliderReleased ();
    void actionTriggered ( int action );
    void moveParentCaret ();

signals:

public slots:
};


#endif // CUSTOMSCROLLBAR_H
