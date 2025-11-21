@echo off
chcp 65001 >nul
title 自动注入 AutoLoadLua.dll

echo ========================================
echo 自动注入 AutoLoadLua.dll 到 GameServer
echo ========================================
echo.

cd /d %~dp0

REM 检查文件是否存在
if not exist "AutoLoadLua.dll" (
    echo [错误] AutoLoadLua.dll 不存在
    echo 请先运行 一键编译和使用.bat 编译 DLL
    pause
    exit /b 1
)

if not exist "Injector.exe" (
    echo [错误] Injector.exe 不存在
    echo 请先运行 一键编译和使用.bat 编译注入工具
    pause
    exit /b 1
)

REM 转换为绝对路径
set DLL_PATH=%CD%\AutoLoadLua.dll

echo DLL 路径: %DLL_PATH%
echo.

REM 查找 GameServer 进程
echo 正在查找 GameServer.exe 进程...
Injector.exe GameServer.exe "%DLL_PATH%"

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo [成功] DLL 注入完成！
    echo ========================================
    echo.
    echo 提示: 使用 DebugView 查看 DLL 的日志输出
    echo.
) else (
    echo.
    echo ========================================
    echo [失败] DLL 注入失败
    echo ========================================
    echo.
    echo 可能的原因:
    echo 1. GameServer.exe 未运行
    echo 2. 需要管理员权限
    echo 3. 防病毒软件阻止
    echo.
)

pause

