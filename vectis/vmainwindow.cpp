#include "vmainwindow.h"
#include "ui_vmainwindow.h"
#include <QMimeData>
#include <QFileInfo>
#include <QPainter>
#include <QScrollArea>
#include <QLayout>
#include <memory>
#include <utility>

// Highlighters
#include <UI/Highlighters/CPPHighlighter.h>

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
  m_tabsBar->setFixedHeight(30);
  //m_tabsBar->insertTab("test tab", false);
  //m_tabsBar->insertTab("another tab", false);
  ui->codeTextEditArea->addWidget(m_tabsBar.get());

  // Create the code editor control
  m_customCodeEdit = std::make_unique<CodeTextEdit>(this);
  ui->codeTextEditArea->addWidget( m_customCodeEdit.get() );


  // Load the sample data
  loadDocumentFromFile("/home/alex/vectis/vectis/TestData/BasicBlock.cpp", false);

  // Load some other sample data
  //loadDocumentFromFile("../vectis/TestData/BasicBlock.cpp", false);
  //loadDocumentFromFile("../vectis/TestData/SimpleFile.cpp", false);


  // NOTICE: link connections AFTER all initial documents have been created
  // Link the "changed selected tab" and "tab was requested to close" signals to slots
//  connect(m_tabsBar.get(), SIGNAL(selectedTabHasChanged(int, int)),
//          this, SLOT(selectedTabChangedSlot(int, int)));
//  connect(m_tabsBar.get(), SIGNAL(tabWasRequestedToClose(int)),
//          this, SLOT(tabWasRequestedToCloseSlot(int)));

  // Mark window as accepting drag'n'drops
  setAcceptDrops(true);

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

void VMainWindow::paintEvent (QPaintEvent *) {
}


// Starts and continues a drag and drop event only if it has uri form
void VMainWindow::dragEnterEvent (QDragEnterEvent *event) {
  if (event->mimeData()->hasFormat("text/uri-list"))
    event->acceptProposedAction();
}

// Finalize a drop event of supported type
void VMainWindow::dropEvent (QDropEvent *event) {
  auto list = event->mimeData()->urls();

  for(auto& url : list)
    loadDocumentFromFile(url.toLocalFile(), true);

  event->acceptProposedAction();
}

// Get an instance of a supported and appropriate syntax highlighter for an extension
// or return nullptr if none could be found
std::unique_ptr<QSyntaxHighlighter> VMainWindow::getSyntaxHighlighterFromExtension(QString extension) {
  if (extension == "cpp" || extension == "c" || extension == "h" ||
      extension == "cxx" || extension == "hpp")
  {
    return std::make_unique<CPPHighlighter>();
  }
  return std::unique_ptr<QSyntaxHighlighter>();
}

void VMainWindow::loadDocumentFromFile (QString path, bool animation) {

  QFileInfo fileInfo(path); // Strip filename from path
  QString filename(fileInfo.fileName());

  // Create document, tab and apply proper lexers

  QFile file(path);
  if (!file.open(QFile::ReadWrite | QFile::Text))
      throw std::runtime_error("Could not open file");

  // Create a new tab and its respective tab id
  int id = m_tabsBar->insertTab(filename, animation);
  // Store it into the document map and create a new QTextDocument
  auto it = m_tabDocumentMap.emplace(id, std::make_unique<QTextDocument>());
  auto& document = it.first->second;
  // Apply a plain text document layout
  QPlainTextDocumentLayout *layout = new QPlainTextDocumentLayout(document.get());
  document->setDocumentLayout(layout);

  document->setDefaultFont(m_customCodeEdit->m_monospaceFont);

  document->setPlainText( file.readAll() ); // Load the document with the file text

  // Try to detect a suitable syntax highlighting scheme from the file extension
  QString extension( fileInfo.completeSuffix() );
  if (!extension.isEmpty()) {
    auto syntaxHighlighter = getSyntaxHighlighterFromExtension( extension );
    if (syntaxHighlighter) {
      syntaxHighlighter->setDocument ( document.get() );
      m_tabDocumentSyntaxHighlighter.emplace ( id, std::move(syntaxHighlighter) );
    }
  }
  // Finally load the newly created document in the viewport
  m_customCodeEdit->setDocument( document.get() );
}

//void VMainWindow::selectedTabChangedSlot (int oldId, int newId) {
//  // qDebug() << "Selected tab has changed from " << oldId << " to " << newId;

//  if (m_tabDocumentMap.find(newId) == m_tabDocumentMap.end())
//    return; // This tab hasn't an associated document. Do nothing.

//  // Save current vertical scrollbar position (there is always a vscrollbar) before switching document
//  if (oldId != -1)
//    m_tabDocumentVScrollPos[oldId] = m_customCodeEdit->m_verticalScrollBar->value();

//  // Restore (if any) vertical scrollbar position
//  auto it = m_tabDocumentVScrollPos.find(newId);
//  int vScrollbarPos = 0;
//  if (it != m_tabDocumentVScrollPos.end())
//    vScrollbarPos = it->second;

//  // Finally load the new requested document
//  m_customCodeEdit->loadDocument( m_tabDocumentMap[newId].get(), vScrollbarPos );



//  // Save everything to buffer
//  //    contents[currentlySelected] = m_customCodeEdit->toPlainText();
//  //    currentlySelected = newId;
//  //    auto it = contents.find(newId);
//  //    if (it != contents.end())
//  //      m_customCodeEdit->setText(it->second);
//  //    else
//  //      m_customCodeEdit->setText("");
//}

//void VMainWindow::tabWasRequestedToCloseSlot(int tabId) {
//  // qDebug() << "Tab was requested to close: " << tabId;

//  m_tabsBar->deleteTab(tabId); // Start tabs bar deletion process and new candidate selection process

//  {
//    // Delete document and tab id
//    auto it = m_tabDocumentMap.find(tabId);
//    m_tabDocumentMap.erase(it);

//    // Also delete the VScrollBar position history (if any)
//    auto itv = m_tabDocumentVScrollPos.find(tabId);
//    if (itv != m_tabDocumentVScrollPos.end())
//      m_tabDocumentVScrollPos.erase(itv);
//  }

//  // If we don't have any other document loaded, unload the viewport completely (but do not halt the
//  // rendering thread)
//  if(m_tabDocumentMap.empty())
//    m_customCodeEdit->unloadDocument();
//}

VMainWindow::~VMainWindow() {
  delete ui;
}
