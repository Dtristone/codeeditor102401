#include "codeeditor.h"
#include "mainwindow.h"
#include <QtWidgets>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QtDebug>
#include <QApplication>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QScrollBar>
#include <QCompleter>
#include <iostream>
#include <QFile>
#include <QStringList>


CodeEditor::CodeEditor(QString rootPath,QWidget *parent) : QTextEdit(parent), completer(0)
{
    //set editor font
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);
    this->setFont(font);

    //init completer and Highlighter
    highlighter=new Highlighter(this->document());

    //init location
    if(rootPath==""){
        rootPath=QDir::currentPath();
    }
    dir.setPath(rootPath);
    QStringList filter;
    filter<<"*.c"<<"*.h"<<"*.o"<<"*.cpp"<<"*.txt";
    this->staticCptList = dir.entryList(filter,QDir::Files|QDir::NoDotAndDotDot,QDir::Name);

    //set static complete
    initStaticCptModel(QDir::currentPath()+"\\file\\staticWords.txt");

    //signal
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
//    connect(this,SIGNAL(textChanged()),this,SLOT(textChangedFunc()));
//    connect(this,SIGNAL(cursorPositionChanged()),this,SLOT(cursorPositionChangedFunc()), Qt::QueuedConnection);
    connect(this->document(),SIGNAL(blockCountChanged(int)),this,SLOT(blockCountChangeFun(int)), Qt::QueuedConnection);
//    connect(this,SIGNAL(sendMessage(QString)),(MainWindow*)this->topLevelWidget(),SLOT(showMassage(QString)));

    MainWindow *mainwin;

    //two param
    lastBlockCount=this->document()->blockCount();
    lastCursorInBlockNum=this->textCursor().blockNumber();
}



//void CodeEditor::textChangedFunc(){
//    QTextCursor *cursor =new QTextCursor(this->textCursor());
//}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(Qt::yellow).lighter(160);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
}


//************************| Completer Part Start|****************************

void CodeEditor::setCompleter(QCompleter *mycompleter)
{
    if (completer)
        QObject::disconnect(completer, 0, this, 0);
    completer = mycompleter;
    if (!completer)
        return;

    completer->setWidget(this);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(completer, SIGNAL(activated(QString)),
                     this, SLOT(insertCompletion(QString)));
}

QString CodeEditor::textUnderCursor() const
{
//    QTextCursor tc = textCursor();
//    tc.select(QTextCursor::WordUnderCursor);
//    QTextCursor tc2=new QTextCursor(&this->textCursor());
    QTextCursor tc2=this->textCursor();
    tc2.select(QTextCursor::WordUnderCursor);
    QString textUnder=tc2.selectedText();

    tc2.movePosition(QTextCursor::StartOfWord,QTextCursor::MoveAnchor);
    int wordStartPosision=tc2.positionInBlock();
    if(wordStartPosision==0){

        return textUnder;
    }else {
        QString textInBlock=tc2.block().text();
        if(wordStartPosision==1){
            if(textInBlock.at(0)=='$'){
               textUnder="$"+textUnder;
            }else{
                //nothing
            }
        }else{//wordStartPosision>=2
            if(textInBlock.at(wordStartPosision-1)=="$"){
                textUnder="$"+textUnder;
            }else if(textInBlock.at(wordStartPosision-1)=="(" && textInBlock.at(wordStartPosision-2)=="$"){
                textUnder="$("+textUnder;
            }else{
                //nothing
            }
        }
    }
    return textUnder;
}

void CodeEditor::focusInEvent(QFocusEvent *e)
{
    if (completer)
        completer->setWidget(this);
    QTextEdit::focusInEvent(e);
}

//to be edit
void CodeEditor::insertCompletion(const QString& completion)
{
//    qDebug()<<"insert activated"<<endl;
    if (completer->widget() != this){
        qDebug()<<"no where to insert."<<endl;
        return;
    }
    QTextCursor tc = textCursor();
    QString mycompletion=completion;
    mycompletion.remove(0,completer->completionPrefix().length());
    int firstloc= mycompletion.indexOf("#");
    int firstjiao=mycompletion.indexOf("<");

    if(firstloc!=-1){
        mycompletion.replace(firstloc,1," ()\n\n");
        mycompletion.replace("#"," \n\n");
        tc.movePosition(QTextCursor::Left);
        int oldposion=tc.position();
        tc.movePosition(QTextCursor::EndOfWord);
        tc.insertText(mycompletion);
        tc.setPosition(oldposion);
        tc.movePosition(QTextCursor::EndOfWord);
        oldposion=tc.position();
        tc.setPosition(oldposion+2);
        setTextCursor(tc);
    }else if(firstjiao!=-1){
        tc.movePosition(QTextCursor::Left);
        int oldposion=tc.position();
        tc.movePosition(QTextCursor::EndOfWord);
        tc.insertText(mycompletion);
        int endposition=tc.position()-1;
        tc.setPosition(endposition);
        int locofempty=mycompletion.length()-mycompletion.indexOf(" ")-2;
        tc.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor,locofempty);
        setTextCursor(tc);
    }else{
        tc.movePosition(QTextCursor::Left);
        tc.movePosition(QTextCursor::EndOfWord);
        tc.insertText(mycompletion);
        setTextCursor(tc);
    }
//    int extra = completion.length() - completer->completionPrefix().length();

}

QStringListModel * CodeEditor::modelFromFile(const QString& fileName){
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)){
        qDebug()<<"error loadfile"<<endl;
        return new QStringListModel(completer);
    }
    //#ifndef QT_NO_CURSOR
    //    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    //#endif
//    QStringList words;


    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (!line.isEmpty()){
//            qDebug()<<line.trimmed()<<endl;
            staticCptList << line.trimmed();
        }

    }
    //#ifndef QT_NO_CURSOR
    //    QApplication::restoreOverrideCursor();
    //#endif
//    staticCptList=new QStringList(&words);

    return new QStringListModel(staticCptList, completer);
}

void CodeEditor::initStaticCptModel(const QString& fileName){
    this->staticModel=modelFromFile(fileName);
    this->cptModel= this->staticModel;
    completer=new QCompleter(this);
    this->completer->setModel(this->cptModel);
    completer->setWidget(this);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    connect(completer, SIGNAL(activated(QString)),this, SLOT(insertCompletion(QString)));

}

//************************| Compelting Part|****************************

//void CodeEditor::cursorPositionChangedFunc(){
//    if(lastBlockCount!=this->document()->blockCount()){//blockcount change ,upadte all
//        lastCursorInBlockNum=this->textCursor().blockNumber();
//        lastBlockCount=this->document()->blockCount();
//        return;
//    }

//    if(lastCursorInBlockNum!=this->textCursor().blockNumber()){
//        lastCursorInBlockNum=this->textCursor().blockNumber();

//    }
//    this->textCursor().blockNumber();
//}

bool isCharOfVariable(const QChar achar){
    if(achar.isLetterOrNumber() ||achar=='_' ||achar=='-'||achar =='.'){
        return true;
    }
    return false;
}

QStringList splitAline(const QString &rawstring){
    QStringList sList;
    QString string;
    int locOfJing=rawstring.indexOf('#');
    if(locOfJing==-1){
        string=rawstring;
    }else{
        if(locOfJing==0){
            return sList;
        }else{
            string=rawstring.left(locOfJing);
        }
    }

    int pointer=0;
    int len=string.length();
//    qDebug()<<len<<": "<<string<<endl;
    if(len==0){
        return sList;
    }

    while(pointer<len){
        QChar charNow=string[pointer++];

        if(charNow.isLetter()){
            QString aword="";
            while(isCharOfVariable(charNow)){
               aword.append(charNow);
               if(pointer<len){
                   charNow=string[pointer++];
               }else{
                   sList.append(aword);
                   return sList;
               }
            }
            pointer--;
            sList.append(aword);
        }else if(charNow==':' || charNow=='+' || charNow=='?'){
            QString aword="";
            aword.append(charNow);
            if(pointer<len){
                charNow=string[pointer++];
                if(charNow=='='){
                    aword.append('=');
                    sList.append(aword);
                }else{
                    pointer--;
                    sList.append(aword);
                }
            }else{
                QString aword=charNow;
                sList.append(aword);
            }
        }else if(charNow!=' ' && charNow!='\t'){
            QString aword=charNow;
            sList.append(aword);
        }
    }//while


    return sList;
}

QString getVaribleName(const QStringList& sList,const int &loc){
    QString varible;
    QStack<QString> stack;

    if(sList.at(loc-1)==')'){
        stack.push(")");
        varible=")";
        int i=loc-2;
        while(i>=0 && !stack.isEmpty()){
            QString charnow=sList.at(i);
            if(charnow==")"){
                stack.push(")");
            }else if(charnow == "("){
                stack.pop();
            }else{
//                varible=sList.at(i)+varible;
            }
            varible=sList.at(i)+varible;
            i--;
        }
//        qDebug()<< "$"+varible<<endl;
        return "$"+varible;

    }else{
//        qDebug()<< "$("+sList.at(loc-1)+")"<<endl;
        return "$("+sList.at(loc-1)+")";
    }

}

void CodeEditor::blockCountChangeFun(const int blocknum){
//    QString textNow=this->document()->toPlainText();
//    qDebug()<<"-------------"<<endl<<textNow<<endl;
    //--------------------------------------------
//    qDebug()<<"===========Block Change activated==========="<<endl;

    QSet<QString> variable;
    QSet<QString> target;
    QMultiMap<QString,QString> variableMap;
    //--------------------------------------------
    QTextBlock block=this->document()->begin();
    int i=0;
    bool notover=false;
    QString lastword;
    while(i<this->document()->blockCount()){
        QString aline=block.text();
        QStringList sList=splitAline(aline);
//        qDebug()<<sList<<endl;

        int locOfEqual=-1;

        if((locOfEqual=sList.indexOf("="))!=-1 && locOfEqual!=0){
            variable.insert(getVaribleName(sList,locOfEqual));
        }else if((locOfEqual=sList.indexOf(":="))!=-1 && locOfEqual!=0){
            variable.insert(getVaribleName(sList,locOfEqual));
        }else if((locOfEqual=sList.indexOf("+="))!=-1 && locOfEqual!=0){
            variable.insert(getVaribleName(sList,locOfEqual));
        }else if((locOfEqual=sList.indexOf("?="))!=-1 && locOfEqual!=0){
            variable.insert(getVaribleName(sList,locOfEqual));
        }else{

        }

        int locOfMaohao=-1;
        if((locOfMaohao=sList.indexOf(":"))!=-1 && locOfMaohao!=0){
            target.insert(sList[locOfMaohao-1]);
//            qDebug()<< sList[locOfMaohao-1]<<endl;
        }


        block=block.next();
        i++;
    }

    updateCptModelDynamic(variable);
    updateCptModelDynamic(target,false);

    int errorline = analyzeMakefile();
//    qDebug()<<"error : "<<errorline<<endl;
    if(errorline!=-1){
        QString errorinfo="() donot match in Line:"+ QString::number(errorline+1);
//        highlightErrorLine(errorline);
        emit sendMessage(errorinfo);
//        qDebug()<<"Message send"<<endl;
    }else{
        QString errorinfo="Ready";
        emit sendMessage(errorinfo);
    }
}

void CodeEditor::highlightErrorLine(int index)
{
    QTextCursor cursor=this->textCursor();
    int posisionNow=cursor.position();
    int move=index-cursor.blockNumber();
    cursor.setVerticalMovementX(move);
    cursor.select(QTextCursor::BlockUnderCursor);

    QTextCharFormat blockformat;
    blockformat.setBackground(Qt::lightGray);
    cursor.setBlockCharFormat(blockformat);

    this->setTextCursor(cursor);
//    this->textCursor().clearSelection();
//    this->textCursor().setPosition(posisionNow);
}


int CodeEditor::analyzeMakefile(){
//    return -1;
    QTextBlock block=this->document()->begin();
    int i=0;
    QStack<int> locStack;
    QStack<QString> strStack;
    bool alineOver=true;

    while (i<this->document()->blockCount()){
        QString aline=block.text();
        if(!aline.isNull() && aline.trimmed()==""){
//            qDebug()<<"-: "<<aline;
            block=block.next();
            i++;
            continue;
        }
//        qDebug()<<"+: "<<aline;
        QStringList sList=splitAline(aline);
        if(sList.size()==0){
            block=block.next();
            i++;
            continue;
        }

        if(alineOver){
            locStack.clear();
            strStack.clear();
        }
        for(int j=0;j<sList.size();j++){
            if(sList.at(j)=="("){
                locStack.push(i);
                strStack.push(")");
            }else if(sList.at(j) == ")"){
                if(strStack.isEmpty()){
                    return i;
                }else{
                    locStack.pop();
                    strStack.pop();
                }
            }
        }

        if(sList.last()=="\\" ){
            alineOver=false;
        }else{
            alineOver=true;
        }

        if(alineOver==true && !strStack.isEmpty()){
            return locStack.top();
        }

        block=block.next();
        i++;
    }

    if(!strStack.isEmpty()){
        return locStack.top();
    }

    return -1;
}


/*
int CodeEditor::analyzeMakefile(){
    QTextBlock block=this->document()->begin();
    int i=0;
    QStack<int> kuohaoStack;
    bool alineOver=true;
    while (i<this->document()->blockCount()){

        QString aline=block.text();
        QStringList sList=splitAline(aline);
        if(sList.isEmpty()){
            i++;
            block=block.next();
            continue;
        }
        qDebug()<<sList<<endl;
        for(int j=0;j<sList.size();j++){
            if(sList[j]=="("){
                kuohaoStack.push(i);
            }else if(sList[j]==")"){
                if(kuohaoStack.isEmpty()){
                    return j;
                }else{
                kuohaoStack.pop();
                }
            }
        }

        if(sList.last()=="\\" ){
            alineOver=false;
        }else{
            alineOver=true;
        }

        if(alineOver==true && !kuohaoStack.isEmpty()){
            return kuohaoStack.top();
        }

        block=block.next();
        i++;
    }
    if(!kuohaoStack.isEmpty()){
        return kuohaoStack.top();
    }

    return -1;
}
*/


void CodeEditor::updateCptModelDynamic(QSet<QString> stringset,bool eraseold){
    if(!dynamic_static_CptList.isEmpty()){
        if(eraseold){
//            delete(dynamic_static_CptList);
//            dynamic_static_CptList=new QStringList(staticCptList);
            dynamic_static_CptList.clear();
            dynamic_static_CptList=staticCptList;
        }
    }else{
//        dynamic_static_CptList=new QStringList(staticCptList);
        dynamic_static_CptList=staticCptList;
    }

    QSet<QString>::const_iterator sit=stringset.begin();
    while(sit!=stringset.end()){
        dynamic_static_CptList.append(*sit);
        sit++;
    }
    cptModel=new QStringListModel(dynamic_static_CptList,this->completer);
    this->completer->setModel(cptModel);
}

void CodeEditor::keyPressEvent(QKeyEvent *e){
//    qDebug()<<"KeyPressed: "<<e->text();
    QTextBlock blockNow=this->textCursor().block();
    QString blockTextNow=blockNow.text();
//    qDebug()<<blockTextNow<<endl;

    //completer has been pop up
    if(completer && completer->popup()->isVisible()){
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Backtab:
//        case Qt::Key_Dollar:
        e->ignore();
            return;
        default://pop up and press a normal key
            break;
        }
    }

    //normal key is pressed (no matter pop up or not)

    //Ctrl or Shitf pressed?
    bool ctrlorShift=e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);

    if(ctrlorShift && e->key()==Qt::Key_V){

    }
    QTextEdit::keyPressEvent(e);


    QString completionPrefix = textUnderCursor();
//    qDebug()<<"completionPrefix: "<<completionPrefix<<endl;

    switch (e->key()) {
    case Qt::Key_Return://hide and return pressed
        break;
    case '/'://"/" pressed

        break;
    default://all others
        break;
    }



    if(!completer){// no completer is set
        return;
    }
    if (ctrlorShift){//hide and only ctrl or shift is pressed
        if(e->text().isEmpty()){
//            qDebug()<<"Ctrl Shift -empty: "<<e->text()<<endl;
        }else{
//            qDebug()<<"Ctrl Shift -notempty: "<<e->text()<<endl;
        }
//        return;
    }

    static QString eow("~!@#%^&*)_+{}|:\"<>?,./;'[]\\-="); // end of word


//    qDebug()<<"completionPrefix: "<<completionPrefix<<endl;
    //end of a word / input has no text / blocktext now is empty
    if(eow.contains(e->text().right(1))|| e->text().isEmpty()||completionPrefix.length() < 1){
        completer->popup()->hide();
        return;
    }

    //set complete pop up
    if (completionPrefix != completer->completionPrefix()) {
        completer->setCompletionPrefix(completionPrefix);
        completer->popup()->setCurrentIndex(completer->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    cr.setWidth(completer->popup()->sizeHintForColumn(0)
                + completer->popup()->verticalScrollBar()->sizeHint().width());
    completer->complete(cr);


}
