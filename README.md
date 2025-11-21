# AutoLoadLua.dll - DLL注入自动加载Lua脚本

## 🎯 功能

这个DLL会在注入到GameServer后，自动执行：
```lua
dofile('data/server/TestServer/scripts/LoadPVP.lua')
```

## 📦 文件说明

### 核心源码
- **`AutoLoadLua_Final.cpp`** - 最终版DLL源码（推荐使用）
- **`Injector.cpp`** - DLL注入工具源码

### 编译脚本
- **`一键编译和使用.bat`** - 一键编译DLL和注入工具
- **`build.bat`** - 单独编译DLL
- **`build_injector.bat`** - 单独编译注入工具

### 使用脚本
- **`自动注入.bat`** - 自动注入DLL到GameServer
- **`使用说明.txt`** - 详细使用说明

## 🚀 快速开始

### 方法1：从GitHub Actions下载（推荐）

1. 访问：https://github.com/yanlongyang806-cyber/ZZZR/actions
2. 等待工作流自动运行（或手动触发）
3. 下载 Artifacts 中的 `AutoLoadLua-DLL.zip`
4. 解压得到 `AutoLoadLua.dll` 和 `Injector.exe`

### 方法2：本地编译

1. **打开 Visual Studio Developer Command Prompt**

2. **进入目录**：
   ```bash
   cd tools/AutoLoadLua
   ```

3. **一键编译**：
   ```bash
   一键编译和使用.bat
   ```

   或者手动编译：
   ```bash
   # 编译 DLL
   cl /LD /EHsc /O2 /W3 AutoLoadLua_Final.cpp /link /OUT:AutoLoadLua.dll kernel32.lib psapi.lib
   
   # 编译注入工具
   cl /EHsc /O2 /W3 Injector.cpp /link /OUT:Injector.exe kernel32.lib tlhelp32.lib
   ```

## 📝 使用方法

### 1. 复制文件

将 `AutoLoadLua.dll` 复制到 GameServer.exe 所在目录：
```
tools\bin\AutoLoadLua.dll
```

### 2. 启动 GameServer

```bash
tools\bin\StartServer.bat
```

### 3. 注入 DLL

**方法1：使用自动注入脚本**
```bash
tools\AutoLoadLua\自动注入.bat
```

**方法2：手动注入**
```bash
cd tools\AutoLoadLua
Injector.exe GameServer.exe AutoLoadLua.dll
```

**⚠️ 注意**：需要以管理员身份运行！

### 4. 查看日志

使用 DebugView 查看 DLL 的输出：
1. 下载：https://docs.microsoft.com/en-us/sysinternals/downloads/debugview
2. 运行 DebugView
3. 启用 "Capture Global Win32"
4. 查看 `[AutoLoadLua]` 开头的日志

## ✅ 成功标志

如果看到以下日志，说明注入成功：
```
[AutoLoadLua] DLL 已加载
[AutoLoadLua] [OK] 找到 GameServer.exe 模块
[AutoLoadLua] [OK] 找到函数: TestServer_RunScript
[AutoLoadLua] [成功] 脚本加载完成
```

## ⚠️ 注意事项

1. **需要管理员权限**：DLL注入通常需要管理员权限
2. **防病毒软件**：某些防病毒软件可能会阻止DLL注入
3. **函数导出**：如果GameServer没有导出相关函数，可能需要修改源码
4. **路径问题**：确保 `LoadPVP.lua` 的路径正确（相对于GameServer工作目录）

## 🔧 修改脚本路径

如果需要加载不同的脚本，修改 `AutoLoadLua_Final.cpp` 中的：

```cpp
const char* scriptPath = "data/server/TestServer/scripts/LoadPVP.lua";
```

改为你需要的路径。
