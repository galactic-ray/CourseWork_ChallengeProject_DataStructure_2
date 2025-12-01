# 编译指南

本文档详细说明如何编译投票选举管理系统的不同版本。

## 目录

- [控制台版本](#控制台版本)
- [GUI版本](#gui版本)
- [依赖项](#依赖项)
- [常见问题](#常见问题)

## 控制台版本

控制台版本不需要额外的依赖，只需要C++11编译器。

### 方法1：使用Makefile（推荐）

```bash
make console
# 或
make
```

### 方法2：手动编译

```bash
g++ -std=c++11 -Iinclude -o election_console src/main.cpp src/election_core.cpp
```

### 运行

```bash
./election_console
```

## GUI版本

GUI版本需要Qt5开发库。

### 安装依赖

#### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install qt5-default
# 或
sudo apt-get install qtbase5-dev qt5-qmake
```

#### Fedora/RHEL

```bash
sudo dnf install qt5-qtbase-devel
```

#### macOS (使用Homebrew)

```bash
brew install qt5
```

#### Windows

下载并安装Qt5：https://www.qt.io/download

### 编译方法

#### 方法1：使用qmake（推荐）

```bash
qmake election_gui.pro
make
```

#### 方法2：使用CMake

```bash
mkdir build
cd build
cmake ..
make
```

#### 方法3：使用Makefile

```bash
make gui
```

#### 方法4：手动编译

```bash
# 生成MOC文件
moc include/gui_mainwindow.h -o src/gui_mainwindow.moc.cpp

# 编译
g++ -std=c++11 -fPIC -Iinclude \
    $(pkg-config --cflags Qt5Widgets) \
    -c src/gui_main.cpp src/gui_mainwindow.cpp src/election_core.cpp src/gui_mainwindow.moc.cpp

# 链接
g++ -o election_gui \
    src/gui_main.o src/gui_mainwindow.o src/election_core.o src/gui_mainwindow.moc.o \
    $(pkg-config --libs Qt5Widgets)
```

### 运行GUI版本

```bash
./election_gui
# 或（如果使用CMake）
./build/bin/election_gui
```

## 依赖项

### 控制台版本

- C++11或更高版本的编译器（g++, clang++, MSVC等）
- 标准C++库（STL）

### GUI版本

- C++11或更高版本的编译器
- Qt5.7或更高版本
  - Qt5Core
  - Qt5Widgets

## 常见问题

### Q1: 找不到Qt5

**错误信息：**
```
Package 'Qt5Widgets' not found
```

**解决方案：**
1. 确认已安装Qt5开发包
2. 检查pkg-config路径：`pkg-config --variable=prefix Qt5Widgets`
3. 设置PKG_CONFIG_PATH环境变量（如果需要）

### Q2: MOC文件生成失败

**错误信息：**
```
moc: command not found
```

**解决方案：**
- 确保Qt5开发包已正确安装
- 使用qmake或CMake自动处理MOC文件

### Q3: 链接错误

**错误信息：**
```
undefined reference to `vtable for MainWindow'
```

**解决方案：**
- 确保MOC文件已正确生成
- 使用qmake或CMake自动处理

### Q4: 运行时找不到Qt库

**错误信息：**
```
error while loading shared libraries: libQt5Core.so.5
```

**解决方案：**
```bash
# 设置库路径
export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH

# 或安装运行时库
sudo apt-get install qt5-default
```

### Q5: 编译时出现C++11错误

**错误信息：**
```
error: 'auto' keyword not allowed
```

**解决方案：**
- 确保使用`-std=c++11`或更高版本的C++标准
- 检查编译器版本（需要支持C++11）

## 性能优化

### 发布版本编译

使用优化选项编译：

```bash
# qmake
qmake CONFIG+=release election_gui.pro
make

# CMake
cmake -DCMAKE_BUILD_TYPE=Release ..
make

# 手动
g++ -std=c++11 -O2 -DNDEBUG ...
```

### 调试版本编译

```bash
# qmake
qmake CONFIG+=debug election_gui.pro
make

# CMake
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# 手动
g++ -std=c++11 -g -Wall ...
```

## 交叉编译

### Windows (使用MinGW)

```bash
# 安装Qt5 for MinGW
# 使用Qt Creator或手动设置环境变量

qmake -spec win32-g++ election_gui.pro
mingw32-make
```

### macOS

```bash
# 使用Homebrew安装的Qt5
export PATH="/usr/local/opt/qt5/bin:$PATH"
qmake election_gui.pro
make
```

## 清理构建文件

```bash
# 使用Makefile
make clean

# 手动清理
rm -rf build bin *.o *.moc.cpp election_console election_gui
```

## 验证安装

编译成功后，可以运行测试：

```bash
# 控制台版本测试
./election_console
# 选择模式2运行测试用例

# GUI版本
./election_gui
# 检查界面是否正常显示
```

