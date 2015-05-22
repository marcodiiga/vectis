#ifndef CUSTOMCODEEDIT_H
#define CUSTOMCODEEDIT_H
#include <UI/CodeTextEdit/Document.h>
#include <UI/ScrollBar/ScrollBar.h>
#include <QAbstractScrollArea>
#include <memory>

// The code and text edit control (everything gets rendered to it)
class CodeTextEdit : public QAbstractScrollArea {
    Q_OBJECT
public:
    explicit CodeTextEdit(QWidget *parent = 0);

    void loadDocument(Document *doc);
    int getViewportWidth() const;
    int getCharacterWidthPixels() const;

private:
    void paintEvent(QPaintEvent *event);
    void renderDocumentOnPixmap();
    std::unique_ptr<QPixmap> m_documentPixmap; // The rendered document
    bool m_invalidatedPixmap; // Whether the pixmap needs to be re-rendered

    // To have wrapping work, the resize event will be forwarded to the loaded
    // document
    void resizeEvent(QResizeEvent *evt);

    int m_sliderValue; // The slider value - gets updated by verticalSliderValueChanged

    QFont m_monospaceFont;
    int m_characterWidthPixels;
    Document *m_document;
    std::unique_ptr<ScrollBar>    m_verticalScrollBar;

private slots:
    void verticalSliderValueChanged( int value );

signals:
    void documentSizeChanged( const QSizeF& newSize, const qreal lineHeight );
};

#endif // CUSTOMCODEEDIT_H
