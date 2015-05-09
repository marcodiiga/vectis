#include "vmainwindow.h"
#include "ui_vmainwindow.h"
#include <QPainter>
#include <QScrollArea>
#include <QLayout>

#include <QDebug>




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
    this->setStyleSheet("QDialog { background-color: #272822; }");

    ui->setupUi(this);



    // Create the TabsBar
    m_tabsBar = std::make_unique<TabsBar>(this);
    //TabsBar ea;
    m_tabsBar->setFixedHeight(35);
    //m_tabsBar->addTab("test tab");
    //m_tabsBar->addTab("another tab");
    ui->codeTextEditArea->addWidget(m_tabsBar.get());


    // Create the code editor control
    m_customCodeEdit = std::make_unique<CodeTextEdit>(this);
    ui->codeTextEditArea->addWidget(m_customCodeEdit.get());

    qWarning() << "TODO: move stylesheet and font setting INSIDE the control, not here!";
    // Set background color for the edit code control
    m_customCodeEdit->setStyleSheet("QTextEdit {                                        \
                                     background-color: #272822;                         \
                                     color: white;                                      \
                                     border: 0px;                                       \
                                     font-size: 12px;                                   \
                                     }");
    // Consolas is installed by default on every Windows system, but not linux.
    // Qt's matching engine will try to find Consolas or a replacement monospace font
    // font-family: Consolas, monospace;
    QFont font("Consolas");
    font.setStyleHint(QFont::Monospace);
    m_customCodeEdit->setFont(font);
    m_customCodeEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
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

    // Create the vertical scrollbar and assign it to the code editor
    m_verticalScrollBar = std::make_unique<ScrollBar>(m_customCodeEdit.get());
    m_customCodeEdit->setVerticalScrollBar(m_verticalScrollBar.get());
    m_customCodeEdit->verticalScrollBar()->setStyleSheet(QString("\
                                                                  QScrollBar:vertical {\
                                                                    width:15px;\
                                                                  }"));

    // DEBUG CODE
    ttf.ptr = m_tabsBar.get();
    m_customCodeEdit->installEventFilter( &ttf );


    //vertScrollBar->setAttribute( Qt::WA_TranslucentBackground );
}

bool tabTestFilter::eventFilter ( QObject *obj, QEvent *event ) { // DEBUG EVENT FILTER
    if ( event->type() == QEvent::KeyPress ) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if( keyEvent->key() == Qt::Key_Plus ) {
            ptr->insertTab("hello");
        } else if( keyEvent->key() == Qt::Key_Minus ) {
          ptr->deleteTab(ptr->getSelectedTabId());
        }
    }
    // Other events: standard event processing
    return QObject::eventFilter( obj, event );
}

void VMainWindow::paintEvent(QPaintEvent *)
{

}

VMainWindow::~VMainWindow() {
    delete ui;
}
