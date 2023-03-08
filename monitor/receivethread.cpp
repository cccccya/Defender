#include "mainwindow.h"
QString OperationType[22]={
    "",
    "MessageBoxA",
    "MessageBoxW",
    "CreateFile",
    "WriteFile",
    "ReadFile",         //5
    "HeapCreate",
    "HeapDestory",
    "HeapAlloc",
    "HeapFree",
    "RegCreateKeyEx",   //10
    "RegSetValueEx",
    "RegCloseKey",
    "RegOpenKeyEx",
    "RegQueryValueEx",
    "RegDeleteValue",   //15
    "RegDeleteKeyEx",
    "socket",
    "bind",
    "send",
    "connect",
    "recv"              //21
};

ReceiveThread::ReceiveThread(){

}

ReceiveThread::~ReceiveThread(){
    StopRunning();
    terminate();
}

void ReceiveThread::run(){
    StartRunnning();
    HANDLE hMap = CreateFileMappingA(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,MAX_QUEUE_NODE_COUNT*sizeof(info),"ShareMemory");
    LPVOID lpMap = MapViewOfFile(hMap,FILE_MAP_ALL_ACCESS,0,0,0);
    QStringList arguments;
    arguments<<FilePath;
    qDebug()<<FilePath;
    qDebug()<<FileName;
    process.start("../syringe/Debug/syringe.exe",arguments);
    int num=0;
    QString WarningStr;
    std::string MD5temp;
    while(running){
        info* temp=((info*)lpMap+num);
        if(temp->type!=0){
            QString str;
            str+='\n';
            str+=OperationType[temp->type];
            str+='\n';
            for(int i=0;i<temp->argNum;i++){
                str+=temp->argName[i];
                str+=": ";
                str+=temp->argValue[i];
                str+='\n';
            }
            switch(temp->type){
            case 3:
                if(!MULTIPLE_FOLDER&&AddFolderPath(temp->argValue[0])){
                    MULTIPLE_FOLDER=1;
                    WarningStr="文件读写操作包含多个文件夹\n";
                    emit addwarning(WarningStr);
                }
                if(!AB_MODIFY&&CheckABModify(temp)){
                    AB_MODIFY=1;
                    WarningStr="修改可执行代码\n";
                    emit addwarning(WarningStr);
                }
                break;
            case 4:
                MD5temp=temp->MD5_Result;
                qDebug()<<"WriteFile MD5:"<<temp->MD5_Result;
                if(!SELFCOPY&&MD5temp==MD5CurrentFile){
                    SELFCOPY=1;
                    WarningStr="存在自我复制操作\n";
                    emit addwarning(WarningStr);
                }
                break;
            case 5://ReadFile
                MD5temp=temp->MD5_Result;
                ReadMD5Map[MD5temp]=1;
                std::cout<<MD5temp<<std::endl;
                break;
            case 8:
                if(!MULTIPLE_FREE&&MultipleFree(temp)){
                    MULTIPLE_FREE=1;
                    WarningStr="堆操作过程重复释放\n";
                    emit addwarning(WarningStr);
                }
                break;
            case 10:
                if(!CREATE_REG&&CreateReg(temp)){
                    CREATE_REG=1;
                    WarningStr="新增注册表项\n";
                    emit addwarning(WarningStr);
                }
                if(!STARTUP&&CreateStartUp(temp)){
                    STARTUP=1;
                    WarningStr="新增注册表自启动执行文件项\n";
                    emit addwarning(WarningStr);
                }
                break;
            case 11:
                if(!MODIFY_REGVALUE){
                    MODIFY_REGVALUE=1;
                    WarningStr="修改注册表值\n";
                    emit addwarning(WarningStr);
                }
                break;
            case 13:
                if(!STARTUP&&CreateStartUp(temp)){
                    STARTUP=1;
                    WarningStr="新增注册表自启动执行文件项\n";
                    emit addwarning(WarningStr);
                }
                break;
            case 15:
                if(!DELETE_REGVALUE){
                    DELETE_REGVALUE=1;
                    WarningStr="删除注册表值\n";
                    emit addwarning(WarningStr);
                }
                break;
            case 16:
                if(!DELETE_REG){
                    DELETE_REG=1;
                    WarningStr="删除注册表\n";
                    emit addwarning(WarningStr);
                }
                break;
            default:
                break;
            }

            num++;
//            qDebug()<<str;
            emit updateinfo(str);
        }
        Sleep(100);
    }
    process.waitForFinished();
    process.terminate();
    CloseHandle(hMap);
}

void ReceiveThread::StartRunnning(){
    running=1;
}

void ReceiveThread::StopRunning(){
    running=0;
    process.terminate();
    QStringList params;
    params<<"/c"<<"taskkill"<<"-f"<<"-im"<<FileName;
    QProcess process;
    process.start("cmd.exe",params);
    process.waitForFinished();
    process.close();
}

void ReceiveThread::GetFilePath(QString str){
    FilePath=str;
    std::string s;
    for(int i=str.length()-1;i>=0;i--){
        if(str[i]=='/')break;
        s+=str[i].toLatin1();
    }
    reverse(s.begin(),s.end());
    FileName+=s.c_str();
    MD5 md5;
    std::ifstream in(FilePath.toStdString(), std::ios::binary);
    if (in){
        md5.reset();
        md5.update(in);
        MD5CurrentFile = md5.toString();
        std::cout<<"MD5CurrentFile:"<<MD5CurrentFile<<std::endl;
    }
}

bool ReceiveThread::AddFolderPath(char* str){
    std::string FolderPath=str;
    qDebug()<<str;
    while(FolderPath.back()!='\\')FolderPath.pop_back();
    FolderPath.pop_back();
    std::cout<<FolderPath<<std::endl;
    if(FolderPathMap[FolderPath])return 0;
    FolderPathMap[FolderPath]=1;return 1;
}

bool ReceiveThread::CheckABModify(info* temp){
    unsigned int dwDesiredAccess=strtoul(temp->argValue[1],NULL,16);
    if(dwDesiredAccess&GENERIC_WRITE){
        std::string s;
        for(int i=strlen(temp->argValue[0])-1;i>=0;i--){
            if(temp->argValue[0][i]=='.')break;
            s+=temp->argValue[0][i];
        }
        reverse(s.begin(),s.end());
        if(s=="exe"||s=="dll"||s=="ocx")return 1;
    }
    return 0;
}

bool ReceiveThread::MultipleFree(info* temp){
    unsigned int lpMem=strtoul(temp->argValue[1],NULL,16);
    if(lpMemNum[lpMem]){
        return 1;
    }
    else {
        lpMemNum[lpMem]=1;
        return 0;
    }
}

bool ReceiveThread::CreateReg(info* temp){
    unsigned int lpdwDisposition=strtoul(temp->argValue[8],NULL,16);
    if(lpdwDisposition&REG_CREATED_NEW_KEY)return 1;
    return 0;
}

bool ReceiveThread::CreateStartUp(info* temp){
    unsigned int hKey=strtoul(temp->argValue[0],NULL,16);
    if(hKey==(unsigned int)HKEY_CURRENT_USER&&strcmp(temp->argValue[1],"Software\\Microsoft\\Windows\\CurrentVersion\\Run")==0) return 1;
    if(hKey==(unsigned int)HKEY_LOCAL_MACHINE&&strcmp(temp->argValue[1],"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run")==0) return 1;
    if(hKey==(unsigned int)HKEY_LOCAL_MACHINE&&strcmp(temp->argValue[1],"Software\\Microsoft\\Windows\\CurrentVersion\\Run")==0) return 1;
    qDebug()<<temp->argValue[1];
    return 0;
}

void ReceiveThread::reset(){
    MULTIPLE_FOLDER=0,AB_MODIFY=0,MULTIPLE_FREE=0,CREATE_REG=0,MODIFY_REGVALUE=0,DELETE_REGVALUE=0,DELETE_REG=0,STARTUP=0,SELFCOPY=0;
}
