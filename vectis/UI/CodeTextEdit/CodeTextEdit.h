#ifndef CUSTOMCODEEDIT_H
#define CUSTOMCODEEDIT_H
#include <UI/CodeTextEdit/Document.h>
#include <UI/ScrollBar/ScrollBar.h>
#include <QPlainTextEdit>
#include <QDateTime>
#include <QTimer>

class MiniMap;

// The code and text edit control (everything gets rendered to it)
class CodeTextEdit : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit CodeTextEdit(QWidget *parent = 0);

    QFont getMonospaceFont() const;
    void renderDocument(QPixmap& map) const;
    QSizeF getDocumentDimensions() const;    
    void renderBlock(QPainter &painter, const QTextBlock &block) const;
    void setDocument(QTextDocument *document);

    float getVScrollbarPos() const;
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);
private:
    QFont m_monospaceFont;
    MiniMap *m_minimap = nullptr;
    void regenerateMiniMap();
    bool eventFilter(QObject *target, QEvent *event);
    QDateTime m_last_document_modification;
    QTimer m_regenerate_minimap_delay;
};

#endif // CUSTOMCODEEDIT_H
