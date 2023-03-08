//预处理项中需添加 _WINSOCK_DEPRECATED_NO_WARNINGS _CRT_SECURE_NO_WARNINGS 两项
// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "framework.h"
#include "detours.h"
#include "stdio.h"
#include "stdarg.h"
#include "windows.h"
#include <WINSOCK2.H>   
#include <iostream>
#include <mutex>
#include <unordered_set>
#include "md5.h"


#pragma comment(lib,"detours.lib")
#pragma comment(lib,"ws2_32.lib")

#define MAX_QUEUE_NODE_COUNT 100

#define MESSAGEBOXA 1
#define MESSAGEBOXW 2
#define CREATEFILE 3
#define WRITEFILE 4
#define READFILE 5
#define HEAPCREATE 6
#define HEAPDESTORY 7
#define HEAPALLOC 8
#define HEAPFREE 9
#define REGCREATEKEYEX 10
#define REGSETVALUEEX 11
#define REGCLOSEKEY 12
#define REGOPENKEYEX 13
#define REGQUERYVALUEEX 14
#define REGDELETEVALUE 15
#define REGDELETEKEYEX 16
#define THESOCKET 17
#define BIND 18
#define SEND 19
#define CONNECT 20
#define RECV 21

std::unordered_set<HANDLE>HeapHandle;

struct info {
    int type, argNum;
    SYSTEMTIME st;
    char argName[10][30] = { 0 };
    char argValue[10][10010] = { 0 };
    char MD5_Result[1000];
}sendInfo;

std::string GbkToUtf8(const char* src_str) {
    int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* str = new char[len + 1];
    memset(str, 0, len + 1);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
    std::string strTemp = str;
    if (wstr)delete[] wstr;
    if (str)delete[] str;
    return strTemp;
}


char* ConvertLPWSTRToLPSTR(LPCWSTR lpwszStrIn)
{
    LPSTR pszOut = NULL;
    try
    {
        if (lpwszStrIn != NULL)
        {
            int nInputStrLen = wcslen(lpwszStrIn);

            // Double NULL Termination  
            int nOutputStrLen = WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, NULL, 0, 0, 0) + 2;
            pszOut = new char[nOutputStrLen];

            if (pszOut)
            {
                memset(pszOut, 0x00, nOutputStrLen);
                WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, pszOut, nOutputStrLen, 0, 0);
            }
        }
    }
    catch (std::exception e)
    {
    }
    return pszOut;
}

SYSTEMTIME st;
int cnt = 0;

HANDLE hMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS, NULL, "ShareMemory");
LPVOID lpMap = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);


//定义和引入需要Hook的函数，和替换的函数
static int (WINAPI* OldMessageBoxW)(_In_opt_ HWND hWnd, _In_opt_ LPCWSTR lpText, _In_opt_ LPCWSTR lpCaption, _In_ UINT uType) = MessageBoxW;
static int (WINAPI* OldMessageBoxA)(_In_opt_ HWND hWnd, _In_opt_ LPCSTR lpText, _In_opt_ LPCSTR lpCaption, _In_ UINT uType) = MessageBoxA;


extern "C" __declspec(dllexport) int WINAPI NewMessageBoxA(_In_opt_ HWND hWnd, _In_opt_ LPCSTR lpText, _In_opt_ LPCSTR lpCaption, _In_ UINT uType)
{
    sendInfo.type = MESSAGEBOXA;
    GetLocalTime(&(sendInfo.st));
    sendInfo.argNum = 4;
    std::string Text = GbkToUtf8(lpText), Caption = GbkToUtf8(lpCaption);
    // 参数名
    sprintf(sendInfo.argName[0], "hWnd");
    sprintf(sendInfo.argName[1], "lpText");
    sprintf(sendInfo.argName[2], "lpCaption");
    sprintf(sendInfo.argName[3], "uType");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", (unsigned int)hWnd);
    sprintf(sendInfo.argValue[1], "%s", Text.c_str());
    sprintf(sendInfo.argValue[2], "%s", Caption.c_str());
    sprintf(sendInfo.argValue[3], "%08X", uType);
    memcpy((info*)lpMap + cnt, &sendInfo, sizeof(info));
    cnt++;
    return OldMessageBoxA(hWnd, lpText, lpCaption, uType);
}


extern "C" __declspec(dllexport) int WINAPI NewMessageBoxW(_In_opt_ HWND hWnd, _In_opt_ LPCWSTR lpText, _In_opt_ LPCWSTR lpCaption, _In_ UINT uType)
{
    sendInfo.type = MESSAGEBOXW;
    GetLocalTime(&(sendInfo.st));
    std::string Text = GbkToUtf8(ConvertLPWSTRToLPSTR(lpText)), Caption = GbkToUtf8(ConvertLPWSTRToLPSTR(lpCaption));
    sendInfo.argNum = 4;
    // 参数名
    sprintf(sendInfo.argName[0], "hWnd");
    sprintf(sendInfo.argName[1], "lpText");
    sprintf(sendInfo.argName[2], "lpCaption");
    sprintf(sendInfo.argName[3], "uType");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", (unsigned int)hWnd);
    strcpy(sendInfo.argValue[1], Text.c_str());
    strcpy(sendInfo.argValue[2], Caption.c_str());
    sprintf(sendInfo.argValue[3], "%08X", uType);
    memcpy((info*)lpMap + cnt, &sendInfo, sizeof(info));
    cnt++;
    return OldMessageBoxW(hWnd, lpText, lpCaption, uType);
}

//文件操作 OpenFile CreateFile
static HANDLE(WINAPI* OldCreateFile)(
    LPCTSTR               lpFileName,               //文件名
    DWORD                 dwDesiredAccess,          //访问模式
    DWORD                 dwShareMode,              //共享模式
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,     //安全属性（销毁方式）
    DWORD                 dwCreationDisposition,    //how to create
    DWORD                 dwFlagsAndAttributes,     //文件属性
    HANDLE                hTemplateFile             //模板文件句柄
    ) = CreateFile;


extern "C" __declspec(dllexport)HANDLE WINAPI NewCreateFile(
    LPCTSTR               lpFileName,               //文件名
    DWORD                 dwDesiredAccess,          //访问模式
    DWORD                 dwShareMode,              //共享模式
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,     //安全属性（销毁方式）
    DWORD                 dwCreationDisposition,    //how to create
    DWORD                 dwFlagsAndAttributes,     //文件属性
    HANDLE                hTemplateFile             //模板文件句柄
)
{

    char temp[70];
    sendInfo.type = CREATEFILE;
    GetLocalTime(&(sendInfo.st));

    sendInfo.argNum = 7;
    // 参数名
    sprintf(sendInfo.argName[0], "lpFileName");
    sprintf(sendInfo.argName[1], "dwDesiredAccess");
    sprintf(sendInfo.argName[2], "dwShareMode");
    sprintf(sendInfo.argName[3], "lpSecurityAttributes");
    sprintf(sendInfo.argName[4], "dwCreationDisposition");
    sprintf(sendInfo.argName[5], "dwFlagsAndAttributes");
    sprintf(sendInfo.argName[6], "hTemplateFile");
    // 参数值
    // 宽字节转char
    memset(temp, 0, sizeof(temp));
    WideCharToMultiByte(CP_ACP, 0, lpFileName, wcslen(lpFileName), temp, sizeof(temp), NULL, NULL);
    strcpy(sendInfo.argValue[0], temp);
    sprintf(sendInfo.argValue[1], "%08X", dwDesiredAccess);
    sprintf(sendInfo.argValue[2], "%08X", dwShareMode);
    sprintf(sendInfo.argValue[3], "%08X", (unsigned int)lpSecurityAttributes);
    sprintf(sendInfo.argValue[4], "%08X", dwCreationDisposition);
    sprintf(sendInfo.argValue[5], "%08X", dwFlagsAndAttributes);
    sprintf(sendInfo.argValue[6], "%08X", (unsigned int)hTemplateFile);

    memcpy((info*)lpMap+cnt, &sendInfo, sizeof(info));
    cnt++;
    return OldCreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

//文件操作 WriteFile
static BOOL(WINAPI* OldWriteFile)(
    HANDLE       hFile,
    LPCVOID      lpBuffer,
    DWORD        nNumberOfBytesToWrite,
    LPDWORD      lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped
    ) = WriteFile;

extern "C" __declspec(dllexport)BOOL WINAPI NewWriteFile(
    HANDLE       hFile,
    LPCVOID      lpBuffer,
    DWORD        nNumberOfBytesToWrite,
    LPDWORD      lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped
)
{
    bool ret = OldWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
    if (GetFileType(hFile) == FILE_TYPE_DISK) {
        sendInfo.argNum = 5;
        // 参数名
        sprintf(sendInfo.argName[0], "hFile");
        sprintf(sendInfo.argName[1], "lpBuffer");
        sprintf(sendInfo.argName[2], "nNumberOfBytesToWrite");
        sprintf(sendInfo.argName[3], "lpNumberOfBytesWritten");
        sprintf(sendInfo.argName[4], "lpOverlapped");
        // 参数值
        sprintf(sendInfo.argValue[0], "%08X", (unsigned int)hFile);
        sprintf(sendInfo.argValue[1], "%08X", (unsigned int)lpBuffer);
        sprintf(sendInfo.argValue[2], "%08X", nNumberOfBytesToWrite);
        sprintf(sendInfo.argValue[3], "%08X", (unsigned int)lpNumberOfBytesWritten);
        sprintf(sendInfo.argValue[4], "%08X", (unsigned int)lpOverlapped);

        MD5 md5;
        md5.reset();
        md5.update(lpBuffer, (size_t)nNumberOfBytesToWrite);
        std::string MD5temp = md5.toString();
        strcpy(sendInfo.MD5_Result, MD5temp.c_str());

        sendInfo.type = WRITEFILE;
        GetLocalTime(&(sendInfo.st));
        memcpy((info*)lpMap + cnt, &sendInfo, sizeof(info));
        cnt++;
    }
    return ret;
}

static BOOL(WINAPI* OldReadFile)(
    HANDLE       hFile,
    LPVOID       lpBuffer,
    DWORD        nNumberOfBytesToRead,
    LPDWORD      lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
    ) = ReadFile;

extern "C" __declspec(dllexport)BOOL WINAPI NewReadFile(
    HANDLE       hFile,
    LPVOID       lpBuffer,
    DWORD        nNumberOfBytesToRead,
    LPDWORD      lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
)
{
    bool ret = OldReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
    if (GetFileType(hFile) == FILE_TYPE_DISK) {
        sendInfo.argNum = 5;
        // 参数名
        sprintf(sendInfo.argName[0], "hFile");
        sprintf(sendInfo.argName[1], "lpBuffer");
        sprintf(sendInfo.argName[2], "nNumberOfBytesToRead");
        sprintf(sendInfo.argName[3], "lpNumberOfBytesRead");
        sprintf(sendInfo.argName[4], "lpOverlapped");
        // 参数值
        sprintf(sendInfo.argValue[0], "%08X", (unsigned int)hFile);
        sprintf(sendInfo.argValue[1], "%08X", (unsigned int)lpBuffer);
        sprintf(sendInfo.argValue[2], "%08X", nNumberOfBytesToRead);
        sprintf(sendInfo.argValue[3], "%08X", (unsigned int)lpNumberOfBytesRead);
        sprintf(sendInfo.argValue[4], "%08X", (unsigned int)lpOverlapped);
        
        MD5 md5;
        md5.reset();
        md5.update(lpBuffer, (size_t)nNumberOfBytesToRead);
        std::string MD5temp = md5.toString();
        strcpy(sendInfo.MD5_Result, MD5temp.c_str());

        sendInfo.type = READFILE;
        GetLocalTime(&(sendInfo.st));
        memcpy((info*)lpMap + cnt, &sendInfo, sizeof(info));
        cnt++;
    }
    return ret;
}

//堆操作 HeapCreate HeapDestroy HeapAlloc HeapFree
static HANDLE(WINAPI* OldHeapCreate)(DWORD fIOoptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize) = HeapCreate;

extern "C" __declspec(dllexport)HANDLE WINAPI NewHeapCreate(DWORD fIOoptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize)
{
    HANDLE hHeap = OldHeapCreate(fIOoptions, dwInitialSize, dwMaximumSize);
    HeapHandle.insert(hHeap);
    sendInfo.argNum = 4;
    // 参数名
    sprintf(sendInfo.argName[0], "fIOoptions");
    sprintf(sendInfo.argName[1], "dwInitialSize");
    sprintf(sendInfo.argName[2], "dwMaximumSize");
    sprintf(sendInfo.argName[3], "HANDLE");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", fIOoptions);
    sprintf(sendInfo.argValue[1], "%08X", dwInitialSize);
    sprintf(sendInfo.argValue[2], "%08X", dwMaximumSize);
    sprintf(sendInfo.argValue[3], "%08X", (unsigned int)hHeap);
    sendInfo.type = HEAPCREATE;
    GetLocalTime(&(sendInfo.st));
    memcpy((info*)lpMap + cnt, &sendInfo, sizeof(info));
    cnt++;
    return hHeap;
}

static BOOL(WINAPI* OldHeapDestory)(HANDLE) = HeapDestroy;

extern "C" __declspec(dllexport) BOOL WINAPI NewHeapDestory(HANDLE hHeap)
{
    HeapHandle.erase(hHeap);
    sendInfo.argNum = 1;
    // 参数名
    sprintf(sendInfo.argName[0], "hHeap");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", (unsigned int)hHeap);
    sendInfo.type = HEAPDESTORY;
    GetLocalTime(&(sendInfo.st));
    memcpy((info*)lpMap + cnt, &sendInfo, sizeof(info));
    cnt++;
    return OldHeapDestory(hHeap);
}

static BOOL(WINAPI* OldHeapFree)(HANDLE hHeap, DWORD dwFlags, _Frees_ptr_opt_ LPVOID lpMem) = HeapFree;
extern "C" __declspec(dllexport) BOOL WINAPI NewHeapFree(HANDLE hHeap, DWORD dwFlags, _Frees_ptr_opt_ LPVOID lpMem) {
    sendInfo.argNum = 3;
    // 参数名
    sprintf(sendInfo.argName[0], "hHeap");
    sprintf(sendInfo.argName[1], "dwFlags");
    sprintf(sendInfo.argName[2], "lpMem");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", (unsigned int)hHeap);
    sprintf(sendInfo.argValue[1], "%08X", dwFlags);
    sprintf(sendInfo.argValue[2], "%08X", (unsigned int)lpMem);
    sendInfo.type = HEAPFREE;
    GetLocalTime(&(sendInfo.st));
    memcpy((info*)lpMap + cnt, &sendInfo, sizeof(info));
    cnt++;
    return OldHeapFree(hHeap, dwFlags, lpMem);
}

static LPVOID (WINAPI* OldHeapAlloc)(HANDLE hHeap, DWORD dwFlags, DWORD dwBytes) = HeapAlloc;
extern "C" __declspec(dllexport) LPVOID WINAPI NewHeapAlloc(HANDLE hHeap, DWORD dwFlags, DWORD dwBytes) {
    if (HeapHandle.count(hHeap))
    {
        sendInfo.argNum = 3;
        // 参数名
        sprintf(sendInfo.argName[0], "hHeap");
        sprintf(sendInfo.argName[1], "dwFlags");
        sprintf(sendInfo.argName[2], "lpMem");
        // 参数值
        sprintf(sendInfo.argValue[0], "%08X", (unsigned int)hHeap);
        sprintf(sendInfo.argValue[1], "%08X", dwFlags);
        sprintf(sendInfo.argValue[2], "%08X", dwBytes);
        sendInfo.type = HEAPALLOC;
        GetLocalTime(&(sendInfo.st));
        memcpy((info*)lpMap + cnt, &sendInfo, sizeof(info));
        cnt++;
    }
    return OldHeapAlloc(hHeap, dwFlags, dwBytes);
}

static LSTATUS(WINAPI* OldRegCreateKeyEx)(
    HKEY                        hKey,
    LPCWSTR                     lpSubKey,
    DWORD                       Reserved,
    LPWSTR                      lpClass,
    DWORD                       dwOptions,
    REGSAM                      samDesired,
    const LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY                       phkResult,
    LPDWORD                     lpdwDisposition) = RegCreateKeyEx;

extern "C" __declspec(dllexport)LSTATUS WINAPI NewRegCreateKeyEx(
    HKEY                        hKey,
    LPCWSTR                     lpSubKey,
    DWORD                       Reserved,
    LPWSTR                      lpClass,
    DWORD                       dwOptions,
    REGSAM                      samDesired,
    const LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY                       phkResult,
    LPDWORD                     lpdwDisposition
) {
    DWORD dwDisposition;
    if (lpdwDisposition == NULL)lpdwDisposition = &dwDisposition;
    LSTATUS ret = OldRegCreateKeyEx(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
    char temp[70];
    sendInfo.argNum = 9;
    // 参数名
    sprintf(sendInfo.argName[0], "hKey");
    sprintf(sendInfo.argName[1], "lpSubKey");
    sprintf(sendInfo.argName[2], "Reserved");
    sprintf(sendInfo.argName[3], "lpClass");
    sprintf(sendInfo.argName[4], "dwOptions");
    sprintf(sendInfo.argName[5], "samDesired");
    sprintf(sendInfo.argName[6], "lpSecurityAttributes");
    sprintf(sendInfo.argName[7], "phkResult");
    sprintf(sendInfo.argName[8], "lpdwDisposition");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", (unsigned int)hKey);
    // 宽字节转char
    memset(temp, 0, sizeof(temp));
    if (lpSubKey == NULL)strcpy(temp, "NULL");
    else WideCharToMultiByte(CP_ACP, 0, lpSubKey, wcslen(lpSubKey), temp, sizeof(temp), NULL, NULL);
    strcpy(sendInfo.argValue[1], temp);
    sprintf(sendInfo.argValue[2], "%08X", Reserved);
    sprintf(sendInfo.argValue[3], "%08X", (unsigned int)lpClass);
    sprintf(sendInfo.argValue[4], "%08X", dwOptions);
    sprintf(sendInfo.argValue[5], "%08X", samDesired);
    sprintf(sendInfo.argValue[6], "%08X", (unsigned int)lpSecurityAttributes);
    sprintf(sendInfo.argValue[7], "%08X", (unsigned int)phkResult);
    sprintf(sendInfo.argValue[8], "%08X", (unsigned int)*lpdwDisposition);

    sendInfo.type = REGCREATEKEYEX;
    GetLocalTime(&(sendInfo.st));
    memcpy((info*)lpMap + cnt, &sendInfo, sizeof(info));
    cnt++;
    return ret;
}

static LSTATUS(WINAPI* OldRegSetValueEx)(
    HKEY       hKey,
    LPCWSTR    lpValueName,
    DWORD      Reserved,
    DWORD      dwType,
    const BYTE* lpData,
    DWORD      cbData
    ) = RegSetValueEx;

extern "C" __declspec(dllexport)LSTATUS WINAPI NewRegSetValueEx(
    HKEY       hKey,
    LPCWSTR    lpValueName,
    DWORD      Reserved,
    DWORD      dwType,
    const BYTE * lpData,
    DWORD      cbData)
{
    char temp[70];
    sendInfo.argNum = 6;
    // 参数名
    sprintf(sendInfo.argName[0], "hKey");
    sprintf(sendInfo.argName[1], "lpValueName");
    sprintf(sendInfo.argName[2], "Reserved");
    sprintf(sendInfo.argName[3], "dwType");
    sprintf(sendInfo.argName[4], "lpData");
    sprintf(sendInfo.argName[5], "cbData");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", (unsigned int)hKey);
    // 宽字节转char
    std::string ValueName = GbkToUtf8(ConvertLPWSTRToLPSTR(lpValueName));
    strcpy(sendInfo.argValue[1], ValueName.c_str());
    sprintf(sendInfo.argValue[2], "%08X", Reserved);
    sprintf(sendInfo.argValue[3], "%08X", dwType);
    std::string Data = GbkToUtf8(ConvertLPWSTRToLPSTR((wchar_t*)lpData));
    strcpy(sendInfo.argValue[4], Data.c_str());
    sprintf(sendInfo.argValue[5], "%08X", cbData);


    sendInfo.type = REGSETVALUEEX;
    GetLocalTime(&(sendInfo.st));
    memcpy((info*)lpMap + cnt, &sendInfo, sizeof(info));
    cnt++;
    return OldRegSetValueEx(hKey, lpValueName, Reserved, dwType, lpData, cbData);
}

static LSTATUS(WINAPI* OldRegCloseKey)(HKEY hKey) = RegCloseKey;

extern "C" __declspec(dllexport)LSTATUS WINAPI NewRegCloseKey(HKEY hKey)
{
    sendInfo.argNum = 1;
    // 参数名
    sprintf(sendInfo.argName[0], "hKey");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", (unsigned int)hKey);
    sendInfo.type = REGCLOSEKEY;
    GetLocalTime(&(sendInfo.st));
    memcpy((info*)lpMap + cnt, &sendInfo, sizeof(info));
    cnt++;
    return OldRegCloseKey(hKey);
}

static LSTATUS(WINAPI* OldRegOpenKeyEx)(
    HKEY    hKey,
    LPCWSTR lpSubKey,
    DWORD   ulOptions,
    REGSAM  samDesired,
    PHKEY   phkResult
    ) = RegOpenKeyEx;
extern "C" __declspec(dllexport)LSTATUS WINAPI NewRegOpenKeyEx(
    HKEY    hKey,
    LPCWSTR lpSubKey,
    DWORD   ulOptions,
    REGSAM  samDesired,
    PHKEY   phkResult)
{
    char temp[70];
    sendInfo.argNum = 5;
    // 参数名
    sprintf(sendInfo.argName[0], "hKey");
    sprintf(sendInfo.argName[1], "lpSubKey");
    sprintf(sendInfo.argName[2], "ulOptions");
    sprintf(sendInfo.argName[3], "samDesired");
    sprintf(sendInfo.argName[4], "phkResult");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", (unsigned int)hKey);
    // 宽字节转char
    if (lpSubKey == NULL)strcpy(temp, "NULL");
    else {
        memset(temp, 0, sizeof(temp));
        WideCharToMultiByte(CP_ACP, 0, lpSubKey, wcslen(lpSubKey), temp, sizeof(temp), NULL, NULL);
        strcpy(sendInfo.argValue[1], temp);
    }
    sprintf(sendInfo.argValue[2], "%08X", ulOptions);
    sprintf(sendInfo.argValue[3], "%08X", samDesired);
    sprintf(sendInfo.argValue[4], "%08X", (unsigned int)phkResult);

    sendInfo.type = REGOPENKEYEX;
    GetLocalTime(&(sendInfo.st));
    memcpy((info*)lpMap + cnt, &sendInfo, sizeof(info));
    cnt++;
    return OldRegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired, phkResult);
}

static LSTATUS(WINAPI* OldRegQueryValueEx)(
    HKEY    hKey,					//已打开的键的句柄
    LPCWSTR  lpValueName,			//键值名称，如果为NULL或“”,则设置键值名为“默认”的项
    LPDWORD lpReserved,			//保留，必须为0
    LPDWORD lpType,				//【出参】，返回数据类型
    LPBYTE  lpData,				//【出参】, 返回数据
    LPDWORD lpcbData				//【出参】, 返回数据长度
    ) = RegQueryValueEx;

extern "C" __declspec(dllexport)LSTATUS WINAPI NewRegQueryValueEx(
    HKEY    hKey,					//已打开的键的句柄
    LPCWSTR  lpValueName,			//键值名称，如果为NULL或“”,则设置键值名为“默认”的项
    LPDWORD lpReserved,			//保留，必须为0
    LPDWORD lpType,				//【出参】，返回数据类型
    LPBYTE  lpData,				//【出参】, 返回数据
    LPDWORD lpcbData				//【出参】, 返回数据长度
)
{
    LSTATUS ret = OldRegQueryValueEx(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
    char temp[70];
    sendInfo.argNum = 6;
    // 参数名
    sprintf(sendInfo.argName[0], "hKey");
    sprintf(sendInfo.argName[1], "lpValueName");
    sprintf(sendInfo.argName[2], "lpReserved");
    sprintf(sendInfo.argName[3], "lpType");
    sprintf(sendInfo.argName[4], "lpData");
    sprintf(sendInfo.argName[5], "lpcbData");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", (unsigned int)hKey);
    // 宽字节转char
    memset(temp, 0, sizeof(temp));
    std::string ValueName = GbkToUtf8(ConvertLPWSTRToLPSTR(lpValueName));
    strcpy(sendInfo.argValue[1], ValueName.c_str());
    sprintf(sendInfo.argValue[2], "%08X", (unsigned int)lpReserved);
    sprintf(sendInfo.argValue[3], "%08X", (unsigned int)lpType);
    std::string Data = GbkToUtf8(ConvertLPWSTRToLPSTR((wchar_t*)lpData));
    strcpy(sendInfo.argValue[4], Data.c_str());
    sprintf(sendInfo.argValue[5], "%08X", (unsigned int)lpcbData);

    sendInfo.type = REGQUERYVALUEEX;
    GetLocalTime(&(sendInfo.st));
    memcpy((info*)lpMap + cnt, &sendInfo, sizeof(info));
    cnt++;
    return ret;
}

static LSTATUS(WINAPI* OldRegDeleteValue)(
    HKEY    hKey,
    LPCWSTR lpValueName
    ) = RegDeleteValue;

extern "C" __declspec(dllexport)LSTATUS WINAPI NewRegDeleteValue(
    HKEY    hKey,
    LPCWSTR lpValueName)
{
    char temp[70];
    sendInfo.argNum = 2;
    // 参数名
    sprintf(sendInfo.argName[0], "hKey");
    sprintf(sendInfo.argName[1], "lpValueName");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", (unsigned int)hKey);
    // 宽字节转char
    memset(temp, 0, sizeof(temp));
    WideCharToMultiByte(CP_ACP, 0, lpValueName, wcslen(lpValueName), temp, sizeof(temp), NULL, NULL);
    strcpy(sendInfo.argValue[1], temp);
    sendInfo.type = REGDELETEVALUE;
    GetLocalTime(&(sendInfo.st));
    memcpy((info*)lpMap + cnt, &sendInfo, sizeof(info));
    cnt++;
    return OldRegDeleteValue(hKey, lpValueName);
}

static LSTATUS(WINAPI* OldRegDeleteKeyEx)(
    HKEY hKey,
    LPCWSTR lpSubKey,
    REGSAM samDesired,
    DWORD Reserved
    ) = RegDeleteKeyEx;

extern "C" __declspec(dllexport)LSTATUS WINAPI NewRegDeleteKeyEx(
    HKEY hKey,
    LPCWSTR lpSubKey,
    REGSAM samDesired,
    DWORD Reserved)
{
    char temp[70];
    sendInfo.argNum = 4;
    // 参数名
    sprintf(sendInfo.argName[0], "hKey");
    sprintf(sendInfo.argName[1], "lpSubKey");
    sprintf(sendInfo.argName[2], "samDesired");
    sprintf(sendInfo.argName[3], "Reserved");
    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", (unsigned int)hKey);
    // 宽字节转char
    memset(temp, 0, sizeof(temp));
    WideCharToMultiByte(CP_ACP, 0, lpSubKey, wcslen(lpSubKey), temp, sizeof(temp), NULL, NULL);
    strcpy(sendInfo.argValue[1], temp);
    sprintf(sendInfo.argValue[2], "%08X", samDesired);
    sprintf(sendInfo.argValue[3], "%08X", (unsigned int)Reserved);

    sendInfo.type = REGDELETEKEYEX;
    GetLocalTime(&(sendInfo.st));
    memcpy((info*)lpMap + cnt, &sendInfo, sizeof(info));
    cnt++;
    return OldRegDeleteKeyEx(hKey, lpSubKey, samDesired, Reserved);
}

static SOCKET(WINAPI* Oldsocket)(
    _In_ int af,
    _In_ int type,
    _In_ int protocol
    ) = socket;

extern "C" __declspec(dllexport)SOCKET WINAPI Newsocket(
    int af,
    int type,
    int protocol)
{
    char temp[70];
    memset(temp, 0, sizeof(temp));
    sendInfo.argNum = 3;
    // 参数名
    sprintf(sendInfo.argName[0], "af");
    sprintf(sendInfo.argName[1], "type");
    sprintf(sendInfo.argName[2], "protocol");
    // 参数值
    if (protocol == 6)strcpy(temp, "TCP");
    else if (protocol == 17)strcpy(temp, "UDP");
    else strcpy(temp, "Others");
    sprintf(sendInfo.argValue[0], "%08X", af);
    sprintf(sendInfo.argValue[1], "%08X", type);
    strcpy(sendInfo.argValue[2], temp);

    sendInfo.type = THESOCKET;
    GetLocalTime(&(sendInfo.st));
    memcpy((info*)lpMap + cnt, &sendInfo, sizeof(info));
    cnt++;
    return Oldsocket(af, type, protocol);
}

static int(WINAPI* Oldbind)(
    _In_ SOCKET s,
    _In_reads_bytes_(namelen) const struct sockaddr FAR* name,
    _In_ int namelen
    ) = bind;

extern "C" __declspec(dllexport)SOCKET WINAPI Newbind(
    SOCKET s,
    const struct sockaddr FAR * name,
    int namelen)
{
    char temp[70];
    sendInfo.argNum = 4;
    // 参数名
    sprintf(sendInfo.argName[0], "s");
    sprintf(sendInfo.argName[1], "port");
    sprintf(sendInfo.argName[2], "IP");
    sprintf(sendInfo.argName[3], "namelen");

    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", s);
    sprintf(sendInfo.argValue[1], "%d", ntohs(((sockaddr_in*)name)->sin_port));
    strcpy(sendInfo.argValue[2], inet_ntoa(((sockaddr_in*)name)->sin_addr));
    sprintf(sendInfo.argValue[3], "%d", namelen);

    sendInfo.type = BIND;
    GetLocalTime(&(sendInfo.st));
    memcpy((info*)lpMap + cnt, &sendInfo, sizeof(info));
    cnt++;
    return Oldbind(s, name, namelen);
}


static int(WINAPI* Oldsend)(
    _In_ SOCKET s,
    _In_reads_bytes_(len) const char FAR* buf,
    _In_ int len,
    _In_ int flags
    ) = send;

extern "C" __declspec(dllexport)SOCKET WINAPI Newsend(
    SOCKET s,
    const char FAR * buf,
    int len,
    int flags)
{
    struct sockaddr_in addr;
    int addr_len = sizeof(addr);
    getpeername(s, (struct sockaddr*)&addr, &addr_len);
    char* ip = inet_ntoa(addr.sin_addr);
    //获取port
    int port = ntohs(addr.sin_port);
    char temp[70];
    sendInfo.argNum = 6;
    // 参数名
    sprintf(sendInfo.argName[0], "s");
    sprintf(sendInfo.argName[1], "buf");
    sprintf(sendInfo.argName[2], "len");
    sprintf(sendInfo.argName[3], "flags");
    sprintf(sendInfo.argName[4], "port");
    sprintf(sendInfo.argName[5], "IP");

    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", s);
    strcpy(sendInfo.argValue[1],buf);
    sprintf(sendInfo.argValue[2], "%d", len);
    sprintf(sendInfo.argValue[3], "%08X", flags);
    sprintf(sendInfo.argValue[4], "%d", port);
    strcpy(sendInfo.argValue[5], ip);

    MD5 md5;
    md5.reset();
    md5.update(buf);
    std::string MD5temp = md5.toString();
    strcpy(sendInfo.MD5_Result, MD5temp.c_str());

    sendInfo.type = SEND;
    GetLocalTime(&(sendInfo.st));
    memcpy((info*)lpMap + cnt, &sendInfo, sizeof(info));
    cnt++;
    return Oldsend(s, buf, len, flags);
}


static int(WINAPI* Oldconnect)(
    _In_ SOCKET s,
    _In_reads_bytes_(namelen) const struct sockaddr FAR* name,
    _In_ int namelen
    ) = connect;

extern "C" __declspec(dllexport)SOCKET WINAPI Newconnect(
    SOCKET s,
    const struct sockaddr FAR * name,
    int namelen)
{
    char temp[70];
    sendInfo.argNum = 4;
    // 参数名
    sprintf(sendInfo.argName[0], "s");
    sprintf(sendInfo.argName[1], "port");
    sprintf(sendInfo.argName[2], "IP");
    sprintf(sendInfo.argName[3], "namelen");

    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", s);
    sprintf(sendInfo.argValue[1], "%d", ntohs(((sockaddr_in*)name)->sin_port));
    strcpy(sendInfo.argValue[2], inet_ntoa(((sockaddr_in*)name)->sin_addr));
    sprintf(sendInfo.argValue[3], "%d", namelen);

    sendInfo.type = CONNECT;
    GetLocalTime(&(sendInfo.st));
    memcpy((info*)lpMap + cnt, &sendInfo, sizeof(info));
    cnt++;
    return Oldconnect(s, name, namelen);
}

static int(WINAPI* Oldrecv)(
    _In_ SOCKET s,
    _Out_writes_bytes_to_(len, return) __out_data_source(NETWORK) char FAR* buf,
    _In_ int len,
    _In_ int flags
    ) = recv;

extern "C" __declspec(dllexport)SOCKET WINAPI Newrecv(
    SOCKET s,
    char FAR* buf,
    int len,
    int flags)
{
    int ret = Oldrecv(s, buf, len, flags);
    std::string buf_tmp(buf, ret);
    struct sockaddr_in addr;
    int addr_len = sizeof(addr);
    getpeername(s, (struct sockaddr*)&addr, &addr_len);
    char* ip = inet_ntoa(addr.sin_addr);
    //获取port
    int port = ntohs(addr.sin_port);

    char temp[70];
    sendInfo.argNum = 6;
    // 参数名
    sprintf(sendInfo.argName[0], "s");
    sprintf(sendInfo.argName[1], "buf");
    sprintf(sendInfo.argName[2], "len");
    sprintf(sendInfo.argName[3], "flags");
    sprintf(sendInfo.argName[4], "port");
    sprintf(sendInfo.argName[5], "IP");

    // 参数值
    sprintf(sendInfo.argValue[0], "%08X", s);
    strcpy(sendInfo.argValue[1], buf_tmp.c_str());
    sprintf(sendInfo.argValue[2], "%d", len);
    sprintf(sendInfo.argValue[3], "%08X", flags);
    sprintf(sendInfo.argValue[4], "%d", port);
    strcpy(sendInfo.argValue[5], ip);

    // MessageBoxA(NULL, "111", "111", MB_OK);

    sendInfo.type = RECV;
    GetLocalTime(&(sendInfo.st));
     memcpy((info*)lpMap + cnt, &sendInfo, sizeof(info));
     cnt++;
    return ret;
}

BOOL WINAPI DllMain(HMODULE hModule,
    DWORD ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        DisableThreadLibraryCalls(hModule);
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)OldMessageBoxW, NewMessageBoxW);
        DetourAttach(&(PVOID&)OldMessageBoxA, NewMessageBoxA);
        DetourAttach(&(PVOID&)OldCreateFile, NewCreateFile);
        DetourAttach(&(PVOID&)OldWriteFile, NewWriteFile);
        DetourAttach(&(PVOID&)OldReadFile, NewReadFile);
        DetourAttach(&(PVOID&)OldHeapCreate, NewHeapCreate);
        DetourAttach(&(PVOID&)OldHeapDestory, NewHeapDestory);
        DetourAttach(&(PVOID&)OldHeapFree, NewHeapFree);
        DetourAttach(&(PVOID&)OldHeapAlloc, NewHeapAlloc);
        DetourAttach(&(PVOID&)OldRegCreateKeyEx, NewRegCreateKeyEx);
        DetourAttach(&(PVOID&)OldRegSetValueEx, NewRegSetValueEx);
        DetourAttach(&(PVOID&)OldRegDeleteValue, NewRegDeleteValue);
        DetourAttach(&(PVOID&)OldRegCloseKey, NewRegCloseKey);
        DetourAttach(&(PVOID&)OldRegQueryValueEx, NewRegQueryValueEx);
        DetourAttach(&(PVOID&)OldRegOpenKeyEx, NewRegOpenKeyEx);
        DetourAttach(&(PVOID&)OldRegDeleteKeyEx, NewRegDeleteKeyEx);
        DetourAttach(&(PVOID&)Oldsocket, Newsocket);
        DetourAttach(&(PVOID&)Oldbind, Newbind);
        DetourAttach(&(PVOID&)Oldsend, Newsend);
        DetourAttach(&(PVOID&)Oldconnect, Newconnect);
        DetourAttach(&(PVOID&)Oldrecv, Newrecv);
        //DetourAttach(&(PVOID&)Oldmemcpy, Newmemcpy);
        DetourTransactionCommit();
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)OldMessageBoxW, NewMessageBoxW);
        DetourDetach(&(PVOID&)OldMessageBoxA, NewMessageBoxA);
        DetourDetach(&(PVOID&)OldCreateFile, NewCreateFile);
        DetourDetach(&(PVOID&)OldWriteFile, NewWriteFile);
        DetourDetach(&(PVOID&)OldReadFile, NewReadFile);
        DetourDetach(&(PVOID&)OldHeapCreate, NewHeapCreate);
        DetourDetach(&(PVOID&)OldHeapDestory, NewHeapDestory);
        DetourDetach(&(PVOID&)OldHeapAlloc, NewHeapAlloc);
        DetourDetach(&(PVOID&)OldHeapFree, NewHeapFree);
        DetourDetach(&(PVOID&)OldRegCreateKeyEx, NewRegCreateKeyEx);
        DetourDetach(&(PVOID&)OldRegSetValueEx, NewRegSetValueEx);
        DetourDetach(&(PVOID&)OldRegDeleteValue, NewRegDeleteValue);
        DetourDetach(&(PVOID&)OldRegCloseKey, NewRegCloseKey);
        DetourDetach(&(PVOID&)OldRegQueryValueEx, NewRegQueryValueEx);
        DetourDetach(&(PVOID&)OldRegOpenKeyEx, NewRegOpenKeyEx);
        DetourDetach(&(PVOID&)OldRegDeleteKeyEx, NewRegDeleteKeyEx);
        DetourDetach(&(PVOID&)Oldsocket, Newsocket);
        DetourDetach(&(PVOID&)Oldbind, Newbind);
        DetourDetach(&(PVOID&)Oldsend, Newsend);
        DetourDetach(&(PVOID&)Oldconnect, Newconnect);
        DetourDetach(&(PVOID&)Oldrecv, Newrecv);
                //DetourDetach(&(PVOID&)Oldmemcpy, Newmemcpy);
        DetourTransactionCommit();
        //		UnmapViewOfFile(lpBase);
        //		CloseHandle(hMapFile);
        break;
    }
    }
    return true;
}