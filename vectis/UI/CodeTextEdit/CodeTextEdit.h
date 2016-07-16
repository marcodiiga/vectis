#ifndef CUSTOMCODEEDIT_H
#define CUSTOMCODEEDIT_H
#include <UI/CodeTextEdit/Document.h>
#include <UI/ScrollBar/ScrollBar.h>
#include <QAbstractScrollArea>
#include <QtConcurrent>
#include <memory>
#include <vector>

class CodeTextEdit;

// A message structure that gets passed to the rendering thread and put in a queue.
// Contains information needed to resize/rewrap the document displayed
struct ResizeParameters {
  ResizeParameters(int ww, int bw, int bh) : wrapWidth(ww),
    bufferWidth(bw), bufferHeight(bh) {}
  ResizeParameters() = default;
  int wrapWidth;
  int bufferWidth;
  int bufferHeight;
};

// Main rendering thread (should never be halted while CodeTextEdit is active)
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

// This class implements a neverending interpolation for the caret blinking.
// It would have probably made more sense to implement this as an inner class but the
// MOC can't handle it.
class CaretBlinkingInterpolation : public QVariantAnimation {
    Q_OBJECT

    CaretBlinkingInterpolation (CodeTextEdit& parent);

    friend class CodeTextEdit;
    void updateCurrentValue(const QVariant &value) override;

    CodeTextEdit& m_parent;

private slots:
    void finished();
};


// The code and text edit control (everything gets rendered to it)
class CodeTextEdit : public QAbstractScrollArea {
    Q_OBJECT
public:
    explicit CodeTextEdit(QWidget *parent = 0);
    ~CodeTextEdit();

    // Load a document into the viewport
    void loadDocument(Document *doc, int VScrollbarPos = 0);
    void unloadDocument();
    int getViewportWidth() const;
    int getCharacterWidthPixels() const;

    // Left-top padding used when rendering documents
    static constexpr const int BITMAP_OFFSET_X = 5;
    static constexpr const int BITMAP_OFFSET_Y = 20;

    void mousePressEvent ( QMouseEvent* evt );
    void keyPressEvent ( QKeyEvent *event );

private:
    friend class RenderingThread;
    friend class CaretBlinkingInterpolation;
    friend class VMainWindow; // Needs access to manipulate document and scrollbar positions

    void paintEvent(QPaintEvent *);
    void renderDocumentOnPixmap();
    std::unique_ptr<QImage> m_documentPixmap; // The rendered document
    std::unique_ptr<QImage> m_backgroundBufferPixmap; // The background buffer
    QMutex m_documentMutex;
    std::unique_ptr<RenderingThread> m_renderingThread;
    bool m_termination = false; // Signal the thread to terminate
    std::vector<ResizeParameters> m_documentUpdateMessages;
    QMutex m_messageQueueMutex;
    QWaitCondition m_messageQueueCV;

    // To have wrapping work, the resize event will be forwarded to the loaded
    // document
    void resizeEvent(QResizeEvent *evt);
    bool m_firstResizeHasHappened = false;

    int m_sliderValue = 0; // The slider value - gets updated by verticalSliderValueChanged.
                           // It is the Y offset into the document expressed in the line number
                           // we're currently at

    QFont m_monospaceFont;
    int m_characterWidthPixels = 0;
    int m_characterDescentPixels = 0;
    Document *m_document = nullptr;

    int m_caretAlpha = 0; // Alpha value of the caret (for animation/blinking purposes)
    CaretBlinkingInterpolation m_caretAlphaInterpolator;

    std::unique_ptr<ScrollBar> m_verticalScrollBar;

private slots:
    void verticalSliderValueChanged( int value );
    void documentSizeChangedFromThread( const QSizeF& newSize, const qreal lineHeight );
signals:
    void documentSizeChanged( const QSizeF& newSize, const qreal lineHeight, const int verticalSliderPos );
};

#endif // CUSTOMCODEEDIT_H
