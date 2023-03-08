// RegOperation.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <windows.h>

int main()
{
    HKEY hReg=NULL; LPDWORD disposition=0;
    size_t result = RegCreateKeyEx(HKEY_CURRENT_USER, L"cccya", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hReg, NULL);
    if (result == ERROR_SUCCESS) {
        std::cout << "succussfully create"<<std::endl;
    }
    else {
        std::cout << "failed to create"<<std::endl;
    }
    wchar_t data[200] = L"U202012039";
    result = RegSetValueEx(hReg, L"My ID", 0, REG_SZ, (const BYTE*)data, wcslen(data)*sizeof(wchar_t));
    if (result == ERROR_SUCCESS){
        std::cout << "successfully set value" << std::endl;
    }
    else {
        std::cout << "failed to set value" << std::endl;
    }
    RegCloseKey(hReg);
    result = RegOpenKeyEx(HKEY_CURRENT_USER, L"cccya", 0, KEY_ALL_ACCESS, &hReg);
    if (result == ERROR_SUCCESS) {
        std::cout << "successfully open" << std::endl;
    }
    else {
        std::cout << "failed to open" << std::endl;
    }
    wchar_t receivedata[100];
    DWORD Type,Len;
    result = RegQueryValueEx(hReg, L"My ID", 0, &Type, (BYTE*)receivedata, &Len);
    wprintf(L"%s\n", receivedata);
    std::cout <<  "Len:" << Len << std::endl;
    if (result == ERROR_SUCCESS) {
        std::cout << "successfully Query Value" << std::endl;
    }
    else {
        std::cout << "failed to Query Value" << std::endl;
    }
    result = RegDeleteValue(hReg,L"My ID");
    if (result == ERROR_SUCCESS) {
        std::cout << "successfully delete value" << std::endl;
    }
    else {
        std::cout << "failed to delete value" << std::endl;
    }
    RegCloseKey(hReg);
    
    result = RegOpenKeyEx(HKEY_CURRENT_USER, NULL, 0, KEY_ALL_ACCESS, &hReg);
    if (result == ERROR_SUCCESS) {
        std::cout << "successfully open" << std::endl;
    }
    else {
        std::cout << "failed to open" << std::endl;
    }

    result = RegDeleteKeyEx(hReg, L"cccya", KEY_WOW64_32KEY,0);
    if (result == ERROR_SUCCESS) {
        std::cout << "successfully delete key" << std::endl;
    }
    else {
        std::cout << "failed to delete key" << std::endl;
    }
    /*result = RegDeleteValue(hReg, L"My ID");
    if (result == ERROR_SUCCESS) {
        std::cout << "successfully delete value" << std::endl;
    }
    else {
        std::cout << "failed to delete value" << std::endl;
    }*/
    RegCloseKey(hReg);

    result = RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hReg, NULL);
    if (result == ERROR_SUCCESS) {
        std::cout << "succussfully create" << std::endl;
    }
    else {
        std::cout << "failed to create" << std::endl;
    }
    wchar_t Data[200] = L"U202012039";
    result = RegSetValueEx(hReg, L"My ID", 0, REG_SZ, (const BYTE*)Data, wcslen(Data) * sizeof(wchar_t));
    if (result == ERROR_SUCCESS) {
        std::cout << "successfully set value" << std::endl;
    }
    else {
        std::cout << "failed to set value" << std::endl;
    }
    result = RegDeleteValue(hReg, L"My ID");
    if (result == ERROR_SUCCESS) {
        std::cout << "successfully delete value" << std::endl;
    }
    else {
        std::cout << "failed to delete value" << std::endl;
    }

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
