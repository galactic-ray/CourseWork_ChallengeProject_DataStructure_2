cmake_minimum_required(VERSION 3.10)
project(ElectionSystem VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 查找Qt5
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

# 设置Qt5的MOC、UIC、RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# GUI 源文件
set(GUI_SOURCES
    src/gui_main.cpp
    src/gui_mainwindow.cpp
    src/election_core.cpp
)

set(GUI_HEADERS
    include/gui_mainwindow.h
    include/election_core.h
)

# 创建 GUI 可执行文件
add_executable(election_gui ${GUI_SOURCES} ${GUI_HEADERS})

# 链接 Qt5 库
target_link_libraries(election_gui 
    Qt5::Core 
    Qt5::Widgets
)

# 设置 GUI 输出目录
set_target_properties(election_gui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# GUI 编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(election_gui PRIVATE -O2)
else()
    target_compile_options(election_gui PRIVATE -g -Wall -Wextra)
endif()

# 控制台版本（终端界面）
add_executable(election_console
    src/main.cpp
    src/election_core.cpp
)

set_target_properties(election_console PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(election_console PRIVATE -O2)
else()
    target_compile_options(election_console PRIVATE -g -Wall -Wextra)
endif()

