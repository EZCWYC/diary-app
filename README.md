# 安全日记本

一款基于 EUI-NEO 框架开发的本地加密日记应用，支持密码保护、数据加密、明暗主题切换等功能。

## 功能特性

- **密码保护** — 首次启动设置主密码，后续凭密码解锁
- **AES-256 加密** — 所有日记内容使用 AES-256 加密存储
- **新建 / 编辑 / 删除 / 查看** — 完整的日记 CRUD 操作
- **明暗主题** — 支持亮色和暗色模式
- **亚克力 / 毛玻璃效果** — 背景效果可选
- **平滑动画** — 弹窗滑入、列表滚动等交互动画

## 项目结构

```
diary-app/
├── app/
│   └── diary.cpp        # 主程序 UI 逻辑
├── diary/
│   ├── diary_data.cpp   # 日记数据读写（加密）
│   ├── diary_data.h
│   ├── encrypt.cpp      # AES-256 加密实现
│   └── encrypt.h
├── CMakeLists.txt      # CMake 构建配置
└── .gitignore
```

## 构建方法

### 环境要求

- CMake 3.14+
- Visual Studio 2019+（Windows）
- OpenSSL
- Git

### 编译步骤

```bash
# 创建并进入构建目录
mkdir build && cd build

# 配置项目
cmake .. -G "Visual Studio 16 2019" -A x64

# 编译
cmake --build . --config Release

# 运行
./Release/diary.exe
```

编译完成后，`build/Release/assets/` 目录下包含字体和图标文件，请确保与 `diary.exe` 同一目录。

## 使用说明

1. **首次启动** — 输入您的名字和密码（至少 6 个字符）
2. **解锁** — 每次启动输入正确密码即可进入主界面
3. **新建日记** — 点击「新建日记」输入内容后保存
4. **编辑日记** — 点击「编辑日记」选择要编辑的日记
5. **删除日记** — 点击「删除日记」选择要删除的日记
6. **查看列表** — 点击「查看列表」浏览所有日记
7. **设置** — 在设置中切换明暗主题和背景效果

## 技术栈

- **UI 框架**: EUI-NEO（自研 DSL 风格 UI 框架）
- **加密**: AES-256-CBC（OpenSSL）
- **构建**: CMake + Visual Studio
