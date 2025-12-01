cmake_minimum_required(VERSION 3.10)
project(ElectionSystem VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt5
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

# 设置Qt5的MOC、UIC、RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 源文件
set(SOURCES
    src/gui_main.cpp
    src/gui_mainwindow.cpp
    src/election_core.cpp
)

set(HEADERS
    include/gui_mainwindow.h
    include/election_core.h
)

# 创建GUI可执行文件
add_executable(election_gui ${SOURCES} ${HEADERS})

# 链接Qt5库
target_link_libraries(election_gui 
    Qt5::Core 
    Qt5::Widgets
)

# 设置输出目录
set_target_properties(election_gui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(election_gui PRIVATE -O2)
else()
    target_compile_options(election_gui PRIVATE -g -Wall -Wextra)
endif()

# 安装规则（可选）
install(TARGETS election_gui
    RUNTIME DESTINATION bin
)

