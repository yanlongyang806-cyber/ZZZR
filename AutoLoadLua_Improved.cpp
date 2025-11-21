// AutoLoadLua_Improved.cpp - 改进版DLL注入自动加载Lua脚本
// 编译: cl /LD /EHsc AutoLoadLua_Improved.cpp /link /OUT:AutoLoadLua.dll kernel32.lib

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
    
    // 方法1: 尝试查找并调用 TestServer_RunScript
    typedef void (*TestServer_RunScriptFunc)(const char*);
    TestServer_RunScriptFunc pRunScript = NULL;
    
    // 尝试多个可能的函数名
    const char* funcNames[] = {
        "TestServer_RunScript",
        "_TestServer_RunScript",
        "TestServer_RunScriptRaw",
        "_TestServer_RunScriptRaw",
        "RunScript",
        "_RunScript"
    };
    
    for (int i = 0; i < sizeof(funcNames) / sizeof(funcNames[0]); i++)
    {
        pRunScript = (TestServer_RunScriptFunc)GetProcAddress(g_hGameServer, funcNames[i]);
        if (pRunScript)
        {
            char msg[256];
            sprintf_s(msg, sizeof(msg), "[AutoLoadLua] [OK] 找到函数: %s\n", funcNames[i]);
            OutputDebugStringA(msg);
            
            const char* script = "data/server/TestServer/scripts/LoadPVP.lua";
            OutputDebugStringA("[AutoLoadLua] 正在加载脚本: LoadPVP.lua\n");
            
            __try {
                pRunScript(script);
                OutputDebugStringA("[AutoLoadLua] [成功] 脚本加载完成\n");
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
            sprintf_s(msg, sizeof(msg), "[AutoLoadLua] [OK] 找到命令解析函数: %s\n", cmdFuncNames[i]);
            OutputDebugStringA(msg);
            
            const char* cmd = "dofile('data/server/TestServer/scripts/LoadPVP.lua')";
            OutputDebugStringA("[AutoLoadLua] 正在执行命令: dofile(...)\n");
            
            __try {
                pCmdParse(cmd, NULL);
                OutputDebugStringA("[AutoLoadLua] [成功] 命令执行完成\n");
                return;
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                char err[256];
                sprintf_s(err, sizeof(err), "[AutoLoadLua] [异常] 执行命令时发生异常: 0x%08X\n", GetExceptionCode());
                OutputDebugStringA(err);
            }
        }
    }
    
    // 方法3: 尝试查找 Lua 状态机并直接调用 dofile
    // 这需要更复杂的实现，暂时跳过
    
    OutputDebugStringA("[AutoLoadLua] [警告] 未找到可用的函数\n");
    OutputDebugStringA("[AutoLoadLua] 建议: 修改源码添加自动加载功能\n");
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
    
    // 再等待一下，确保Lua环境完全初始化
    OutputDebugStringA("[AutoLoadLua] 等待 Lua 环境初始化...\n");
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // 尝试加载脚本
    TryLoadLuaScript();
    
    OutputDebugStringA("========================================\n");
    OutputDebugStringA("[AutoLoadLua] 脚本加载尝试完成\n");
    OutputDebugStringA("========================================\n");
    
    return 0;
}

// DLL入口点
BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
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
        
    case DLL_PROCESS_DETACH:
        OutputDebugStringA("[AutoLoadLua] DLL 已卸载\n");
        break;
    }
    
    return TRUE;
}

