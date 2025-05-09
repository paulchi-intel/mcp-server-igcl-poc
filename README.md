# MCP Server - Intel Graphics Control Library POC

> 本專案為 IGCL 概念驗證（Proof of Concept, PoC），基於 IGCL 0.95 版本開發。
>
> This project is a Proof of Concept (PoC) for IGCL, developed based on IGCL version 0.95.

<img src="mcp-server-igcl-icon.png" alt="MCP Server IGCL Icon" width="120" />

[繁中](#繁體中文) / [English](#english)

## 繁體中文

### 簡介

這是一個基於 Model Context Protocol (MCP) 的伺服器實作，專為 Intel Graphics Control Library (IGCL) 的概念驗證 (POC) 所設計。本專案允許透過 MCP 協議與 Intel 圖形硬體進行互動和控制。

### 演示影片

觀看[示範影片](demo_video_usage.mp4)以了解完整的安裝和使用流程。

### 特點

- 模組化的插件架構
- 輕量級的通訊協議
- 支援各種 Intel 圖形設定的控制
- 與 Claude 等 AI 助手的無縫整合

### 支援的平台

| 平台          | 支援狀態 | 編譯器     |
|--------------|---------|-----------|
| Windows      | ✅      | MINGW64   |
| Ubuntu Linux | ✅      | GCC       |
| macOS        | ✅      | GCC       |

### 插件功能

本專案目前包含以下 Intel 圖形控制插件：

- **get_3d_capabilities**: 獲取 3D 圖形處理能力的相關信息
- **set_anisotropic**: 控制各向異性過濾設定
- **set_endurance_gaming**: 啟用/停用耐久遊戲模式
- **set_frame_sync**: 控制幀同步設定

### 編譯指南

#### 前置需求
- CMake (3.20 或更高版本)
- 支援 C++20 的編譯器
- 支援的作業系統 (Windows, Linux, macOS)

#### 步驟

1. 複製此存儲庫
```bash
git clone https://github.com/yourusername/mcp_server_igcl_poc.git
cd mcp_server_igcl_poc
```

2. 準備好 IGCL 的 package，例如放在 C:\ControlApi

3. 把 IGCL 的 .dll build 成 .lib 給 "MCP Server" make 的時候用
```bash
# 在 MinGW 64-bit 終端機中執行
cd /c/ControlApi/Release/Dll
gendef IntelControlLib.dll
dlltool -d IntelControlLib.def -l IntelControlLib.lib -D IntelControlLib.dll
gendef ControlLib.dll
dlltool -d ControlLib.def -l ControlLib.lib -D ControlLib.dll
gendef IntelControlLib32.dll
dlltool -d IntelControlLib32.def -l IntelControlLib32.lib -D IntelControlLib32.dll
gendef ControlLib32.dll
dlltool -d ControlLib32.def -l ControlLib32.lib -D ControlLib32.dll
```

4. 使用 MinGW（GCC for Windows）來建構
   
   安裝 MSYS2：https://www.msys2.org/
   
   開啟 MSYS2 MinGW 64-bit 終端機 (C:\msys64\mingw64.exe)
   
   安裝必要套件：
```bash
pacman -Syu
pacman -S mingw-w64-x86_64-toolchain cmake git make
```
   
   Build：
```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

### 安裝步驟

編譯完成後，需要將程式和相關檔案部署到系統中以便與 Claude Desktop 整合：

1. 創建目標資料夾
```bash
mkdir -p /c/mcp_server_igcl/plugins
mkdir -p /c/mcp_server_igcl/logs
```

2. 複製主執行檔
```bash
cp build/server_igcl_poc.exe /c/mcp_server_igcl/
```

3. 複製所有插件
```bash
cp build/plugins/*.dll /c/mcp_server_igcl/plugins/
```

4. 下載、安裝並登入 Claude Desktop ([下載連結](https://claude.ai/download))

5. 在 Claude Desktop 中配置 MCP 伺服器:
   - 點擊 Claude Desktop 左上角的 漢堡選單 / 檔案 / 設定... / 開發者 / 編輯配置
   - 開啟並編輯 "claude_desktop_config.json" (如下列配置所示)
   - 儲存並關閉編輯器

6. 重新啟動 Claude Desktop

### 與 Claude 桌面版整合

在 Claude 桌面應用程式的配置中添加以下設定：

```json
{
  "mcpServers": {
    "server_igcl_poc": {
      "command": "C:\\mcp_server_igcl\\server_igcl_poc.exe",
      "args": [
        "-n", "server_igcl_poc",
        "-l", "C:\\mcp_server_igcl\\logs",
        "-p", "C:\\mcp_server_igcl\\plugins"
      ]
    }
  }
}
```

### 參數說明

- `-n`: 伺服器名稱 (可選)
- `-p`: 插件目錄路徑
- `-l`: 日誌目錄路徑

### 開發說明

要創建新的 IGCL 插件，請參考現有插件的結構，並確保實現所需的接口。所有插件應放置在 `plugins` 目錄中。

### 常見問題

- **問題：MCP Server 無法啟動。**
  - 解決方案：檢查配置檔案中的路徑是否正確，並確認所有檔案已完整複製。

- **問題：編譯時出現錯誤。**
  - 解決方案：確保已正確安裝所有依賴項，並檢查 IGCL .lib 檔案是否正確生成。

- **問題：Claude 無法連接到 MCP Server。**
  - 解決方案：確認 server_igcl_poc.exe 和插件 .dll 檔案都已正確複製到指定目錄，且 Claude 配置文件中的路徑正確無誤。

### 授權

本專案依據 [MIT 授權](LICENSE) 發布。

### 免責聲明

本專案為概念驗證，不代表最終產品。

## English

### Introduction

This is a server implementation based on the Model Context Protocol (MCP), designed specifically for the Proof of Concept (POC) of the Intel Graphics Control Library (IGCL). This project allows interaction and control of Intel graphics hardware through the MCP protocol.

### Demo Video

Watch the [demonstration video](demo_video_usage.mp4) to understand the complete installation and usage process.

### Features

- Modular plugin architecture
- Lightweight communication protocol
- Support for various Intel graphics settings
- Seamless integration with AI assistants like Claude

### Supported Platforms

| Platform      | Support | Compiler   |
|--------------|---------|-----------|
| Windows      | ✅      | MINGW64   |
| Ubuntu Linux | ✅      | GCC       |
| macOS        | ✅      | GCC       |

### Plugin Functions

The project currently includes the following Intel graphics control plugins:

- **get_3d_capabilities**: Get information about 3D graphics processing capabilities
- **set_anisotropic**: Control anisotropic filtering settings
- **set_endurance_gaming**: Enable/disable endurance gaming mode
- **set_frame_sync**: Control frame synchronization settings

### Compilation Guide

#### Prerequisites
- CMake (version 3.20 or higher)
- Compiler supporting C++20
- Supported operating systems (Windows, Linux, macOS)

#### Steps

1. Clone this repository
```bash
git clone https://github.com/yourusername/mcp_server_igcl_poc.git
cd mcp_server_igcl_poc
```

2. Prepare the IGCL package, for example, place it in C:\ControlApi

3. Build the IGCL .dll files into .lib files for use when making the "MCP Server"
```bash
# Execute in MinGW 64-bit terminal
cd /c/ControlApi/Release/Dll
gendef IntelControlLib.dll
dlltool -d IntelControlLib.def -l IntelControlLib.lib -D IntelControlLib.dll
gendef ControlLib.dll
dlltool -d ControlLib.def -l ControlLib.lib -D ControlLib.dll
gendef IntelControlLib32.dll
dlltool -d IntelControlLib32.def -l IntelControlLib32.lib -D IntelControlLib32.dll
gendef ControlLib32.dll
dlltool -d ControlLib32.def -l ControlLib32.lib -D ControlLib32.dll
```

4. Use MinGW (GCC for Windows) for building
   
   Install MSYS2: https://www.msys2.org/
   
   Open MSYS2 MinGW 64-bit terminal (C:\msys64\mingw64.exe)
   
   Install necessary packages:
```bash
pacman -Syu
pacman -S mingw-w64-x86_64-toolchain cmake git make
```
   
   Build:
```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

### Installation Steps

After compilation, you need to deploy the program and related files to your system for integration with Claude Desktop:

1. Create target directories
```bash
mkdir -p /c/mcp_server_igcl/plugins
mkdir -p /c/mcp_server_igcl/logs
```

2. Copy the main executable
```bash
cp build/server_igcl_poc.exe /c/mcp_server_igcl/
```

3. Copy all plugins
```bash
cp build/plugins/*.dll /c/mcp_server_igcl/plugins/
```

4. Download, install, and log in to Claude Desktop ([Download Link](https://claude.ai/download))

5. Configure the MCP server in Claude Desktop:
   - Click on the hamburger menu in the top-left corner of Claude Desktop / File / Settings... / Developer / Edit Config
   - Open and edit "claude_desktop_config.json" (as shown in the configuration below)
   - Save and close the editor

6. Restart Claude Desktop

### Integration with Claude Desktop

Add the following settings to the Claude desktop application's configuration:

```json
{
  "mcpServers": {
    "server_igcl_poc": {
      "command": "C:\\mcp_server_igcl\\server_igcl_poc.exe",
      "args": [
        "-n", "server_igcl_poc",
        "-l", "C:\\mcp_server_igcl\\logs",
        "-p", "C:\\mcp_server_igcl\\plugins"
      ]
    }
  }
}
```

### Parameter Description

- `-n`: Server name (optional)
- `-p`: Plugin directory path
- `-l`: Log directory path

### Development Instructions

To create new IGCL plugins, refer to the structure of existing plugins and ensure that the required interfaces are implemented. All plugins should be placed in the `plugins` directory.

### FAQ

- **Issue: MCP Server cannot start.**
  - Solution: Check if the path in the configuration file is correct and ensure all files are completely copied.

- **Issue: Errors during compilation.**
  - Solution: Ensure all dependencies are correctly installed and check if the IGCL .lib files are correctly generated.

- **Issue: Claude cannot connect to the MCP Server.**
  - Solution: Verify that server_igcl_poc.exe and all plugin .dll files have been correctly copied to the specified directory, and that the paths in the Claude configuration file are correct.

### License

This project is released under the [MIT License](LICENSE).

### Disclaimer

This project is a proof of concept and does not represent a final product. 