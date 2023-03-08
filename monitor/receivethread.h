#ifndef RECEIVETHREAD_H
#define RECEIVETHREAD_H
#include <QObject>
#include <QThread>
#include <QProcess>
#include <windows.h>
#include <string.h>
#include <iostream>
#include <QDebug>
#include <locale.h>


struct Information{
    int type, argNum;
    SYSTEMTIME st;
    char argName[10][30] = { 0 };
    char argValue[10][70] = { 0 };
};




#endif // RECEIVETHREAD_H
