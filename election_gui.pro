QT += core widgets

CONFIG += c++11

TARGET = election_gui
TEMPLATE = app

SOURCES += \
    src/gui_main.cpp \
    src/gui_mainwindow.cpp \
    src/election_core.cpp

HEADERS += \
    include/gui_mainwindow.h \
    include/election_core.h

# 包含目录
INCLUDEPATH += include

# 默认构建目录
DESTDIR = $$PWD/bin
OBJECTS_DIR = $$PWD/build
MOC_DIR = $$PWD/build
RCC_DIR = $$PWD/build
UI_DIR = $$PWD/build

# 编译器标志
QMAKE_CXXFLAGS += -Wall -Wextra -fPIC

# 发布模式优化
CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS += -O2
}

# 调试模式
CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS += -g
    DEFINES += QT_DEBUG
}

