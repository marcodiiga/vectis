#ifndef CUSTOMCODEEDIT_H
#define CUSTOMCODEEDIT_H
#include <UI/CodeTextEdit/Document.h>
#include <UI/ScrollBar/ScrollBar.h>
#include <QAbstractScrollArea>
#include <QtConcurrent>
#include <memory>
#include <vector>

class CodeTextEdit;

struct ResizeParameters {
  ResizeParameters(int ww, int bw, int bh) : wrapWidth(ww),
    bufferWidth(bw), bufferHeight(bh) {}
  ResizeParameters() = default;
  int wrapWidth;
  int bufferWidth;
  int bufferHeight;
};

class RenderingThread : public QThread {
  Q_OBJECT
public:
  RenderingThread(CodeTextEdit& p) : m_cte(p) {}
private:
  void run();
  bool getFrontElement();
  CodeTextEdit& m_cte;
  ResizeParameters m_currentElement;
signals:
  void renderingReady();
  void documentSizeChangedFromThread( const QSizeF& newSize, const qreal lineHeight );
};


// The code and text edit control (everything gets rendered to it)
class CodeTextEdit : public QAbstractScrollArea {
    Q_OBJECT
public:
    explicit CodeTextEdit(QWidget *parent = 0);

    // Load a document into the viewport
    void loadDocument(Document *doc, int VScrollbarPos = 0);
    void unloadDocument();
    int getViewportWidth() const;
    int getCharacterWidthPixels() const;

private:
    friend class RenderingThread;
    friend class VMainWindow; // Needs access to manipulate document and scrollbar positions

    void paintEvent(QPaintEvent *);
    void renderDocumentOnPixmap();
    std::unique_ptr<QImage> m_documentPixmap; // The rendered document
    std::unique_ptr<QImage> m_backgroundBufferPixmap; // The background buffer
    QMutex m_documentMutex;
    std::unique_ptr<RenderingThread> m_renderingThread;
    std::vector<ResizeParameters> m_documentUpdateMessages;
    QMutex m_messageQueueMutex;

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
    void documentSizeChangedFromThread( const QSizeF& newSize, const qreal lineHeight );
signals:
    void documentSizeChanged( const QSizeF& newSize, const qreal lineHeight, const int verticalSliderPos );
};

#endif // CUSTOMCODEEDIT_H
