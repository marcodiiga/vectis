#ifndef VMAINWINDOW_H
#define VMAINWINDOW_H

#include <UI/CodeTextEdit/CodeTextEdit.h>
#include <UI/ScrollBar/ScrollBar.h>
#include <UI/TabsBar/TabsBar.h>
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

private slots:
    void selectedTabChangedSlot(int oldId, int newId);
    void tabWasRequestedToCloseSlot(int tabId);

private:

    Ui::VMainWindow *ui;

    // Window controls
    std::unique_ptr<CodeTextEdit> m_customCodeEdit;
    std::unique_ptr<TabsBar>      m_tabsBar;

    // A map associating Documents to tab ids (which are also document ids)
    std::map<int, std::unique_ptr<Document>> m_tabDocumentMap;
    // A map that stores the vertical scrollbar position for each document (to remember it)
    std::map<int /* Document/Tab id */, int> m_tabDocumentVScrollPos;

};

#endif // VMAINWINDOW_H
