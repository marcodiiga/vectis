#include "vmainwindow.h"
#include "ui_vmainwindow.h"
#include <UI/Utils.h>
#include <QMimeData>
#include <QFileInfo>
#include <QPainter>
#include <QScrollArea>
#include <QLayout>
#include <QMessageBox>
#include <QScrollBar>
#include <memory>
#include <utility>

#include <QDebug>
#include <QLabel>
#include <QPushButton>

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

  this->setAcceptDrops(true);  

  ui->setupUi(this);

  this->resize(1100, 600);

  // Create the TabsBar
  m_tabsBar = new TabsBar(this);
  //TabsBar ea;
  m_tabsBar->setFixedHeight(30);
  //m_tabsBar->insertTab("test tab", false);
  //m_tabsBar->insertTab("another tab", false);
  ui->codeTextEditArea->addWidget(m_tabsBar);

  // Create the code editor control
  m_customCodeEdit = new CodeTextEdit(this);
  ui->codeTextEditArea->addWidget( m_customCodeEdit );


  // Load the sample data
  loadDocumentFromFile("../vectis/TestData/BasicBlock.cpp", false);

  // Load some other sample data
  //loadDocumentFromFile("../vectis/TestData/BasicBlock.cpp", false);
  //loadDocumentFromFile("../vectis/TestData/SimpleFile.cpp", false);


  // NOTICE: link connections AFTER all initial documents have been created
  // Link the "changed selected tab" and "tab was requested to close" signals to slots
  connect(m_tabsBar, SIGNAL(selectedTabHasChanged(int, int)),
          this, SLOT(selectedTabChangedSlot(int, int)));
  connect(m_tabsBar, SIGNAL(tabWasRequestedToClose(int)),
          this, SLOT(tabWasRequestedToCloseSlot(int)));




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
  auto it = m_tabDocumentMap.emplace(id, new QTextDocument(m_customCodeEdit));
  QTextDocument *document = it.first->second;

  // Apply a plain text document layout
  QPlainTextDocumentLayout *layout = new QPlainTextDocumentLayout(document);
  document->setDocumentLayout(layout);

  document->setDefaultFont(m_customCodeEdit->getMonospaceFont());

  document->setPlainText( file.readAll() ); // Load the document with the file text

  // Try to detect a suitable syntax highlighting scheme from the file extension
  QString extension( fileInfo.completeSuffix() );
  if (!extension.isEmpty()) {
    auto syntaxHighlighter = getSyntaxHighlighterFromExtension( extension );
    if (syntaxHighlighter != nullptr) {
      document->setProperty("syntax_highlighter", extension);
      syntaxHighlighter->setDocument ( document );
      syntaxHighlighter->setParent ( document );
      m_tabDocumentSyntaxHighlighter.emplace ( id, syntaxHighlighter );
    }
  }
  // Finally load the newly created document in the viewport
  m_customCodeEdit->setDocument( document );
}

void VMainWindow::selectedTabChangedSlot (int oldId, int newId) {
  // qDebug() << "Selected tab has changed from " << oldId << " to " << newId;

  if (m_tabDocumentMap.find(newId) == m_tabDocumentMap.end())
    return; // This tab hasn't an associated document. Do nothing.

  // Save current vertical scrollbar position (there is always a vscrollbar) before switching document
  if (oldId != -1)
    m_tabDocumentVScrollPos[oldId] = m_customCodeEdit->verticalScrollBar()->value();

  // Restore (if any) vertical scrollbar position
  auto it = m_tabDocumentVScrollPos.find(newId);
  int vScrollbarPos = 0;
  if (it != m_tabDocumentVScrollPos.end())
    vScrollbarPos = it->second;

  // Finally load the new requested document
  m_customCodeEdit->setDocument( m_tabDocumentMap[newId], vScrollbarPos );



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
  // qDebug() << "Tab was requested to close: " << tabId;

  m_tabsBar->deleteTab(tabId); // Start tabs bar deletion process and new candidate selection process

  {
    // Delete document and tab id
    auto it = m_tabDocumentMap.find(tabId);    
    m_tabDocumentMap.erase(it);

    // Also delete the VScrollBar position history (if any)
    auto itv = m_tabDocumentVScrollPos.find(tabId);
    if (itv != m_tabDocumentVScrollPos.end())
      m_tabDocumentVScrollPos.erase(itv);
  }

  // If we don't have any other document loaded, unload the viewport completely (but do not halt the
  // rendering thread)
  if(m_tabDocumentMap.empty())
    m_customCodeEdit->unloadDocument();
}

VMainWindow::~VMainWindow() {
  delete ui;
}

void VMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
  // Accept any file
  QUrl url(event->mimeData()->text());
  if (url.isValid() && url.scheme().toLower() == "file")
    event->accept();
}

void VMainWindow::dragMoveEvent(QDragMoveEvent *event)
{
  // Accept any file
  QUrl url(event->mimeData()->text());
  if (url.isValid() && url.scheme().toLower() == "file")
    event->acceptProposedAction();
}

void VMainWindow::dropEvent(QDropEvent *event)
{
  // Accept any drop if they exist
  const QMimeData* mime_data = event->mimeData();

  // Check the mime type, allowed types: a file or a list of files
  if (mime_data->hasUrls())
  {
    QStringList path_list;
    QList<QUrl> url_list = mime_data->urls();

    // Get local paths and check for existence
    for (int i = 0; i < url_list.size(); ++i) {
      QString filepath = url_list.at(i).toLocalFile();
      if (!QFile::exists(filepath)) {
        QMessageBox::warning(this, "File not found", "Cannot read or access file:\n\n'"
                             + filepath + "'\n\nAborting");
        return;
      }
      path_list.append(filepath);
    }

    for (auto& file : path_list)
      loadDocumentFromFile(file, true);
  }
}
