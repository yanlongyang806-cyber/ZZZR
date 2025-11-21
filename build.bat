@echo off
chcp 65001 >nul
echo ========================================
echo 编译 AutoLoadLua.dll
echo ========================================
echo.

REM 检查是否有Visual Studio环境
where cl >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [错误] 未找到 cl.exe (Visual Studio编译器)
    echo 请先运行 Visual Studio 的 Developer Command Prompt
    echo 或者设置 Visual Studio 环境变量
    pause
    exit /b 1
)

echo [OK] 找到编译器
echo.

REM 编译DLL
echo 正在编译...
cl /LD /EHsc /O2 /W3 AutoLoadLua_Improved.cpp /link /OUT:AutoLoadLua.dll kernel32.lib psapi.lib

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo [成功] AutoLoadLua.dll 编译完成！
    echo ========================================
    echo.
    echo 使用方法：
    echo 1. 将 AutoLoadLua.dll 复制到 GameServer.exe 所在目录
    echo 2. 使用 DLL 注入工具（如 Injector.exe）注入到 GameServer.exe
    echo 3. 或者修改 GameServer 启动参数添加 -dll:AutoLoadLua.dll（如果支持）
    echo.
) else (
    echo.
    echo [错误] 编译失败
    echo.
)

pause

