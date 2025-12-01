#include "../include/gui_mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("投票选举管理系统");
    app.setApplicationVersion("2.0");
    app.setOrganizationName("研究性学习项目");
    
    // 设置样式（可选）
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // 创建并显示主窗口
    MainWindow window;
    window.show();
    
    return app.exec();
}

