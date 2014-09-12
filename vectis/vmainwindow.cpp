#include "vmainwindow.h"
#include "ui_vmainwindow.h"
#include <QPainter>
#include <QScrollArea>


VMainWindow::VMainWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VMainWindow)
{
    // Set the window maximize / minimize / exit buttons
    Qt::WindowFlags flags = Qt::Window   |
            Qt::WindowMaximizeButtonHint |
            Qt::WindowMinimizeButtonHint |
            Qt::WindowCloseButtonHint;
    this->setWindowFlags(flags);
    // Set the background color for window. Notice: style sheets are
    // more portable than modifying the palette directly
    /*this->setStyleSheet("QDialog { background-color: #272822; }"); */

    ui->setupUi(this);

    // Set background color for the edit code control
    ui->plainTextEdit->setStyleSheet("QPlainTextEdit {                                  \
                                     background-color: #272822;                         \
                                     color: white;                                      \
                                     border: 0px;                                       \
                                     font-family: Consolas, monospace;                  \
                                     font-size: 15px;                                   \
                                     }");
    ui->plainTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    //ui->plainTextEdit->setVerticalScrollBar(new CustomScrollBar(ui->plainTextEdit));
    //QScrollBar *vertScrollBar = ui->plainTextEdit->verticalScrollBar();

    /*vertScrollBar->setStyleSheet("\
    QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { \
                             background: red; \
                             }\
                         ");*/
    /*ui->plainTextEdit->verticalScrollBar()->setStyleSheet(
        QString("\
                QScrollBar:vertical {\
                    background: rgba(34,34,34);\
                    width:13px;\
                }\
                QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {\
                    background: none;\
                }\
                QScrollBar:left-arrow:vertical, QScrollBar::right-arrow:vertical {\
                    height: 1px;\
                    width: 1px;\
                }\
                QScrollBar::handle:vertical {\
                    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,\
                    stop:0 rgb(55,55,55), stop: 0.5 rgb(71,71,71), stop:1 rgb(47,48,41));\
                    border-radius: 5px;\
                    width: 4px;\
                }\
                "));*/
    ui->plainTextEdit->setVerticalScrollBar(new CustomScrollBar(ui->plainTextEdit));

    //vertScrollBar->setAttribute( Qt::WA_TranslucentBackground );
}

void VMainWindow::paintEvent(QPaintEvent *)
{

}

VMainWindow::~VMainWindow() {
    delete ui;
}
