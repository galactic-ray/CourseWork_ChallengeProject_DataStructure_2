#include "../include/gui_mainwindow.h"

#include <QApplication>
#include <QStyleFactory>
#include <QFont>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    /* ===== 1. 高 DPI 支持（Ubuntu / 高分屏 必须） ===== */
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);

    /* ===== 2. 应用程序基本信息 ===== */
    app.setApplicationName("投票选举管理系统");
    app.setApplicationVersion("2.0");
    app.setOrganizationName("研究性学习项目");

    /* ===== 3. 统一 Qt 控件风格（跨平台一致） ===== */
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    /* ===== 4. 统一字体（解决 Windows / Ubuntu 差异的关键） ===== */
#ifdef Q_OS_WIN
    QFont appFont("Microsoft YaHei", 10);
#else   // Linux / Ubuntu
    QFont appFont("Noto Sans CJK SC", 10);
#endif
    app.setFont(appFont);

    /* ===== 5. 创建并显示主窗口 ===== */
    MainWindow window;
    window.show();

    return app.exec();
}
