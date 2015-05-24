#include "vmainwindow.h"
#include "ui_vmainwindow.h"
#include <UI/CodeTextEdit/Document.h>
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
  //m_tabsBar->insertTab("test tab", false);
  //m_tabsBar->insertTab("another tab", false);
  ui->codeTextEditArea->addWidget(m_tabsBar.get());

  // Create the code editor control
  m_customCodeEdit = std::make_unique<CodeTextEdit>(this);
  ui->codeTextEditArea->addWidget( m_customCodeEdit.get() );


  // Load the sample data
  int id = m_tabsBar->insertTab("SimpleFile.cpp", false);
  auto it = m_tabDocumentMap.insert(std::make_pair(id, std::make_unique<Document>(*m_customCodeEdit)));
  it.first->second->loadFromFile( "../vectis/TestData/SimpleFile.cpp" );
  it.first->second->applySyntaxHighlight( CPP );
  m_customCodeEdit->loadDocument( it.first->second.get() );

  // Load some other sample data
  int id2 = m_tabsBar->insertTab("BasicBlock.cpp", false);
  auto it2 = m_tabDocumentMap.insert(std::make_pair(id2, std::make_unique<Document>(*m_customCodeEdit)));
  it2.first->second->loadFromFile( "../vectis/TestData/BasicBlock.cpp" );
  it2.first->second->applySyntaxHighlight( CPP );
  m_customCodeEdit->loadDocument( it2.first->second.get() );


  // NOTICE: link connections AFTER all initial documents have been created
  // Link the "changed selected tab" and "tab was requested to close" signals to slots
  connect(m_tabsBar.get(), SIGNAL(selectedTabHasChanged(int)),
          this, SLOT(selectedTabChangedSlot(int)));
  connect(m_tabsBar.get(), SIGNAL(tabWasRequestedToClose(int)),
          this, SLOT(tabWasRequestedToCloseSlot(int)));

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

  m_customCodeEdit->loadDocument( m_tabDocumentMap[newId].get() );



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
  qDebug() << "Tab was requested to close: " << tabId;
  m_tabsBar->deleteTab(tabId);
  auto it = m_tabDocumentMap.find(tabId);
  m_tabDocumentMap.erase(it);
  if(m_tabDocumentMap.empty())
    m_customCodeEdit->unloadDocument();
  // if (tabId == currentlySelected) {
  //  m_customCodeEdit->setText("");
  //      currentlySelected = m_tabsBar->getSelectedTabId();
  //    }
  //    contents.erase(tabId);
}

VMainWindow::~VMainWindow() {
  delete ui;
}
