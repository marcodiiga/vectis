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

class VMainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit VMainWindow(QWidget *parent = 0);
    ~VMainWindow();

    void paintEvent(QPaintEvent *);
    QPixmap editShot;
    bool shotSet;

private slots:

private:
    Ui::VMainWindow *ui;
    std::unique_ptr<CodeTextEdit> m_customCodeEdit;
    std::unique_ptr<ScrollBar>    m_verticalScrollBar;
    std::unique_ptr<TabsBar>      m_tabsBar;
};

#endif // VMAINWINDOW_H
