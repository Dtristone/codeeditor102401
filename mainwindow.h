#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QString>
#include <QDockWidget>
#include <QListWidget>
#include "codeeditor.h"
#include <QFileSystemModel>
#include <QPlainTextEdit>
#include <QSessionManager>
#include <QDockWidget>
#include <QVBoxLayout>
#include <map>
#include <QLabel>
#include <QMenu>
#include <list>
using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void loadFile(const QString &filename);

    ~MainWindow();
protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
    QTabWidget *tabPages;
    QFileSystemModel *fsmodel;
    QString projectPath;
    QLabel *messageLabel;
    QString openedProjectPath;


    QLineEdit *findLineEdit;
    QDialog *findDlg;
    QLabel *findlabel;
    QLabel *countlabel;

    QLineEdit *findEdit;
    QLineEdit *replaceEdit;
    QDialog *replaceDlg;
    QLabel *befindlabel;
    QLabel *replacelabel;
    QString replacedtext;
    int sign_for_replace = 0;
    float currentzoomsize = 1.0;
    float originfontsize = 12;

    QDialog *if_Saver;
    QLabel *remainText;
    int yes_to_Save;
    int if_saver_close = 0;

private:
    void initMenu();
    void initFileMenu();
    QMenu *menu2;
    QMenu *menu3;
    void freshRecent();
    void initEditMenu();
    void initSearchMenu();
    void initViewMenu();
    QMenu *Font;
    QMenu *FontSize;
    void initHelpMenu();

    void FontChoice();
    void FontSizeChoice();

    void initTabPage();
    void createDockWindows();
    void createStatusBar();

    void createFileDirDock(QString rootDir="", QDockWidget* dock = NULL);
    bool canBeShow(const QModelIndex &index);
    void messageRemain(const char *info);
    void loadRecord();
    QAction* addMenuButton(string theme, string QIconPath, string use, string tip);

    void remainSave();

    CodeEditor *currentEditor;

    QString curFile;

    map<int, QString> map_for_save;
    map<int, int> map_for_remember;
    map<int, int> map_for_save_need;
    list<string> open_records;
    list<string> open_records_dir;

    //QMenu *fileMenu;

    void newEmptyMakefilePage(const QString str="Makefile");
    bool newPlainTextPage(const QString filePath,const QString filename);
    bool newMakefilePage(const QString filePath,const QString filename);

    bool if_need_to_saveAs(int index){
        QString tempname = tabPages->tabText(index);
        if(tempname.contains("new")) {
            return true;
        }
        else{
            return false;
        }
    }

private slots:
    void newFile();
    void openpathFile();
    void open();
    void openrecentFile(bool);
    void openrecentPro(bool);
    bool savefile();
    bool saveAsfile(QString name);
    bool save();
    bool saveAs(QString name);
    bool saveAs2();
    bool saveAll();
    void exit();
    void about();

    void on_action_Undo_triggered();
    void on_action_Redo_triggered();
    void on_action_Cut_triggered();
    void on_action_Copy_triggered();
    void on_action_Paste_triggered();

    void on_action_Search_triggered();
    void on_action_Replace_triggered();

    void findnextText();
    void searchText();
    void countText();

    void find_replace_Text();
    void replaceText();
    void replaceallText();

    void on_action_Magnify_triggered();
    void on_action_Shrink_triggered();
    void on_action_100_triggered();
    void on_action_110_triggered();
    void on_action_120_triggered();
    void on_action_150_triggered();
    void on_action_200_triggered();

    void on_action_Font_triggered(bool);
    void on_action_FontSize_triggered(bool);

    void yesSave();
    void noSave();


public slots:
    bool openFile(const QModelIndex &index);
    void closeTabPage(int index);
    void closeTab();
    void closeAll();
    void showMassage(QString string);

};

#endif // MAINWINDOW_H
