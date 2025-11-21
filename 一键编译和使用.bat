@echo off
chcp 65001 >nul
title AutoLoadLua DLL 编译和使用

echo ========================================
echo AutoLoadLua DLL 编译和使用
echo ========================================
echo.

cd /d %~dp0

REM 检查编译器
where cl >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [错误] 未找到 cl.exe
    echo.
    echo 请先运行 Visual Studio Developer Command Prompt
    echo 或者执行以下命令设置环境：
    echo   "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
    echo.
    pause
    exit /b 1
)

echo [OK] 找到编译器
echo.

REM 编译 DLL
echo ========================================
echo 正在编译 AutoLoadLua.dll...
echo ========================================
echo.

cl /LD /EHsc /O2 /W3 AutoLoadLua_Final.cpp /link /OUT:AutoLoadLua.dll kernel32.lib psapi.lib

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [错误] DLL 编译失败
    pause
    exit /b 1
)

echo.
echo ========================================
echo [成功] AutoLoadLua.dll 编译完成！
echo ========================================
echo.

REM 编译注入工具
echo ========================================
echo 正在编译 Injector.exe...
echo ========================================
echo.

cl /EHsc /O2 /W3 Injector.cpp /link /OUT:Injector.exe kernel32.lib tlhelp32.lib

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [警告] Injector.exe 编译失败（可以稍后手动编译）
    echo.
) else (
    echo.
    echo [成功] Injector.exe 编译完成！
    echo.
)

echo ========================================
echo 使用说明
echo ========================================
echo.
echo 1. 将 AutoLoadLua.dll 复制到 GameServer.exe 所在目录
echo    例如: tools\bin\AutoLoadLua.dll
echo.
echo 2. 启动 GameServer.exe
echo.
echo 3. 使用 Injector.exe 注入 DLL:
echo    Injector.exe GameServer.exe AutoLoadLua.dll
echo.
echo 4. 使用 DebugView 查看日志输出
echo    下载: https://docs.microsoft.com/en-us/sysinternals/downloads/debugview
echo.
echo ========================================
echo.

pause

