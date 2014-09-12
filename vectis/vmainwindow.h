#ifndef VMAINWINDOW_H
#define VMAINWINDOW_H

#include <QDialog>
#include <QPixmap>
#include "customscrollbar.h"

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
};

#endif // VMAINWINDOW_H
