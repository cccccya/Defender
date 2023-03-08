// FileOperation.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <windows.h>

void read(const wchar_t* strPath)
{
    HANDLE hFile = ::CreateFile(strPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD nFileSize = GetFileSize(hFile, NULL);
        char* lpBuffer = new char[nFileSize+1];
        memset(lpBuffer, 0, nFileSize +1);
        DWORD nNumberOfBytesRead;
        if(ReadFile(hFile, lpBuffer, nFileSize, &nNumberOfBytesRead, NULL))
        // TODO ...
        //std::cout << nFileSize<<" "<<nNumberOfBytesRead << std::endl;
        //std::cout << lpBuffer << std::endl;
        std::wcout << lpBuffer << std::endl;
        //std::cout << nFileSize<<" "<<nNumberOfBytesRead << std::endl;
        delete[] lpBuffer;
        CloseHandle(hFile);
    }
}
void write(const wchar_t* strPath, const char* strContent, const int nContentSize)
{
    HANDLE hFile = ::CreateFile(strPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, NULL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD nNumberOfBytesWritten;
        WriteFile(hFile, strContent, nContentSize, &nNumberOfBytesWritten, NULL);

        // TODO ...
        CloseHandle(hFile);
    }
}
wchar_t* StringToLPCWSTR(std::string str)
{
    size_t size = str.length();
    int wLen = ::MultiByteToWideChar(CP_UTF8,
        0,
        str.c_str(),
        -1,
        NULL,
        0);
    wchar_t* buffer = new wchar_t[wLen + 1];
    memset(buffer, 0, (wLen + 1) * sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), size, (LPWSTR)buffer, wLen);
    return buffer;
}
wchar_t FilePath1[100] = L"G:\\Defender\\example\\FileOperationFile.in";
wchar_t FilePath2[100] = L"G:\\Defender\\example\\FileOperation\\FileOperationFile.out";
wchar_t FilePath3[100] = L"G:\\Defender\\example\\my.exe";

std::string content1 = "1Hello World!";
std::string content2 = "2Hello World!";
int main()
{
    write(FilePath1, content1.c_str(), strlen(content1.c_str()));
    read(FilePath1);
    write(FilePath2, content2.c_str(), strlen(content2.c_str()));
    read(FilePath2);
    write(FilePath3, content1.c_str(), strlen(content1.c_str()));

}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
