# TeaJarsKV

TeaJarsKV 是一个轻量级的键值存储系统，支持本地命令行操作和网络HTTP API访问。

## 快速开始

### 安装与构建

项目提供了 Go 语言编写的跨平台构建工具，位于 `exec_build/` 目录。

#### 方式一：使用预编译的 Builder（推荐）

`exec_build/` 目录下已包含各平台预编译好的 Builder 可执行文件，直接使用即可：

| 平台 | 文件 |
|------|------|
| Windows x64 | `exec_build/windows_amd64.exe` |
| Windows ARM64 | `exec_build/windows_arm64.exe` |
| Linux x64 | `exec_build/linux_amd64` |
| Linux ARM64 | `exec_build/linux_arm64` |
| macOS Intel | `exec_build/darwin_amd64` |
| macOS Apple Silicon | `exec_build/darwin_arm64` |

**Windows 用户**可直接运行快捷脚本：

```cmd
b.cmd
```

选择选项 1 可交叉编译全部平台的 Builder，选项 2 使用 MSVC 编译 C++ 项目。

#### 方式二：使用 Builder 编译 C++ 项目

Builder 支持以下编译器选项：

| 选项 | 说明 |
|------|------|
| `gcc` | 使用 g++ 编译（Windows 上即 MinGW） |
| `clang` | 使用 clang++ 编译 |
| `msvc` | 使用 MSVC cl.exe 编译（仅 Windows） |
| `auto` | 自动检测系统中可用的编译器（默认） |
| `cmake` | 通过 CMake 构建 |

运行示例：

```bash
# Windows
.\exec_build\windows_amd64.exe gcc
.\exec_build\windows_amd64.exe msvc

# macOS / Linux
./exec_build/darwin_amd64 clang
./exec_build/linux_amd64 auto
```

不带参数运行可查看帮助信息：

```bash
./exec_build/<你的平台文件>
```

#### 方式三：从源码编译 Builder

需要 Go 1.22+ 环境：

```bash
cd exec_build
go build -o builder .
```

或通过 `b.cmd` 脚本选项 1 一键交叉编译所有平台。

### 基础使用

#### 本地模式
直接运行程序进入交互式命令行模式：

```bash
./teajars
```

在 Windows 上：

```bash
.\teajars.exe
```

可用命令：
- `set key value` - 设置键值对
- `get key` - 获取键对应的值
- `type key` - 查看键的类型
- `del key` - 删除键
- `save filename` - 保存数据到文件
- `open filename` - 从文件加载数据
- `encryptkey key` - 设置加密密钥
- `clearall` - 清空所有数据
- `debug` - 开启调试模式
- `exit` - 退出程序

#### 网络模式
启动HTTP服务器：

```bash
./teajars net [-port 端口号] [-host 主机地址]
```

例如：
```bash
./teajars net -port 8080 -host 0.0.0.0
```

### API 使用

#### HTTP API 接口

- GET /api/status - 获取服务器状态
  - 请求体：无
  - 返回体：`{"status":"200"}`

- GET /api/get?key={key} - 获取指定键的值（兼容json请求）
  - 请求参数：key（查询参数）
  - 请求体：无（或 `{"key":"your_key"}`）
  - 返回体：`{"key":"your_key","value":"your_value"}` 或 `{"error":"Key not found"}`

- POST /api/set - 设置键值对
  - 请求体：`{"key":"your_key","value":"your_value"}`
  - 返回体：`{"status":"200"}` 或 `{"error":"错误信息"}`

- DELETE /api/del - 删除指定键
  - 请求体：`{"key":"your_key"}`
  - 返回体：`{"status":"200"}` 或 `{"error":"错误信息"}`

- POST /api/save - 保存数据到文件
  - 请求体：`{"file":"filename"}`
  - 返回体：`{"status":"200"}` 或 `{"error":"错误信息"}`

- GET /api/load - 从文件加载数据
  - 请求体：`{"file":"filename","clear":"true"}`（可选clear参数）
  - 返回体：`{"status":"200"}` 或 `{"error":"错误信息"}`

- POST /api/set_encrypt_key - 设置加密密钥
  - 请求体：`{"encrypt_key":"your_key"}`
  - 返回体：`{"status":"200"}` 或 `{"error":"错误信息"}`

#### 测试工具

项目包含一个Python编写的UI测试工具 (`test/ui_test.py`)，可以方便地测试HTTP API接口。需要安装requests库：

```bash
pip install requests
```

运行测试工具：

```bash
python test/ui_test.py
```

### 依赖项

- C++17 或更高版本的编译器
- Go 1.22+（用于编译跨平台 Builder，可选）
- Python 3.x + requests 库（用于测试工具，可选）

### 文件格式

数据文件采用自定义格式，包含：
- 文件头（版本信息，经过加密）
- 数据项数量
- 每个键值对的键和值都经过加密处理

## 版本信息

当前版本：0.97fastfix
创建时间：2026-07-04

## 许可证

请参阅项目中的许可证文件（如果存在）。