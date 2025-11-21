// Injector.cpp - 简单的DLL注入工具
// 编译: cl /EHsc Injector.cpp /link /OUT:Injector.exe

#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>

// 查找进程ID
DWORD FindProcessId(const char* processName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return 0;
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    if (Process32First(hSnapshot, &pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile, processName) == 0)
            {
                CloseHandle(hSnapshot);
                return pe32.th32ProcessID;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    return 0;
}

// 注入DLL到进程
bool InjectDLL(DWORD processId, const char* dllPath)
{
    // 打开目标进程
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!hProcess)
    {
        std::cerr << "[错误] 无法打开进程 (错误代码: " << GetLastError() << ")" << std::endl;
        return false;
    }
    
    // 在目标进程中分配内存
    size_t pathLen = strlen(dllPath) + 1;
    LPVOID pRemoteMemory = VirtualAllocEx(hProcess, NULL, pathLen, MEM_COMMIT, PAGE_READWRITE);
    if (!pRemoteMemory)
    {
        std::cerr << "[错误] 无法在目标进程中分配内存" << std::endl;
        CloseHandle(hProcess);
        return false;
    }
    
    // 写入DLL路径
    if (!WriteProcessMemory(hProcess, pRemoteMemory, dllPath, pathLen, NULL))
    {
        std::cerr << "[错误] 无法写入内存" << std::endl;
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }
    
    // 获取LoadLibraryA函数地址
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    LPTHREAD_START_ROUTINE pLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryA");
    
    // 在目标进程中创建远程线程
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pLoadLibrary, pRemoteMemory, 0, NULL);
    if (!hThread)
    {
        std::cerr << "[错误] 无法创建远程线程 (错误代码: " << GetLastError() << ")" << std::endl;
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }
    
    // 等待线程完成
    WaitForSingleObject(hThread, INFINITE);
    
    // 检查结果
    DWORD exitCode;
    GetExitCodeThread(hThread, &exitCode);
    
    // 清理
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    
    if (exitCode == 0)
    {
        std::cerr << "[错误] LoadLibrary 返回 NULL" << std::endl;
        return false;
    }
    
    return true;
}

int main(int argc, char* argv[])
{
    std::cout << "========================================" << std::endl;
    std::cout << "DLL注入工具 - AutoLoadLua" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    if (argc < 3)
    {
        std::cout << "用法: Injector.exe <进程名> <DLL路径>" << std::endl;
        std::cout << "示例: Injector.exe GameServer.exe AutoLoadLua.dll" << std::endl;
        std::cout << std::endl;
        return 1;
    }
    
    const char* processName = argv[1];
    const char* dllPath = argv[2];
    
    // 转换为绝对路径
    char fullPath[MAX_PATH];
    if (!GetFullPathNameA(dllPath, MAX_PATH, fullPath, NULL))
    {
        std::cerr << "[错误] 无法获取DLL的绝对路径" << std::endl;
        return 1;
    }
    
    std::cout << "查找进程: " << processName << std::endl;
    DWORD processId = FindProcessId(processName);
    
    if (processId == 0)
    {
        std::cerr << "[错误] 未找到进程: " << processName << std::endl;
        std::cout << "提示: 请确保 GameServer.exe 正在运行" << std::endl;
        return 1;
    }
    
    std::cout << "[OK] 找到进程 ID: " << processId << std::endl;
    std::cout << "DLL路径: " << fullPath << std::endl;
    std::cout << std::endl;
    
    std::cout << "正在注入DLL..." << std::endl;
    if (InjectDLL(processId, fullPath))
    {
        std::cout << "[成功] DLL注入完成！" << std::endl;
        std::cout << std::endl;
        std::cout << "提示: 使用 DebugView 查看 DLL 的日志输出" << std::endl;
        return 0;
    }
    else
    {
        std::cerr << "[失败] DLL注入失败" << std::endl;
        return 1;
    }
}

