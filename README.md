# TeaJarsKV

TeaJarsKV 是一个轻量级的键值存储系统，支持本地命令行操作和网络HTTP API访问。

## 快速开始

### 安装与构建

项目提供了便捷的构建脚本 `b.cmd`，支持以下编译器：
- GCC (MinGW)
- MSVC

运行构建脚本：

```bash
b.cmd
```

然后根据提示选择相应的编译器。

### 基础使用

#### 本地模式
直接运行程序进入交互式命令行模式：

```bash
./main.exe
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
./main.exe net [-port 端口号] [-host 主机地址]
```

例如：
```bash
./main.exe net -port 8080 -host 0.0.0.0
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

- C++11 或更高版本的编译器
- Python 3.x（用于测试工具）
- requests 库（用于测试工具）

### 文件格式

数据文件采用自定义格式，包含：
- 文件头（版本信息，经过加密）
- 数据项数量
- 每个键值对的键和值都经过加密处理

## 版本信息

当前版本：0.93

## 许可证

请参阅项目中的许可证文件（如果存在）。
