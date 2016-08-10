#ifndef VMAINWINDOW_H
#define VMAINWINDOW_H

#include <UI/CodeTextEdit/CodeTextEdit.h>
#include <UI/ScrollBar/ScrollBar.h>
#include <UI/TabsBar/TabsBar.h>
#include <QSyntaxHighlighter>
#include <QDialog>
#include <QPixmap>
#include <memory>

namespace Ui {
class VMainWindow;
}


class tabTestFilter : public QObject { // DEBUG
    Q_OBJECT
public:
    tabTestFilter () {}
    TabsBar *ptr;
    bool eventFilter ( QObject *obj, QEvent *event );
}; // DEBUG

class VMainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit VMainWindow(QWidget *parent = 0);
    ~VMainWindow();

    void paintEvent(QPaintEvent *);
    QPixmap editShot;
    bool shotSet;
    tabTestFilter ttf; // DEBUG

    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

    // Loads a new document from a file
    void loadDocumentFromFile(QString path, bool animation = false);

private slots:
   // void selectedTabChangedSlot(int oldId, int newId);
   // void tabWasRequestedToCloseSlot(int tabId);

private:

    Ui::VMainWindow *ui;

    // Window controls
    std::unique_ptr<CodeTextEdit> m_customCodeEdit;
    std::unique_ptr<TabsBar>      m_tabsBar;

    // A map associating QTextDocuments to tab ids (which are also document ids)
    std::map <int, std::unique_ptr<QTextDocument> > m_tabDocumentMap;
    // A map that stores the vertical scrollbar position for each document (to remember it)
    std::map <int /* Document/Tab id */, int> m_tabDocumentVScrollPos;
    // A map that stores the syntax highlighter that a document might have
    std::map <int /* Document/Tab id */, std::unique_ptr<QSyntaxHighlighter> > m_tabDocumentSyntaxHighlighter;

    std::unique_ptr<QSyntaxHighlighter> getSyntaxHighlighterFromExtension(QString extension);
};

#endif // VMAINWINDOW_H
