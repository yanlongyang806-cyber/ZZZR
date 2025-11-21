// AutoLoadLua_Final.cpp - 最终版DLL注入自动加载Lua脚本
// 编译: cl /LD /EHsc AutoLoadLua_Final.cpp /link /OUT:AutoLoadLua.dll kernel32.lib psapi.lib

#include <windows.h>
#include <string>
#include <thread>
#include <chrono>
#include <psapi.h>

// 全局变量
HMODULE g_hGameServer = NULL;
HMODULE g_hModule = NULL;

// 尝试通过多种方法加载Lua脚本
void TryLoadLuaScript()
{
    OutputDebugStringA("========================================\n");
    OutputDebugStringA("[AutoLoadLua] 开始尝试加载 Lua 脚本\n");
    OutputDebugStringA("========================================\n");
    
    const char* scriptPath = "C:/Night/data/server/TestServer/scripts/LoadPVP.lua";
    
    // 方法1: 尝试查找并调用 TestServer_RunScript
    typedef void (*TestServer_RunScriptFunc)(const char*);
    TestServer_RunScriptFunc pRunScript = NULL;
    
    // 尝试多个可能的函数名（包括C++修饰名）
    const char* funcNames[] = {
        "TestServer_RunScript",           // 标准名称
        "_TestServer_RunScript",          // C风格
        "?TestServer_RunScript@@YAXPBD@Z", // C++修饰名（void TestServer_RunScript(const char*)）
        "TestServer_RunScriptRaw",        // Raw版本
        "_TestServer_RunScriptRaw",
        "RunScript",
        "_RunScript"
    };
    
    for (int i = 0; i < sizeof(funcNames) / sizeof(funcNames[0]); i++)
    {
        pRunScript = (TestServer_RunScriptFunc)GetProcAddress(g_hGameServer, funcNames[i]);
        if (pRunScript)
        {
            char msg[512];
            sprintf_s(msg, sizeof(msg), "[AutoLoadLua] [OK] 找到函数: %s (地址: 0x%p)\n", funcNames[i], pRunScript);
            OutputDebugStringA(msg);
            
            OutputDebugStringA("[AutoLoadLua] 正在加载脚本: LoadPVP.lua\n");
            
            __try {
                pRunScript(scriptPath);
                OutputDebugStringA("[AutoLoadLua] [成功] 脚本加载完成\n");
                OutputDebugStringA("========================================\n");
                return;
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                char err[256];
                sprintf_s(err, sizeof(err), "[AutoLoadLua] [异常] 调用函数时发生异常: 0x%08X\n", GetExceptionCode());
                OutputDebugStringA(err);
            }
        }
    }
    
    // 方法2: 尝试通过 CmdParseTextCommand（如果可用）
    typedef int (*CmdParseTextCommandFunc)(const char*, void*);
    CmdParseTextCommandFunc pCmdParse = NULL;
    
    const char* cmdFuncNames[] = {
        "CmdParseTextCommand",
        "_CmdParseTextCommand"
    };
    
    for (int i = 0; i < sizeof(cmdFuncNames) / sizeof(cmdFuncNames[0]); i++)
    {
        pCmdParse = (CmdParseTextCommandFunc)GetProcAddress(g_hGameServer, cmdFuncNames[i]);
        if (pCmdParse)
        {
            char msg[256];
            sprintf_s(msg, sizeof(msg), "[AutoLoadLua] [OK] 找到命令解析函数: %s (地址: 0x%p)\n", cmdFuncNames[i], pCmdParse);
            OutputDebugStringA(msg);
            
            const char* cmd = "dofile('data/server/TestServer/scripts/LoadPVP.lua')";
            OutputDebugStringA("[AutoLoadLua] 正在执行命令: dofile(...)\n");
            
            __try {
                int result = pCmdParse(cmd, NULL);
                char msg2[256];
                sprintf_s(msg2, sizeof(msg2), "[AutoLoadLua] [成功] 命令执行完成 (返回值: %d)\n", result);
                OutputDebugStringA(msg2);
                OutputDebugStringA("========================================\n");
                return;
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                char err[256];
                sprintf_s(err, sizeof(err), "[AutoLoadLua] [异常] 执行命令时发生异常: 0x%08X\n", GetExceptionCode());
                OutputDebugStringA(err);
            }
        }
    }
    
    // 方法3: 尝试通过符号查找（需要PDB文件或符号表）
    // 这需要更复杂的实现，暂时跳过
    
    OutputDebugStringA("[AutoLoadLua] [警告] 未找到可用的函数\n");
    OutputDebugStringA("[AutoLoadLua] 建议: 修改源码添加自动加载功能\n");
    OutputDebugStringA("========================================\n");
}

// 列出所有导出函数（用于调试）
void ListExportedFunctions()
{
    OutputDebugStringA("[AutoLoadLua] 正在列出 GameServer.exe 的导出函数...\n");
    
    // 获取模块信息
    MODULEINFO modInfo;
    if (!GetModuleInformation(GetCurrentProcess(), g_hGameServer, &modInfo, sizeof(modInfo)))
    {
        OutputDebugStringA("[AutoLoadLua] 无法获取模块信息\n");
        return;
    }
    
    // 读取PE头
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)modInfo.lpBaseOfDll;
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
    {
        OutputDebugStringA("[AutoLoadLua] 无效的PE文件\n");
        return;
    }
    
    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)pDosHeader + pDosHeader->e_lfanew);
    if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE)
    {
        OutputDebugStringA("[AutoLoadLua] 无效的NT头\n");
        return;
    }
    
    // 查找导出表
    DWORD exportRVA = pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if (exportRVA == 0)
    {
        OutputDebugStringA("[AutoLoadLua] 模块没有导出表（可能是静态链接）\n");
        return;
    }
    
    PIMAGE_EXPORT_DIRECTORY pExportDir = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)pDosHeader + exportRVA);
    DWORD* pFunctions = (DWORD*)((BYTE*)pDosHeader + pExportDir->AddressOfFunctions);
    DWORD* pNames = (DWORD*)((BYTE*)pDosHeader + pExportDir->AddressOfNames);
    WORD* pOrdinals = (WORD*)((BYTE*)pDosHeader + pExportDir->AddressOfNameOrdinals);
    
    char msg[512];
    sprintf_s(msg, sizeof(msg), "[AutoLoadLua] 找到 %d 个导出函数\n", pExportDir->NumberOfFunctions);
    OutputDebugStringA(msg);
    
    // 列出前20个函数名（用于调试）
    int count = min(20, (int)pExportDir->NumberOfNames);
    for (int i = 0; i < count; i++)
    {
        const char* funcName = (const char*)((BYTE*)pDosHeader + pNames[i]);
        sprintf_s(msg, sizeof(msg), "[AutoLoadLua]   导出函数 %d: %s\n", i, funcName);
        OutputDebugStringA(msg);
    }
}

// 工作线程：等待GameServer初始化后加载脚本
DWORD WINAPI LoadScriptThread(LPVOID lpParam)
{
    OutputDebugStringA("[AutoLoadLua] 工作线程已启动\n");
    
    // 等待GameServer完全启动
    OutputDebugStringA("[AutoLoadLua] 等待 GameServer 初始化...\n");
    std::this_thread::sleep_for(std::chrono::seconds(15));
    
    // 尝试获取GameServer模块句柄
    g_hGameServer = GetModuleHandleA("GameServer.exe");
    if (!g_hGameServer)
    {
        OutputDebugStringA("[AutoLoadLua] 等待 GameServer.exe 模块加载...\n");
        for (int i = 0; i < 60; i++)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            g_hGameServer = GetModuleHandleA("GameServer.exe");
            if (g_hGameServer)
            {
                char msg[256];
                sprintf_s(msg, sizeof(msg), "[AutoLoadLua] [OK] 找到 GameServer.exe 模块 (0x%p)\n", g_hGameServer);
                OutputDebugStringA(msg);
                break;
            }
        }
    }
    else
    {
        char msg[256];
        sprintf_s(msg, sizeof(msg), "[AutoLoadLua] [OK] GameServer.exe 模块已加载 (0x%p)\n", g_hGameServer);
        OutputDebugStringA(msg);
    }
    
    if (!g_hGameServer)
    {
        OutputDebugStringA("[AutoLoadLua] [错误] 无法找到 GameServer.exe 模块\n");
        return 1;
    }
    
    // 获取模块信息
    MODULEINFO modInfo;
    if (GetModuleInformation(GetCurrentProcess(), g_hGameServer, &modInfo, sizeof(modInfo)))
    {
        char msg[256];
        sprintf_s(msg, sizeof(msg), "[AutoLoadLua] 模块基址: 0x%p, 大小: 0x%X\n", 
                  modInfo.lpBaseOfDll, modInfo.SizeOfImage);
        OutputDebugStringA(msg);
    }
    
    // 列出导出函数（用于调试）
    ListExportedFunctions();
    
    // 再等待一下，确保Lua环境完全初始化
    OutputDebugStringA("[AutoLoadLua] 等待 Lua 环境初始化...\n");
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // 尝试加载脚本
    TryLoadLuaScript();
    
    return 0;
}

// DLL入口点
BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);
        
        OutputDebugStringA("========================================\n");
        OutputDebugStringA("[AutoLoadLua] DLL 已加载\n");
        OutputDebugStringA("========================================\n");
        
        // 创建线程来加载脚本
        HANDLE hThread = CreateThread(NULL, 0, LoadScriptThread, NULL, 0, NULL);
        if (hThread)
        {
            CloseHandle(hThread);
            OutputDebugStringA("[AutoLoadLua] 工作线程已创建\n");
        }
        else
        {
            char msg[256];
            sprintf_s(msg, sizeof(msg), "[AutoLoadLua] [错误] 无法创建线程 (错误代码: %d)\n", GetLastError());
            OutputDebugStringA(msg);
        }
        break;
    }
        
    case DLL_PROCESS_DETACH:
    {
        OutputDebugStringA("[AutoLoadLua] DLL 已卸载\n");
        break;
    }
    }
    
    return TRUE;
}

