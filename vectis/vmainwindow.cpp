#include "vmainwindow.h"
#include "ui_vmainwindow.h"
#include <QPainter>
#include <QScrollArea>
#include <QLayout>

#include <QDebug>

#include <QFontDatabase>


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
    m_tabsBar->insertTab("test tab", false);
    m_tabsBar->insertTab("another tab", false);
    ui->codeTextEditArea->addWidget(m_tabsBar.get());
    // Link the "changed selected tab" and "tab was requested to close" signals to slots
    connect(m_tabsBar.get(), SIGNAL(selectedTabHasChanged(int)),
            this, SLOT(selectedTabChangedSlot(int)));
    connect(m_tabsBar.get(), SIGNAL(tabWasRequestedToClose(int)),
            this, SLOT(tabWasRequestedToCloseSlot(int)));







    // Create the code editor control
    m_customCodeEdit = std::make_unique<CodeTextEdit>(this);
    ui->codeTextEditArea->addWidget(m_customCodeEdit.get());
    // DEBUG: load a cpp document and try to render everything
    m_customCodeEdit->loadFile("TestData/BasicBlock.cpp");







//    // Consolas is installed by default on every Windows system, but not Linux. On Linux the
//    // preferred one is Monospace. Anyway Qt's matching engine will try to find either or a
//    // replacement monospace font
//#ifdef _WIN32
//    QFont monospace("Consolas");
//#else
//    QFont monospace("Monospace");
//#endif
//    monospace.setStyleHint(QFont::Monospace);
//    m_customCodeEdit->setFont(monospace);



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
//    m_verticalScrollBar = std::make_unique<ScrollBar>(m_customCodeEdit.get());
//    m_customCodeEdit->setVerticalScrollBar(m_verticalScrollBar.get());
//    m_customCodeEdit->verticalScrollBar()->setStyleSheet(QString("\
//                                                                  QScrollBar:vertical {\
//                                                                    width:15px;\
//                                                                  }"));

//    // DEBUG CODE
//    ttf.ptr = m_tabsBar.get();
//    m_customCodeEdit->installEventFilter( &ttf );


    //vertScrollBar->setAttribute( Qt::WA_TranslucentBackground );
}

#include <sstream>
std::map<int, QString> contents;
int currentlySelected = -1;

bool tabTestFilter::eventFilter ( QObject *obj, QEvent *event ) { // DEBUG EVENT FILTER
    if ( event->type() == QEvent::KeyPress ) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if( keyEvent->key() == Qt::Key_T && keyEvent->modifiers() == Qt::CTRL ) {
          static int number = 0;
          std::stringstream ss;
          ss << "Document" << ++number;
          std::string str = ss.str();
          currentlySelected = ptr->insertTab(QString(str.c_str()));
        } else if( keyEvent->key() == Qt::Key_F4 && keyEvent->modifiers() == Qt::CTRL ) {
          ptr->deleteTab(ptr->getSelectedTabId());
        }
    }
    // Other events: standard event processing
    return QObject::eventFilter( obj, event );
}

void VMainWindow::paintEvent(QPaintEvent *)
{

}


void VMainWindow::selectedTabChangedSlot(int newId) {
    qDebug() << "Selected tab has changed to " << newId;
    // Save everything to buffer
//    contents[currentlySelected] = m_customCodeEdit->toPlainText();
//    currentlySelected = newId;
//    auto it = contents.find(newId);
//    if (it != contents.end())
//      m_customCodeEdit->setText(it->second);
//    else
//      m_customCodeEdit->setText("");
}

void VMainWindow::tabWasRequestedToCloseSlot(int tabId) {
//    qDebug() << "Tab was requested to close: " << tabId;
//    m_tabsBar->deleteTab(tabId);
//    if (tabId == currentlySelected) {
//      m_customCodeEdit->setText("");
//      currentlySelected = m_tabsBar->getSelectedTabId();
//    }
//    contents.erase(tabId);
}

VMainWindow::~VMainWindow() {
    delete ui;
}
