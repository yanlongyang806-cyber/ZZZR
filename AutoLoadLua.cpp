// AutoLoadLua.cpp - DLL注入自动加载Lua脚本
// 编译: cl /LD /EHsc AutoLoadLua.cpp /link /OUT:AutoLoadLua.dll

#include <windows.h>
#include <string>
#include <thread>
#include <chrono>

// 前向声明（需要根据实际GameServer导出函数调整）
typedef void (*TestServer_RunScriptFunc)(const char*);
typedef void (*TestServer_RunScriptRawFunc)(const char*);

// 全局变量
HMODULE g_hGameServer = NULL;
TestServer_RunScriptFunc g_pRunScript = NULL;
TestServer_RunScriptRawFunc g_pRunScriptRaw = NULL;

// 尝试加载Lua脚本
void TryLoadLuaScript()
{
    // 方法1: 如果GameServer导出了TestServer_RunScript函数
    if (g_pRunScript)
    {
        OutputDebugStringA("[AutoLoadLua] 使用方法1: TestServer_RunScript\n");
        const char* script = "data/server/TestServer/scripts/LoadPVP.lua";
        g_pRunScript(script);
        OutputDebugStringA("[AutoLoadLua] 已调用 TestServer_RunScript\n");
        return;
    }
    
    // 方法2: 如果导出了TestServer_RunScriptRaw函数
    if (g_pRunScriptRaw)
    {
        OutputDebugStringA("[AutoLoadLua] 使用方法2: TestServer_RunScriptRaw\n");
        // 直接执行dofile命令
        const char* cmd = "dofile('data/server/TestServer/scripts/LoadPVP.lua')";
        g_pRunScriptRaw(cmd);
        OutputDebugStringA("[AutoLoadLua] 已调用 TestServer_RunScriptRaw\n");
        return;
    }
    
    // 方法3: 尝试通过CmdParseTextCommand执行（如果可用）
    // 注意：这需要GameServer内部有CmdParseTextCommand函数
    OutputDebugStringA("[AutoLoadLua] 警告: 未找到导出函数，尝试其他方法...\n");
    OutputDebugStringA("[AutoLoadLua] 提示: 可能需要修改源码或使用Hook方法\n");
}

// 工作线程：等待GameServer初始化后加载脚本
DWORD WINAPI LoadScriptThread(LPVOID lpParam)
{
    // 等待GameServer完全启动（给足够的时间）
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // 尝试获取GameServer模块句柄
    g_hGameServer = GetModuleHandleA("GameServer.exe");
    if (!g_hGameServer)
    {
        // 如果GameServer.exe还没加载，等待一下再试
        for (int i = 0; i < 30; i++)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            g_hGameServer = GetModuleHandleA("GameServer.exe");
            if (g_hGameServer)
                break;
        }
    }
    
    if (!g_hGameServer)
    {
        // 无法找到GameServer，记录错误（可以通过OutputDebugString）
        OutputDebugStringA("[AutoLoadLua] ERROR: Cannot find GameServer.exe module\n");
        return 1;
    }
    
    // 尝试获取导出函数
    // 注意：这些函数可能不是导出的，需要通过其他方法调用
    g_pRunScript = (TestServer_RunScriptFunc)GetProcAddress(g_hGameServer, "TestServer_RunScript");
    g_pRunScriptRaw = (TestServer_RunScriptRawFunc)GetProcAddress(g_hGameServer, "TestServer_RunScriptRaw");
    
    // 也尝试查找其他可能的导出名称
    if (!g_pRunScript)
    {
        g_pRunScript = (TestServer_RunScriptFunc)GetProcAddress(g_hGameServer, "_TestServer_RunScript");
    }
    if (!g_pRunScriptRaw)
    {
        g_pRunScriptRaw = (TestServer_RunScriptRawFunc)GetProcAddress(g_hGameServer, "_TestServer_RunScriptRaw");
    }
    
    if (!g_pRunScript && !g_pRunScriptRaw)
    {
        char msg[256];
        sprintf_s(msg, sizeof(msg), "[AutoLoadLua] WARNING: Cannot find TestServer_RunScript functions in module 0x%p\n", g_hGameServer);
        OutputDebugStringA(msg);
        OutputDebugStringA("[AutoLoadLua] 提示: 这些函数可能不是导出的\n");
        OutputDebugStringA("[AutoLoadLua] 建议: 修改源码在 TestServer_StartLuaThread 中自动加载\n");
    }
    else
    {
        OutputDebugStringA("[AutoLoadLua] [OK] 找到导出函数\n");
    }
    
    // 再等待一下，确保Lua环境完全初始化
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // 尝试加载脚本
    TryLoadLuaScript();
    
    OutputDebugStringA("[AutoLoadLua] Script loading attempted\n");
    
    return 0;
}

// DLL入口点
BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        // 禁用线程库调用（提高性能）
        DisableThreadLibraryCalls(hModule);
        
        // 创建线程来加载脚本
        CreateThread(NULL, 0, LoadScriptThread, NULL, 0, NULL);
        
        OutputDebugStringA("[AutoLoadLua] DLL loaded, thread created\n");
        break;
        
    case DLL_PROCESS_DETACH:
        OutputDebugStringA("[AutoLoadLua] DLL unloaded\n");
        break;
    }
    
    return TRUE;
}

