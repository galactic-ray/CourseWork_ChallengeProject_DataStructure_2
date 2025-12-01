# 目录结构说明

## 项目目录组织

本项目采用标准的C++项目目录结构：

```
code2/
├── include/                    # 头文件目录
│   ├── election_core.h        # 核心选举系统头文件
│   └── gui_mainwindow.h       # GUI主窗口头文件
│
├── src/                        # 源文件目录
│   ├── election_core.cpp      # 核心选举系统实现
│   ├── main.cpp               # 控制台版本主程序
│   ├── gui_main.cpp           # GUI版本主程序
│   └── gui_mainwindow.cpp     # GUI主窗口实现
│
├── build/                      # 构建目录（自动生成，已忽略）
├── bin/                        # 可执行文件目录（自动生成，已忽略）
│
├── election_gui.pro           # Qt qmake项目文件
├── CMakeLists.txt             # CMake项目文件
├── Makefile                    # 通用Makefile
├── .gitignore                 # Git忽略文件
│
└── 文档文件/
    ├── README.md              # 项目说明
    ├── BUILD.md               # 编译指南
    ├── PERFORMANCE_ANALYSIS.md # 性能分析
    ├── CODE_REVIEW.md         # 代码审查
    ├── PROJECT_SUMMARY.md     # 项目总结
    └── DIRECTORY_STRUCTURE.md # 本文件
```

## 目录说明

### include/ 目录
存放所有头文件（.h文件）。这些文件定义了接口和数据结构。

- **election_core.h**: 核心选举系统的头文件，包含：
  - `Candidate` 结构体
  - `DataValidator` 类
  - `FileManager` 类
  - `Statistics` 类
  - `ElectionSystem` 类

- **gui_mainwindow.h**: GUI主窗口的头文件，包含：
  - `MainWindow` 类定义
  - 所有GUI组件的声明

### src/ 目录
存放所有源文件（.cpp文件）。这些文件实现了头文件中声明的功能。

- **election_core.cpp**: 核心选举系统的实现
- **main.cpp**: 控制台版本的主程序，包含UI类和测试用例
- **gui_main.cpp**: GUI版本的主程序入口
- **gui_mainwindow.cpp**: GUI主窗口的实现

## Include 路径

### 在源文件中引用头文件

由于头文件在 `include/` 目录，源文件在 `src/` 目录，引用方式如下：

```cpp
// 在 src/ 目录的 .cpp 文件中
#include "../include/election_core.h"
#include "../include/gui_mainwindow.h"
```

### 在头文件中引用头文件

由于都在 `include/` 目录中，可以直接引用：

```cpp
// 在 include/gui_mainwindow.h 中
#include "election_core.h"  // 直接引用，因为都在include目录
```

## 编译配置

所有项目配置文件都已更新以支持新的目录结构：

### Makefile
- 使用 `-Iinclude` 选项指定包含目录
- 源文件路径：`src/*.cpp`
- 对象文件输出到：`src/*.o`

### CMakeLists.txt
- 使用 `include_directories(include)` 指定包含目录
- 源文件路径：`src/*.cpp`

### election_gui.pro (qmake)
- 使用 `INCLUDEPATH += include` 指定包含目录
- 源文件路径：`src/*.cpp`

## 优势

采用这种目录结构的优势：

1. **清晰的组织**: 头文件和源文件分离，结构清晰
2. **易于维护**: 修改时容易找到对应文件
3. **标准实践**: 符合C++项目的常见组织方式
4. **便于扩展**: 添加新模块时结构清晰
5. **IDE友好**: 大多数IDE能更好地识别这种结构

## 迁移说明

如果从旧版本迁移，需要注意：

1. 所有 `#include` 路径已更新
2. 项目配置文件已更新
3. 编译命令需要包含 `-Iinclude` 选项
4. 文档已更新以反映新结构

## 验证

编译前可以验证目录结构：

```bash
# 检查目录结构
ls -la include/
ls -la src/

# 验证头文件引用
grep -r "#include" src/ | head -5
```

