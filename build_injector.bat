@echo off
chcp 65001 >nul
echo ========================================
echo 编译 Injector.exe
echo ========================================
echo.

REM 检查是否有Visual Studio环境
where cl >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [错误] 未找到 cl.exe (Visual Studio编译器)
    echo 请先运行 Visual Studio 的 Developer Command Prompt
    pause
    exit /b 1
)

echo [OK] 找到编译器
echo.

REM 编译Injector
echo 正在编译 Injector.exe...
cl /EHsc /O2 /W3 Injector.cpp /link /OUT:Injector.exe

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo [成功] Injector.exe 编译完成！
    echo ========================================
    echo.
    echo 使用方法：
    echo   Injector.exe GameServer.exe AutoLoadLua.dll
    echo.
) else (
    echo.
    echo [错误] 编译失败
    echo.
)

pause

