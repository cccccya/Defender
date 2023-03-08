#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>
#include <stdlib.h>
#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QThread>
#include <QTextCodec>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QDateTime>
#include <windows.h>
#include <unordered_map>
#include <md5.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define MAX_QUEUE_NODE_COUNT 1000
struct info {
    int type, argNum;
    SYSTEMTIME st;
    char argName[10][30];
    char argValue[10][10010];
    char MD5_Result[1000];
};

class ReceiveThread : public QThread
{
    Q_OBJECT
protected:
    //执行函数
    void run();
public:
    ReceiveThread();
    ~ReceiveThread();
    //状态设置
    void StartRunnning();
    void StopRunning();
    void reset();
private:
    //被监测文件路径
    QString FilePath;
    //被监测文件名
    QString FileName;
    //被监测二进制文件MD5摘要
    std::string MD5CurrentFile;
    QProcess process;
    bool running;
    //危险操作警告
    bool MULTIPLE_FOLDER=0,AB_MODIFY=0,MULTIPLE_FREE=0,CREATE_REG=0,MODIFY_REGVALUE=0,DELETE_REGVALUE=0,DELETE_REG=0,STARTUP=0,SELFCOPY=0;
    DWORD SelfFileSize;
    unsigned char* SelfBuffer;
    //检验是否修改可执行文件
    bool CheckABModify(info*);
    //确定当前操作目录，判断是否操作多目录
    bool AddFolderPath(char*);
    //检验是否重复释放
    bool MultipleFree(info*);
    //检验是否创建注册表新键
    bool CreateReg(info*);
    //检验是否创建自启动注册表项
    bool CreateStartUp(info*);
    //操作目录
    std::unordered_map<std::string,bool>FolderPathMap;
    //readfile内容
    std::unordered_map<std::string,bool>ReadMD5Map;
    //堆创建次数
    std::unordered_map<unsigned int,bool>lpMemNum;
signals:
    void updateinfo(QString);
    void addwarning(QString);
public slots:
    void GetFilePath(QString);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
public:
    ReceiveThread ThreadA;
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void FilePathInfo(QString);

private slots:
    void on_ChooseFile_clicked();

    void on_Start_clicked();

    void UpdateInfomation(QString);

    void UpdateWarning(QString);

    void on_StopMonitor_clicked();

    void on_pushButton_clicked();

    void on_clear_clicked();

private:
    Ui::MainWindow *ui;
    void init();
};

#endif // MAINWINDOW_H
