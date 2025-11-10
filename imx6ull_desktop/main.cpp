#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    MainWindow w;
    
    // 嵌入式设备使用全屏显示
#ifdef Q_OS_LINUX
    w.showFullScreen();  // 在 Linux 平台（IMX6ULL）上全屏显示
#else
    w.resize(800, 480);  // 在桌面平台上设置固定大小（模拟 IMX6ULL 屏幕分辨率）
    w.show();
#endif
    
    return a.exec();
}
