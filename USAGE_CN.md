# AutoLoadLua DLL 注入器使用说明

## 📦 下载编译好的文件

1. 访问 GitHub Actions：https://github.com/yanlongyang806-cyber/ZZZR/actions
2. 点击最新的成功编译（绿色勾号）
3. 在页面底部找到 "Artifacts"
4. 下载 `AutoLoadLua-DLL` 压缩包
5. 解压后得到：
   - `AutoLoadLua.dll` - 自动加载 Lua 脚本的 DLL
   - `Injector.exe` - DLL 注入工具

## 🚀 使用方法

### 方法 1：使用注入器（推荐）

**步骤：**

1. **准备文件**
   ```
   I:\Night\Night\
   ├── tools\bin\
   │   ├── GameServer.exe
   │   ├── AutoLoadLua.dll        ← 复制到这里
   │   └── Injector.exe            ← 复制到这里
   └── data\server\TestServer\scripts\
       └── LoadPVP.lua              ← 确保这个文件存在
   ```

2. **启动 GameServer**
   - 先正常启动 GameServer.exe
   - 等待服务器完全启动（显示 "Server ready" 或类似信息）

3. **注入 DLL**
   
   打开命令提示符（CMD），切换到 `tools\bin` 目录：
   ```cmd
   cd I:\Night\Night\tools\bin
   ```
   
   执行注入命令：
   ```cmd
   Injector.exe GameServer.exe AutoLoadLua.dll
   ```

4. **查看结果**
   
   注入成功后，DLL 会自动：
   - 等待 15 秒让 GameServer 初始化
   - 查找 GameServer.exe 模块
   - 列出导出函数（前 20 个）
   - 尝试调用 `TestServer_RunScript` 加载 `LoadPVP.lua`
   - `LoadPVP.lua` 会自动加载 `DuelCommands.lua`

5. **验证是否成功**
   
   在游戏中输入决斗命令测试：
   ```
   /duel 玩家名字
   /duelaccept
   /dueldecline
   ```

### 方法 2：创建自动注入批处理

创建 `tools\bin\启动并注入.bat`：

```batch
@echo off
chcp 65001 >nul
title 启动 GameServer 并注入 Lua 脚本
color 0A

echo ========================================
echo 启动 GameServer 并自动注入 Lua 脚本
echo ========================================
echo.

REM 检查文件是否存在
if not exist "GameServer.exe" (
    echo [错误] 未找到 GameServer.exe
    pause
    exit /b 1
)

if not exist "AutoLoadLua.dll" (
    echo [错误] 未找到 AutoLoadLua.dll
    echo 请从 GitHub Actions 下载并放到此目录
    pause
    exit /b 1
)

if not exist "Injector.exe" (
    echo [错误] 未找到 Injector.exe
    echo 请从 GitHub Actions 下载并放到此目录
    pause
    exit /b 1
)

echo [1/3] 启动 GameServer...
start "GameServer" "%CD%\GameServer.exe"

echo [2/3] 等待 GameServer 启动（20秒）...
timeout /t 20 /nobreak >nul

echo [3/3] 注入 AutoLoadLua.dll...
Injector.exe GameServer.exe AutoLoadLua.dll

echo.
echo ========================================
echo 注入完成
echo ========================================
echo.
echo 提示：
echo - DLL 会自动加载 LoadPVP.lua
echo - LoadPVP.lua 会加载 DuelCommands.lua
echo - 在游戏中使用 /duel 命令测试
echo.

pause
```

使用方法：
- 双击 `启动并注入.bat` 即可自动完成所有步骤

## 🔍 调试方法

如果注入后没有效果，使用 DebugView 查看调试输出：

1. **下载 DebugView**
   - 访问：https://learn.microsoft.com/sysinternals/downloads/debugview
   - 下载并运行 `Dbgview.exe`

2. **配置 DebugView**
   - 菜单：Capture > Capture Global Win32
   - 菜单：Capture > Capture Win32（确保都勾选）

3. **查看输出**
   
   注入成功后会看到类似输出：
   ```
   [AutoLoadLua] DLL 已加载
   [AutoLoadLua] 工作线程已创建
   [AutoLoadLua] 工作线程已启动
   [AutoLoadLua] 等待 GameServer 初始化...
   [AutoLoadLua] [OK] 找到 GameServer.exe 模块 (0x...)
   [AutoLoadLua] 正在列出 GameServer.exe 的导出函数...
   [AutoLoadLua] 找到 20 个导出函数
   [AutoLoadLua]   导出函数 0: ...
   [AutoLoadLua] 开始尝试加载 Lua 脚本
   [AutoLoadLua] [OK] 找到函数: TestServer_RunScript (地址: 0x...)
   [AutoLoadLua] 正在加载脚本: LoadPVP.lua
   [AutoLoadLua] [成功] 脚本加载完成
   ```

## ⚠️ 注意事项

1. **GameServer 必须先启动**
   - 注入器需要找到正在运行的 GameServer.exe 进程
   - 如果 GameServer 没运行，注入会失败

2. **路径问题**
   - `AutoLoadLua.dll` 中硬编码的脚本路径是：`data/server/TestServer/scripts/LoadPVP.lua`
   - 确保这个文件存在且路径正确

3. **权限问题**
   - 如果注入失败，尝试以管理员身份运行 `Injector.exe`

4. **32位 vs 64位**
   - 当前编译的是 64位（x64）版本
   - 如果 GameServer.exe 是 32位，需要重新编译 32位版本

5. **防病毒软件**
   - DLL 注入可能被杀毒软件拦截
   - 如果遇到问题，请将 `AutoLoadLua.dll` 和 `Injector.exe` 添加到白名单

## 🛠️ 修改脚本路径

如果你的 Lua 脚本路径不同，需要修改 `AutoLoadLua_Final.cpp` 第 21 行：

```cpp
const char* scriptPath = "data/server/TestServer/scripts/LoadPVP.lua";
```

改为你的路径，然后重新编译。

## 📚 相关文件

- `AutoLoadLua_Final.cpp` - DLL 源代码
- `Injector.cpp` - 注入器源代码
- `build.bat` - DLL 编译脚本
- `build_injector.bat` - 注入器编译脚本

## 🎯 目标

注入成功后，你的服务器将：
- 自动加载 `LoadPVP.lua`
- 自动加载 `DuelCommands.lua`
- 玩家可以使用 `/duel` 等命令进行决斗
- 所有 PVP 功能都将可用

## 📞 遇到问题？

如果遇到问题，请检查：
1. DebugView 中的输出
2. GameServer 日志
3. Lua 脚本是否有语法错误
4. 路径是否正确

