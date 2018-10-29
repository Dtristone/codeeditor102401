#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QObject>
#include <QWidget>
#include <QTextEdit>
#include <QCompleter>
#include <highlighter.h>
#include <QStringListModel>
#include <QAbstractItemModel>
#include <QList>
#include <QDir>

class CodeEditor : public QTextEdit
{
    Q_OBJECT

//////////////////////////////////////////////////////
public:
    explicit CodeEditor(QString rootPath="",QWidget *parent = nullptr);
    void setCompleter(QCompleter *c);
//    QCompleter *completer() const;

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;

private:
    QString textUnderCursor() const;
    QStringListModel *modelFromFile(const QString& fileName);
    void updataCptModel();
    void ReturnPressedHandler(QKeyEvent *e);
    void updateCptModelDynamic(QSet<QString> stringset,bool eraseold=true);
    void highlightErrorLine(int index);

signals:
    void sendMessage(QString stirng);

public slots:
     void highlightCurrentLine();


private slots:
    void insertCompletion(const QString &completion);
//    void textChangedFunc();
//    void cursorPositionChangedFunc();
    void initStaticCptModel(const QString& fileName=":/file/staticWords.txt");
    void blockCountChangeFun(const int blocknum);

private:
    QCompleter *completer;
    Highlighter *highlighter;

    QStringListModel *cptModel;
    QStringListModel *staticModel;
//    QStringListModel *dynamicModel;

    //System Path complete model

    QStringList staticCptList;
    QStringList dynamic_static_CptList;
    int analyzeMakefile();

    //root
//    QString rootPath;
    QDir dir;

    int lastBlockCount;
    int lastCursorInBlockNum;

    bool analyzeResult;
};

#endif // CODEEDITOR_H
