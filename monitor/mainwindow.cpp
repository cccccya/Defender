#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QFile f("../QSS/darkstyle.qss");

    if (!f.exists())   {
        printf("Unable to set stylesheet, file not found\n");
    }
    else   {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }
    init();
}

MainWindow::~MainWindow()
{
    ThreadA.~ReceiveThread();
    delete ui;
}

void MainWindow::init(){
    connect(&ThreadA, SIGNAL(updateinfo(QString)), this, SLOT(UpdateInfomation(QString)));
    connect(&ThreadA, SIGNAL(addwarning(QString)), this, SLOT(UpdateWarning(QString)));
    connect(this,SIGNAL(FilePathInfo(QString)),&ThreadA,SLOT(GetFilePath(QString)));
}

void MainWindow::on_ChooseFile_clicked()
{
    QString FileName=QFileDialog::getOpenFileName(this,tr("open a file."),"../example",tr("app(*.exe);;All files(*.*)"));
    if(!FileName.isEmpty()){
        ui->lineEdit->setText(FileName);
    }
}

void MainWindow::on_Start_clicked()
{
    emit FilePathInfo((QString)ui->lineEdit->text());
    ThreadA.start();
}

void MainWindow::UpdateInfomation(QString output){
    ui->Dialog->append(output);
    ui->Dialog->moveCursor(QTextCursor::End);
}
void MainWindow::UpdateWarning(QString output){
    ui->Warning->insertPlainText(output);
}

void MainWindow::on_StopMonitor_clicked()
{
    ThreadA.StopRunning();
}

void MainWindow::on_pushButton_clicked()
{
    QMessageBox msgBox;
    msgBox.setText("提示");
    msgBox.setInformativeText("确认要导出到Log中吗?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();
    if(ret == QMessageBox::Yes){
        QDateTime time = QDateTime::currentDateTime();
        QString filename = "../Log/"+time.toString("yyyy-MM-dd hh-mm-ss")+".txt";
        qDebug()<<filename;
        QFile file(filename);
        //file.open( QIODevice::ReadWrite | QIODevice::Text );
        //file.close();
        file.open(QIODevice::WriteOnly);
        QString str=ui->Dialog->toPlainText();
        int len=file.write(str.toUtf8());
        file.close();
        if(len==str.length()){
            msgBox.setText("提示");
            msgBox.setInformativeText("导出成功");
            msgBox.setStandardButtons(QMessageBox::Ok);
        }
        else {
            msgBox.setText("提示");
            msgBox.setInformativeText("导出失败");
            msgBox.setStandardButtons(QMessageBox::Ok);
        }
        msgBox.exec();
    }
}

void MainWindow::on_clear_clicked()
{
    QMessageBox msgBox;
    msgBox.setText("提示");
    msgBox.setInformativeText("确认要清楚所有监控数据吗?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();
    if(ret == QMessageBox::Yes){
        ui->Dialog->clear();
        ui->Warning->clear();
        ThreadA.reset();
        msgBox.setText("提示");
        msgBox.setInformativeText("清空成功");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }
}
