#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTreeView>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

int sign_for_dir = 0;
QDockWidget *dock;
int tab_num = 2;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->projectPath=QDir::currentPath();
    initMenu();
    initTabPage();

    this->resize(1000,800);
    dock = new QDockWidget(tr("Project Dir"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    createFileDirDock("", dock);

//    craeteFileDirDock(this->projectPath);

    sign_for_dir = 1;

    createStatusBar();

    this->currentEditor->setFontPointSize(12);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initTabPage(){
    tabPages=new QTabWidget();
    this->setCentralWidget(tabPages);
    tabPages->setTabsClosable(true);
    connect(tabPages,SIGNAL(tabCloseRequested(int)),this,SLOT(closeTabPage(int)));

    newEmptyMakefilePage("new1");
    this->map_for_save[0] = "new1";
    this->map_for_save_need[0] = 1;
}

void MainWindow::newEmptyMakefilePage(const QString str){
    CodeEditor *editor=new CodeEditor(this->projectPath);
    connect(editor,SIGNAL(sendMessage(QString)),this,SLOT(showMassage(QString)));
    tabPages->addTab(editor, str);
    tabPages->setCurrentWidget(editor);
    this->currentEditor = editor;
    int index = tabPages->currentIndex();
    this->map_for_save[index] = str;
    this->map_for_save_need[index] = 1;
}

bool MainWindow::newMakefilePage(const QString filePath,const QString filename){
    CodeEditor *editor=new CodeEditor(filePath);
    connect(editor,SIGNAL(sendMessage(QString)),this,SLOT(showMassage(QString)));
    QFile file(filePath);

    if (!file.open(QIODevice::ReadWrite|QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(filename), file.errorString()));
        return false;
    }
    QTextStream in(&file);
    editor->setText(in.readAll());
    tabPages->addTab(editor,filename);
//    tabPages->setFocusProxy(editor);
    tabPages->setCurrentWidget(editor);
    this->currentEditor = editor;
    int index = tabPages->currentIndex();
    this->map_for_save[index] = filePath;
    this->map_for_save_need[index] = 1;
    return true;
}

bool MainWindow::newPlainTextPage(const QString filePath,const QString filename){
    QFile file(filePath);

    if (!file.open(QIODevice::ReadWrite|QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(filename), file.errorString()));
        return false;
    }
    QTextStream in(&file);
    QTextEdit *neweditor = new QTextEdit();
    neweditor->setText(in.readAll());
    tabPages->addTab(neweditor,filename);
//    tabPages->setFocusProxy(neweditor);
    tabPages->setCurrentWidget(neweditor);
    int index = tabPages->currentIndex();
    this->map_for_save[index] = filePath;
    this->map_for_save_need[index] = 1;
    return true;
}

bool MainWindow::openFile(const QModelIndex &index){
    QString filepath=fsmodel->filePath(index);
    QString filename=fsmodel->fileName(index);
    qDebug() << filepath << endl;
    if(!canBeShow(index)){
        return false;
    }else if(filename == "makefile" || filename == "MakeFile" || filename == "Makefile"){
        return newMakefilePage(filepath, filename);
    }else{
        return newPlainTextPage(filepath, filename);
    }
    return true;
}

bool MainWindow::canBeShow(const QModelIndex &index ){

    if(fsmodel->isDir(index)){
        return false;
    }
    QSet<QString> canBeShowFileTypes;
    canBeShowFileTypes <<"h"<<"c"<<"cpp"<<"txt"<<"makefile"<<"Makefile"<<"MakeFile";
    QString filename=fsmodel->fileName(index);
    QStringList sList=filename.split('.');
    QString nameback=sList.back();

    if(!canBeShowFileTypes.contains(nameback)){
        qDebug()<<"Can not be shown."<<endl;
        return false;
    }else{
        qDebug()<<nameback<<endl;
        return true;
    }

}

void MainWindow::createFileDirDock(QString rootDir, QDockWidget*dock)
{
    if(dock->isHidden()){
        dock->setVisible(true);
    }
    QString sign = rootDir;
    if(rootDir == ""){
        rootDir = QDir::currentPath();
    }

    fsmodel = new QFileSystemModel();
    this->fsmodel->setRootPath(rootDir);
    QTreeView *tree = new QTreeView(dock);
    tree->setModel(fsmodel);

    if (!rootDir.isEmpty()) {
        const QModelIndex rootIndex = fsmodel->index(QDir::cleanPath(rootDir));
        if (rootIndex.isValid())
            tree->setRootIndex(rootIndex);
    }

    for(int i=1;i<fsmodel->columnCount();i++)
    {
        tree->hideColumn(i);
    }

    tree->setAnimated(false);
    tree->setIndentation(20);
    tree->setSortingEnabled(true);

    connect(tree,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(openFile(QModelIndex)));

    dock->setWidget(tree);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    list<string>::iterator temp_it = std::find(this->open_records_dir.begin(), this->open_records_dir.end(), rootDir.toStdString());
    if(temp_it != this->open_records_dir.end()){
        this->open_records_dir.erase(temp_it);
        this->open_records_dir.push_front(rootDir.toStdString());
    }
    else{
        if(this->open_records_dir.size() <= 10){
            this->open_records_dir.push_front(rootDir.toStdString());
        }
        else{
            this->open_records_dir.pop_back();
            this->open_records_dir.push_front(rootDir.toStdString());
        }
    }
    freshRecent();
}

void MainWindow::initMenu(){
    this->initFileMenu();
    this->initEditMenu();
    this->initSearchMenu();
    this->initViewMenu();
    this->initHelpMenu();
}

void MainWindow::initFileMenu(){
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QToolBar *fileToolBar = addToolBar(tr("File"));

    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(projectPath+"\\Image\\newfile.png"));
    QAction *newAct = new QAction(newIcon, tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(newAct);
    fileToolBar->addAction(newAct);

    const QIcon openIcon = QIcon::fromTheme("dir-open", QIcon(projectPath+"\\Image\\opendir.png"));
    QAction *openAct = new QAction(openIcon, tr("&Open..."), this);
    openAct->setStatusTip(tr("Open an existing directory"));
    connect(openAct, &QAction::triggered, this, &MainWindow::open);
    fileMenu->addAction(openAct);
    fileToolBar->addAction(openAct);

    const QIcon openfileIcon = QIcon::fromTheme("document-open", QIcon(projectPath+"\\Image\\openfile.png"));
    QAction *openfileAct = new QAction(openfileIcon, tr("&Open File"), this);
    openfileAct->setShortcuts(QKeySequence::Open);
    openfileAct->setStatusTip(tr("Open an existing file"));
    connect(openfileAct, &QAction::triggered, this, &MainWindow::openpathFile);
    fileMenu->addAction(openfileAct);
    fileToolBar->addAction(openfileAct);

    const QIcon savefileIcon = QIcon::fromTheme("document-save", QIcon(projectPath+"\\Image\\savefile.png"));
    QAction *savefileAct = new QAction(savefileIcon, tr("&Save File"), this);
    savefileAct->setShortcuts(QKeySequence::Save);
    savefileAct->setStatusTip(tr("Save current file"));
    connect(savefileAct, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(savefileAct);
    fileToolBar->addAction(savefileAct);

    const QIcon saveallfileIcon = QIcon::fromTheme("document-save", QIcon(projectPath+"\\Image\\saveall.png"));
    QAction *saveallfileAct = new QAction(saveallfileIcon, tr("&Save All Files"), this);
    //saveallfileAct->setShortcuts(QKeySequence::Save);
    saveallfileAct->setStatusTip(tr("Save All Files"));
    connect(saveallfileAct, &QAction::triggered, this, &MainWindow::saveAll);
    fileMenu->addAction(saveallfileAct);
    fileToolBar->addAction(saveallfileAct);

    const QIcon saveasfileIcon = QIcon::fromTheme("document-save", QIcon(projectPath+"\\Image\\saveAs.png"));
    QAction *saveasfileAct = new QAction(saveasfileIcon, tr("&Save As"), this);
    saveasfileAct->setShortcuts(QKeySequence::SaveAs);
    saveasfileAct->setStatusTip(tr("Save As"));
    connect(saveasfileAct, &QAction::triggered, this, &MainWindow::saveAs2);
    fileMenu->addAction(saveasfileAct);
    fileToolBar->addAction(saveasfileAct);

    const QIcon closeIcon = QIcon::fromTheme("document-close", QIcon(projectPath+"\\Image\\close.png"));
    QAction *closeAct = new QAction(closeIcon, tr("&Close File"), this);
    closeAct->setShortcuts(QKeySequence::Close);
    closeAct->setStatusTip(tr("Close Tab"));
    connect(closeAct, &QAction::triggered, this, &MainWindow::closeTab);
    fileMenu->addAction(closeAct);
    fileToolBar->addAction(closeAct);

    const QIcon closeAllIcon = QIcon::fromTheme("document-close-all", QIcon(projectPath+"\\Image\\closeall.png"));
    QAction *closeAllAct = new QAction(closeAllIcon, tr("&Close All Tab"), this);
    //closeAllAct->setShortcuts(QKeySequence::Close);
    closeAllAct->setStatusTip(tr("Close All Tab"));
    connect(closeAllAct, &QAction::triggered, this, &MainWindow::closeAll);
    fileMenu->addAction(closeAllAct);
    fileToolBar->addAction(closeAllAct);

    fileMenu->addSeparator();

    menu2 = fileMenu->addMenu("Open Recent Files");
    menu3 = fileMenu->addMenu("Open Recent Project");
    menu3->addSeparator();

    loadRecord();

    this->freshRecent();

    fileMenu->addSeparator();

    QAction *exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    connect(exitAct, &QAction::triggered, this, &MainWindow::exit);
    exitAct->setStatusTip(tr("Exit the application"));
    fileMenu->addAction(exitAct);
}

void MainWindow::initEditMenu(){
    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    QToolBar *editToolBar = addToolBar(tr("Edit"));

    const QIcon undoIcon = QIcon::fromTheme("undo", QIcon(projectPath+"\\Image\\undo.png"));
    QAction *undoAct = new QAction(undoIcon, tr("&Undo"), this);
    undoAct->setShortcuts(QKeySequence::Undo);
    undoAct->setStatusTip(tr("Undo what you have done"));
    connect(undoAct, &QAction::triggered, this, &MainWindow::on_action_Undo_triggered);
    editMenu->addAction(undoAct);
    editToolBar->addAction(undoAct);

    const QIcon redoIcon = QIcon::fromTheme("redo", QIcon(projectPath+"\\Image\\redo.png"));
    QAction *redoAct = new QAction(redoIcon, tr("&Redo"), this);
    redoAct->setShortcuts(QKeySequence::Redo);
    redoAct->setStatusTip(tr("Redo what you have done"));
    connect(redoAct, &QAction::triggered, this, &MainWindow::on_action_Redo_triggered);
    editMenu->addAction(redoAct);
    editToolBar->addAction(redoAct);

    editMenu->addSeparator();

    const QIcon cutIcon = QIcon::fromTheme("cut", QIcon(projectPath+"\\Image\\cut.png"));
    QAction *cutAct = new QAction(cutIcon, tr("&Cut"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut what you have written"));
    connect(cutAct, &QAction::triggered, this, &MainWindow::on_action_Cut_triggered);
    editMenu->addAction(cutAct);
    editToolBar->addAction(cutAct);

    const QIcon copyIcon = QIcon::fromTheme("copy", QIcon(projectPath+"\\Image\\copy.png"));
    QAction *copyAct = new QAction(copyIcon, tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy"));
    connect(copyAct, &QAction::triggered, this, &MainWindow::on_action_Copy_triggered);
    editMenu->addAction(copyAct);
    editToolBar->addAction(copyAct);

    const QIcon pasteIcon = QIcon::fromTheme("paste", QIcon(projectPath+"\\Image\\paste.png"));
    QAction *pasteAct = new QAction(pasteIcon, tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Undo);
    pasteAct->setStatusTip(tr("Paste"));
    connect(pasteAct, &QAction::triggered, this, &MainWindow::on_action_Paste_triggered);
    editMenu->addAction(pasteAct);
    editToolBar->addAction(pasteAct);
}

// 撤销操作
void MainWindow::on_action_Undo_triggered(){
    this->currentEditor->undo();
}

// 重做操作
void MainWindow::on_action_Redo_triggered(){
    this->currentEditor->redo();
}

// 剪切操作
void MainWindow::on_action_Cut_triggered(){
    this->currentEditor->cut();
}

// 复制操作
void MainWindow::on_action_Copy_triggered(){
    this->currentEditor->copy();
}

//粘贴操作
void MainWindow::on_action_Paste_triggered(){
    this->currentEditor->paste();
}

void MainWindow::initSearchMenu(){
    QMenu *searchMenu = menuBar()->addMenu(tr("&Search"));
    QToolBar *searchToolBar = addToolBar(tr("Search and Replace"));

    const QIcon searchIcon = QIcon::fromTheme("search", QIcon(projectPath+"\\Image\\search.png"));
    QAction *searchAct = new QAction(searchIcon, tr("&Search"), this);
    searchAct->setShortcuts(QKeySequence::Find);
    searchAct->setStatusTip(tr("Find"));
    connect(searchAct, &QAction::triggered, this, &MainWindow::on_action_Search_triggered);
    searchMenu->addAction(searchAct);
    searchToolBar->addAction(searchAct);

    const QIcon replaceIcon = QIcon::fromTheme("replace", QIcon(projectPath+"\\Image\\replace.png"));
    QAction *replaceAct = new QAction(replaceIcon, tr("&Replace"), this);
    replaceAct->setShortcuts(QKeySequence::Replace);
    replaceAct->setStatusTip(tr("Replace"));
    connect(replaceAct, &QAction::triggered, this, &MainWindow::on_action_Replace_triggered);
    searchMenu->addAction(replaceAct);
    searchToolBar->addAction(replaceAct);
}

//弹出搜索框
void MainWindow::on_action_Search_triggered(){
    findDlg = new QDialog(this);
    findDlg->setWindowTitle(tr("查找"));
    findlabel = new QLabel(findDlg);
    countlabel = new QLabel(findDlg);
    findlabel->setText("查找目标:");
    findlabel->adjustSize();

    findLineEdit = new QLineEdit(findDlg);

    QPushButton *findBtn = new QPushButton(tr("向上查找"), findDlg);
    QPushButton *countBtn = new QPushButton(tr("计数"), findDlg);
    QPushButton *findnextBtn = new QPushButton(tr("向下查找"), findDlg);

    QVBoxLayout *layout = new QVBoxLayout(findDlg);
    QHBoxLayout *layoutinner = new QHBoxLayout(findDlg);

    layoutinner->addWidget(findBtn);
    layoutinner->addWidget(countBtn);
    layoutinner->addWidget(findnextBtn);

    layout->addSpacing(10);
    layout->addWidget(findlabel);
    layout->addSpacing(10);
    layout->addWidget(findLineEdit);
    layout->addSpacing(10);
    layout->addLayout(layoutinner);
    layout->addSpacing(10);
    layout->addWidget(countlabel);

    connect(findBtn, SIGNAL(clicked()), this, SLOT(searchText()));
    connect(findnextBtn, SIGNAL(clicked()), this, SLOT(findnextText()));
    connect(countBtn, SIGNAL(clicked()), this, SLOT(countText()));

    findDlg->show();
}

void MainWindow::searchText(){
    QString str = findLineEdit->text();
    if(!this->currentEditor->find(str, QTextDocument::FindBackward)){
        QMessageBox::warning(this, tr("查找"), tr("未查询到相关内容:%1").arg(str));
    }
}

void MainWindow::findnextText(){
    QString str = findLineEdit->text();
    if(!this->currentEditor->find(str)){
        QMessageBox::warning(this, tr("查找"), tr("未查询到相关内容:%1").arg(str));
    }
}

void MainWindow::countText(){
    QString str = findLineEdit->text();
    QTextCursor tempcursor1 = this->currentEditor->textCursor();
    QTextCursor tempcursor2 = this->currentEditor->textCursor();
    tempcursor2.movePosition(QTextCursor::Start);
    this->currentEditor->setTextCursor(tempcursor2);
    int counter = 0;
    while(true){
        if(!this->currentEditor->find(str)){
            break;
        }
        counter++;
    }
    qDebug() << counter << endl;
    this->currentEditor->setTextCursor(tempcursor1);
    this->countlabel->setText(QString::number(counter));
}

//弹出替换框
void MainWindow::on_action_Replace_triggered(){
    replaceDlg = new QDialog(this);
    replaceDlg->setWindowTitle(tr("替换"));
    befindlabel = new QLabel(replaceDlg);
    befindlabel->setText("替换目标:");
    befindlabel->adjustSize();
    replacelabel = new QLabel(replaceDlg);
    replacelabel->setText("替换内容:");
    replacelabel->adjustSize();

    findEdit = new QLineEdit(replaceDlg);
    replaceEdit = new QLineEdit(replaceDlg);

    QPushButton *findBtn = new QPushButton(tr("查找"), replaceDlg);
    QPushButton *replaceBtn = new QPushButton(tr("替换"), replaceDlg);
    QPushButton *replaceallBtn = new QPushButton(tr("全部替换"), replaceDlg);

    QVBoxLayout *layout = new QVBoxLayout(replaceDlg);
    QHBoxLayout *layoutinnerUper1 = new QHBoxLayout(replaceDlg);
    QHBoxLayout *layoutinnerUper2 = new QHBoxLayout(replaceDlg);
    QHBoxLayout *layoutinnerUnder = new QHBoxLayout(replaceDlg);

    layoutinnerUper1->addWidget(befindlabel);
    layoutinnerUnder->addSpacing(10);
    layoutinnerUper1->addWidget(findEdit);

    layoutinnerUper2->addWidget(replacelabel);
    layoutinnerUnder->addSpacing(10);
    layoutinnerUper2->addWidget(replaceEdit);

    layoutinnerUnder->addWidget(findBtn);
    layoutinnerUnder->addSpacing(20);
    layoutinnerUnder->addWidget(replaceBtn);
    layoutinnerUnder->addSpacing(20);
    layoutinnerUnder->addWidget(replaceallBtn);

    layout->addSpacing(10);
    layout->addLayout(layoutinnerUper1);
    layout->addSpacing(10);
    layout->addLayout(layoutinnerUper2);
    layout->addSpacing(10);
    layout->addLayout(layoutinnerUnder);

    connect(findBtn, SIGNAL(clicked()), this, SLOT(find_replace_Text()));
    connect(replaceBtn, SIGNAL(clicked()), this, SLOT(replaceText()));
    connect(replaceallBtn, SIGNAL(clicked()), this, SLOT(replaceallText()));

    replaceDlg->show();
}

void MainWindow::find_replace_Text(){
    QTextCursor tempcursor = this->currentEditor->textCursor();
    tempcursor.movePosition(QTextCursor::Start);
    this->currentEditor->setTextCursor(tempcursor);

    QString str = findEdit->text();
    this->replacedtext = str;
    if(!this->currentEditor->find(str)){
        QMessageBox::warning(this, tr("查找"), tr("未查询到相关内容:%1").arg(str));
    }
    else{
        sign_for_replace = 1;
    }
}

void MainWindow::replaceText(){
    QString replace_str = replaceEdit->text();
    if(sign_for_replace == 1){
        this->currentEditor->textCursor().insertText(replace_str);
        sign_for_replace = 0;
    }
}

void MainWindow::replaceallText(){
    QString replace_str = replaceEdit->text();
    QTextCursor tempcursor1 = this->currentEditor->textCursor();
    QTextCursor tempcursor2 = this->currentEditor->textCursor();
    tempcursor2.movePosition(QTextCursor::Start);
    this->currentEditor->setTextCursor(tempcursor2);
    int counter = 0;
    while(true){
        if(!this->currentEditor->find(this->replacedtext)){
            break;
        }
        else{
            this->currentEditor->textCursor().insertText(replace_str);
        }
    }
    this->currentEditor->setTextCursor(tempcursor1);
}

void MainWindow::initViewMenu(){
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    QToolBar *viewToolBar = addToolBar(tr("View"));

    QMenu *zoom = viewMenu->addMenu(tr("Zoom"));

    QAction *magnifyAct = new QAction(tr("&magnify"), this);
    magnifyAct->setStatusTip(tr("magnify"));
    connect(magnifyAct, &QAction::triggered, this, &MainWindow::on_action_Magnify_triggered);
    zoom->addAction(magnifyAct);

    QAction *shrinkAct = new QAction(tr("&shrink"), this);
    shrinkAct->setStatusTip(tr("shrink"));
    connect(shrinkAct, &QAction::triggered, this, &MainWindow::on_action_Shrink_triggered);
    zoom->addAction(shrinkAct);

    QMenu *zoomRatio = viewMenu->addMenu(tr("ZoomRatio"));

    QAction *ratio1Act = new QAction(tr("&100%"), this);
    ratio1Act->setStatusTip(tr("ratio 100%"));
    connect(ratio1Act, &QAction::triggered, this, &MainWindow::on_action_100_triggered);
    zoomRatio->addAction(ratio1Act);

    QAction *ratio2Act = new QAction(tr("&110%"), this);
    ratio2Act->setStatusTip(tr("ratio 110%"));
    connect(ratio2Act, &QAction::triggered, this, &MainWindow::on_action_110_triggered);
    zoomRatio->addAction(ratio2Act);

    QAction *ratio3Act = new QAction(tr("&120%"), this);
    ratio3Act->setStatusTip(tr("ratio 120%"));
    connect(ratio3Act, &QAction::triggered, this, &MainWindow::on_action_120_triggered);
    zoomRatio->addAction(ratio3Act);

    QAction *ratio4Act = new QAction(tr("&150%"), this);
    ratio4Act->setStatusTip(tr("ratio 150%"));
    connect(ratio4Act, &QAction::triggered, this, &MainWindow::on_action_150_triggered);
    zoomRatio->addAction(ratio4Act);

    QAction *ratio5Act = new QAction(tr("&200%"), this);
    ratio5Act->setStatusTip(tr("ratio 200%"));
    connect(ratio5Act, &QAction::triggered, this, &MainWindow::on_action_200_triggered);
    zoomRatio->addAction(ratio5Act);

    viewMenu->addSeparator();

    Font = new QMenu(tr("&Font"));
    viewMenu->addMenu(Font);
    this->FontChoice();

    FontSize = new QMenu(tr("&Font Size"));
    viewMenu->addMenu(FontSize);
    this->FontSizeChoice();
}

void MainWindow::on_action_Magnify_triggered(){
    this->currentEditor->zoomOut(1.2);
    float size = (this->currentEditor->fontPointSize());
    this->currentEditor->setFontPointSize(size * 1.2);
    QString temp = this->currentEditor->toPlainText();
    this->currentEditor->clear();
    this->currentEditor->setText(temp);
}

void MainWindow::on_action_Shrink_triggered(){
    this->currentEditor->zoomIn(0.8);
    float size = (this->currentEditor->fontPointSize());
    this->currentEditor->setFontPointSize(size * 0.8);
    QString temp = this->currentEditor->toPlainText();
    this->currentEditor->clear();
    this->currentEditor->setText(temp);
}

void MainWindow::on_action_100_triggered(){
    float tempsize = 1.0 / this->currentzoomsize;
    qDebug() << tempsize << endl;
    this->currentEditor->zoomIn(tempsize);
    this->currentEditor->setFontPointSize(this->originfontsize);
    QString temp = this->currentEditor->toPlainText();
    this->currentEditor->clear();
    this->currentEditor->setText(temp);
    this->currentzoomsize = 1.0;
}

void MainWindow::on_action_110_triggered(){
    this->currentEditor->zoomIn(1.0 / this->currentzoomsize);
    this->currentEditor->zoomIn(1.1);
    this->currentEditor->setFontPointSize(this->originfontsize * 1.1);
    QString temp = this->currentEditor->toPlainText();
    this->currentEditor->clear();
    this->currentEditor->setText(temp);
    this->currentzoomsize = 1.1;
}

void MainWindow::on_action_120_triggered(){
    this->currentEditor->zoomIn(1.0 / this->currentzoomsize);
    this->currentEditor->zoomOut(1.2);
    this->currentEditor->setFontPointSize(this->originfontsize * 1.2);
    QString temp = this->currentEditor->toPlainText();
    this->currentEditor->clear();
    this->currentEditor->setText(temp);
    this->currentzoomsize = 1.2;
}

void MainWindow::on_action_150_triggered(){
    this->currentEditor->zoomIn(1.0 / this->currentzoomsize);
    this->currentEditor->zoomOut(1.5);
    this->currentEditor->setFontPointSize(this->originfontsize * 1.5);
    QString temp = this->currentEditor->toPlainText();
    this->currentEditor->clear();
    this->currentEditor->setText(temp);
    this->currentzoomsize = 1.5;
}

void MainWindow::on_action_200_triggered(){
    this->currentEditor->zoomIn(1.0 / this->currentzoomsize);
    this->currentEditor->zoomOut(2.0);
    this->currentEditor->setFontPointSize(this->originfontsize * 2.0);
    QString temp = this->currentEditor->toPlainText();
    this->currentEditor->clear();
    this->currentEditor->setText(temp);
    this->currentzoomsize = 2.0;
}

void MainWindow::FontChoice(){
    string fonts[5] = {"&Arial", "&Consolas", "&System", "&Times New Roman", "&Veinda"};
    for(int i = 0;i < 5;i++){
        QAction *fontAct = new QAction(tr(fonts[i].c_str()), this);
        connect(fontAct, SIGNAL(triggered(bool)), this, SLOT(on_action_Font_triggered(bool)));
        Font->addAction(fontAct);
    }
}

void MainWindow::FontSizeChoice(){
    string fontsizes[10] = {"&8", "&10", "&12", "&14", "&16", "&18", "&20", "&22", "&24", "&26"};
    for(int i = 0;i < 10;i++){
        QAction *fontsizeAct = new QAction(tr(fontsizes[i].c_str()), this);
        connect(fontsizeAct, SIGNAL(triggered(bool)), this, SLOT(on_action_FontSize_triggered(bool)));
        FontSize->addAction(fontsizeAct);
    }
}

void MainWindow::on_action_Font_triggered(bool){
    QAction *menuact = (QAction*)sender();
    QString font = menuact->text();
    font = font.mid(1);
    this->currentEditor->setFontFamily(font);
    QString temp = this->currentEditor->toPlainText();
    this->currentEditor->clear();
    this->currentEditor->setText(temp);
}

void MainWindow::on_action_FontSize_triggered(bool){
    QAction *menuact = (QAction*)sender();
    QString fontsize = menuact->text();
    fontsize = fontsize.mid(1);
    float size = fontsize.toFloat();
    this->currentEditor->setFontPointSize(size);
    QString temp = this->currentEditor->toPlainText();
    this->currentEditor->clear();
    this->currentEditor->setText(temp);
    this->originfontsize = fontsize.toFloat();
}

void MainWindow::exit(){
    std::ofstream fout("C:\\Users\\ljl\\Documents\\record", std::ios::out);
    list<string>::iterator it1= this->open_records.begin();
    for(;it1 != this->open_records.end();it1++){
        fout << (*it1) << endl;
    }
    fout.clear();
    fout.close();

    std::ofstream fout_dir("C:\\Users\\ljl\\Documents\\record_dir", std::ios::out);
    list<string>::iterator it2= this->open_records_dir.begin();
    for(;it2 != this->open_records_dir.end();it2++){
        fout_dir << (*it2) << endl;
    }
    fout_dir.clear();
    fout_dir.close();

    QWidget::close();
}

void MainWindow::freshRecent(){
    this->menu2->clear();
    this->menu3->clear();
    list<string>::iterator it1 = this->open_records.begin();
    for(;it1 != this->open_records.end();it1++){
        string openrecord = (*it1);
        string temp = "&";
        temp = temp.append(openrecord);
        QAction *openSpecialAct = new QAction(tr(temp.c_str()), this);
        openSpecialAct->setStatusTip(tr("Open this file"));

        connect(openSpecialAct, SIGNAL(triggered(bool)), this, SLOT(openrecentFile(bool)));
        menu2->addAction(openSpecialAct);
    }

    list<string>::iterator it2 = this->open_records_dir.begin();
    for(;it2 != this->open_records_dir.end();it2++){
        string openrecord = (*it2);
        string temp = "&";
        temp = temp.append(openrecord);
        QAction *openSpecialAct = new QAction(tr(temp.c_str()), this);
        openSpecialAct->setStatusTip(tr("Open this project"));

        connect(openSpecialAct, SIGNAL(triggered(bool)), this, SLOT(openrecentPro(bool)));
        menu3->addAction(openSpecialAct);
    }
}

void MainWindow::loadRecord(){
//    String curPath=QDir::currentPath().toStdString();
//    String path1=curPath+"\\Documents\\record";

    std::ifstream fin("C:\\Users\\dl188\\OneDrive\\workspace-QT\\Documents\\record", std::ios::in);
    char line[1024]={0};
    while(fin.getline(line, sizeof(line))){
        this->open_records.push_back(line);
    }
    fin.clear();
    fin.close();

    std::ifstream fin_dir("C:\\Users\\dl188\\OneDrive\\workspace-QT\\Documents\\record", std::ios::in);
    char line_dir[1024]={0};
    while(fin_dir.getline(line_dir, sizeof(line_dir))){
        this->open_records_dir.push_back(line_dir);
    }
    fin_dir.clear();
    fin_dir.close();
}


void MainWindow::initHelpMenu(){
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    QAction *aboutAct = new QAction(tr("&About us"), this);
    aboutAct->setStatusTip(tr("About us"));
    connect(aboutAct, &QAction::triggered, this, &MainWindow::about);
    helpMenu->addAction(aboutAct);
}


void MainWindow::createDockWindows()
{
    QListWidget *customerList;
    QListWidget *paragraphsList;
    QMenu *viewMenu;

    QDockWidget *dock = new QDockWidget(tr("Customers"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    customerList = new QListWidget(dock);
    customerList->addItems(QStringList()
            << "John Doe, Harmony Enterprises, 12 Lakeside, Ambleton");
    dock->setWidget(customerList);
    addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::createStatusBar()
{
    this->messageLabel=new QLabel(tr("Ready"),this);
    messageLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(messageLabel,1);

}

void MainWindow::showMassage(QString string){
    qDebug()<<"Message Receive"<<endl;
    this->messageLabel->setText(string);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    /*
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
    */
}

void MainWindow::newFile()
{
    //直接新增一个tab
    connect(tabPages,SIGNAL(tabCloseRequested(int)),this,SLOT(closeTabPage(int)));
    //将int转化为string
    stringstream ss;
    ss << tab_num;
    tab_num++;
    string name = "new";
    newEmptyMakefilePage(QString::fromStdString(name.append(ss.str())));
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getExistingDirectory(this);
//    this->projectPath=fileName;
    if (!fileName.isEmpty()){
        //loadFile(fileName);
        cout << fileName.toStdString() << endl;
        if(sign_for_dir == 0) {
            createFileDirDock(fileName, dock);
        }
        else {
            createFileDirDock(fileName, dock);
        }
    }
}

void MainWindow::openrecentPro(bool){
    QAction *menuact = (QAction*)sender();
    QString filePath = menuact->text().replace("/","\\");
    string filepath = filePath.toStdString();
    filepath = filepath.substr(1, filepath.length());
    QString filePath2 = QString::fromStdString(filepath);
    qDebug() << filePath2 << endl;

    this->createFileDirDock(filePath2, dock);
}

void MainWindow::openrecentFile(bool){
    QAction *menuact = (QAction*)sender();
    QString filePath = menuact->text().replace("/","\\");
    string filepath = filePath.toStdString();
    filepath = filepath.substr(1, filepath.length());
    QString filePath2 = QString::fromStdString(filepath);
    qDebug() << filePath2 << endl;

    char *filepath_char = new char[filepath.length()];
    strcpy(filepath_char, filepath.c_str());
    char *ptr = strrchr(filepath_char, '\\');
    string filename = ptr;
    filename = filename.substr(1, filename.length());
    QString fileName = QString::fromStdString(filename);
    qDebug() << fileName << endl;

    if(filename == "makefile" || filename == "MakeFile" || filename == "Makefile"){
        newMakefilePage(filePath2, fileName);
    }else{
        newPlainTextPage(filePath2, fileName);
    }

    return;
}


void MainWindow::openpathFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("open image file"),
                                                    "./", tr("text files(*.txt *.c *.cpp *.h *.pro);;All files (*.*)"));
    string filepath = filePath.toStdString();
    char *filepath_char = new char[filepath.length()];
    strcpy(filepath_char, filepath.c_str());
    char *ptr = strrchr(filepath_char, '/');
    string filename = ptr;
    filename = filename.substr(1, filename.length());
    QString fileName = QString::fromStdString(filename);
    qDebug() << fileName << endl;

    if(filename == "makefile" || filename == "MakeFile" || filename == "Makefile"){
        newMakefilePage(filePath, fileName);
    }else{
        newPlainTextPage(filePath, fileName);
    }

    return;
}

bool MainWindow::savefile()
{
    if(this->curFile.isEmpty())
    {
        return false;
    }
    else
    {
        QFile filename(this->curFile);
        //qDebug() << this->curFile << endl;
        if (filename.open(QFile::WriteOnly | QIODevice::Truncate)) {
            QTextStream out(&filename);
            int index = tabPages->currentIndex();
            this->currentEditor = (CodeEditor *)tabPages->widget(index);
            if(index != -1) {
                QString content = this->currentEditor->toPlainText();
                if(content != NULL){
                    out << content;
                    qDebug() << content << endl;
                }
            }
        }

        list<string>::iterator temp_it = std::find(this->open_records.begin(), this->open_records.end(), this->curFile.toStdString());
        if(temp_it != this->open_records.end()){
            this->open_records.erase(temp_it);
            this->open_records.push_front(this->curFile.toStdString());
        }
        else{
            if(this->open_records.size() <= 10){
                this->open_records.push_front(this->curFile.toStdString());
            }
            else{
                this->open_records.pop_back();
                this->open_records.push_front(this->curFile.toStdString());
            }
        }
        filename.close();
        return true;
    }
}

bool MainWindow::saveAsfile(QString name)
{
    QString filePath = QFileDialog::getSaveFileName(this,tr("Open File"), name);//获取文件夹路径
    if(filePath.isEmpty())
    {
        return false;
    }
    else
    {
        QFile filename(filePath);
        //qDebug() << filePath << endl;
        if (filename.open(QFile::WriteOnly | QIODevice::Truncate)) {
            QTextStream out(&filename);
            int index = tabPages->currentIndex();
            this->currentEditor = (CodeEditor *)tabPages->widget(index);
            if(index != -1) {
                QString content = this->currentEditor->toPlainText();
                if(content != NULL){
                    out << content;
                }
            }
        }
        list<string>::iterator temp_it = std::find(this->open_records.begin(), this->open_records.end(), this->curFile.toStdString());
        if(temp_it != this->open_records.end()){
            this->open_records.erase(temp_it);
            this->open_records.push_front(this->curFile.toStdString());
        }
        else{
            if(this->open_records.size() <= 10){
                this->open_records.push_front(this->curFile.toStdString());
            }
            else{
                this->open_records.pop_back();
                this->open_records.push_front(this->curFile.toStdString());
            }
        }
        filename.close();
        return true;
    }
}

bool MainWindow::save()
{
    int index = tabPages->currentIndex();
    QString name = tabPages->tabText(index);
    QString currentFile = this->map_for_save[index];
    bool sign = this->if_need_to_saveAs(index);
    this->curFile = currentFile;
    if(this->currentEditor->document()->isModified()){
        if (sign == true) {
            return saveAs(name);
        } else {
            this->remainSave();
        }
    }
}

bool MainWindow::saveAs(QString name)
{
    return this->saveAsfile(name);
}

bool MainWindow::saveAs2()
{
    int index = tabPages->currentIndex();
    QString name = tabPages->tabText(index);
    return this->saveAsfile(name);
}

bool MainWindow::saveAll()
{
    int counts = this->tabPages->count();
    for(int i = 0;i < counts;i++){
        QString currentFile = this->map_for_save[i];
        bool if_saved = this->if_need_to_saveAs(i);
        this->curFile = currentFile;
        this->tabPages->setCurrentIndex(i);
        this->currentEditor = (CodeEditor *)(this->tabPages->currentWidget());
        if(this->currentEditor->document()->isModified()){
            qDebug() << currentFile << endl;
            if (if_saved == true) {
                saveAs(currentFile);
            } else {
                savefile();
            }
        }
        else{
            continue;
        }
    }
    return true;
}

int if_closed = 0;

void MainWindow::closeTabPage(const int index){
    //need to configure weather need to save
    //add code here
    if(if_closed == 1) {
        if_closed = 0;
        return;
    }
    tabPages->setCurrentIndex(index);
    //int if_need_save = this->map_for_save_need[index];
    this->currentEditor = (CodeEditor *)(tabPages->currentWidget());
    if(this->currentEditor->document()->isModified()){
        QString name = tabPages->tabText(index);
        QString currentFile = this->map_for_save[index];
        bool sign = this->if_need_to_saveAs(index);
        this->curFile = currentFile;
        if (sign == true) {
            saveAs(name);
            this->tabPages->removeTab(index);
            if_closed = 1;
        } else {
            this->remainSave();
            if_closed = 1;
        }
    }
    else{
        tabPages->removeTab(index);
        if_closed = 1;
    }
    this->freshRecent();
}

void MainWindow::closeTab(){
    //need to configure weather need to save
    //add code here
    int index = this->tabPages->currentIndex();
    //int if_need_save = this->map_for_save_need[index];
    if(this->currentEditor->document()->isModified()){
        this->save();
    }
    else{
        tabPages->removeTab(index);
    }
    this->freshRecent();
}

void MainWindow::closeAll(){
    /*
    int counts = this->tabPages->count();
    for(int i = 0;i < counts;i++){
        //int if_need_to_save = this->map_for_save_need[i];
        this->tabPages->setCurrentIndex(i);
        this->currentEditor = (CodeEditor *)(this->tabPages->currentWidget());
        if(this->currentEditor->document()->isModified()) {
            this->closeTab();
        }
        else {
            tabPages->removeTab(i);
        }
    }*/
    this->saveAll();
    int counts = this->tabPages->count();
    qDebug() << counts << endl;
    tabPages->clear();
    this->freshRecent();
}

void MainWindow::about()
{
   QMessageBox::about(this, tr("About Us"),
            tr("The application is created by "
               "<b>Lei Dai, Zibing Liu, Sai Tian and Jinliang Lu.   </b>"));
}

void MainWindow::remainSave(){
    this->yes_to_Save = 0;
    this->if_Saver = new QDialog(this);
    this->if_Saver->setWindowTitle("保存");
    this->remainText = new QLabel("是否保存文件?");
    QPushButton *yesBtn = new QPushButton(tr("yes"), if_Saver);
    QPushButton *noBtn = new QPushButton(tr("no"), if_Saver);
    QVBoxLayout *mylayout = new QVBoxLayout(if_Saver);
    QHBoxLayout *under = new QHBoxLayout(if_Saver);
    under->addWidget(yesBtn);
    under->addSpacing(20);
    under->addWidget(noBtn);

    mylayout->addWidget(this->remainText);
    mylayout->addSpacing(10);
    mylayout->addLayout(under);

    connect(yesBtn, SIGNAL(clicked()), this, SLOT(yesSave()));
    connect(noBtn, SIGNAL(clicked()), this, SLOT(noSave()));

    if_Saver->show();
}

void MainWindow::yesSave(){
    qDebug() << QString::fromStdString("maybe") << endl;
    savefile();
    this->tabPages->removeTab(this->tabPages->currentIndex());
    if_Saver->close();
}

void MainWindow::noSave(){
    this->if_saver_close = 1;
    this->tabPages->removeTab(this->tabPages->currentIndex());
    if_Saver->close();
}
