#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QMap>
#include <functional>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class BasePage;
class HomePage;

/**
 * @brief 主窗口类 - 使用 QStackedWidget 和惰性加载管理页面
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 页面索引枚举
    enum PageIndex {
        PAGE_HOME = 0,      // 主页
        PAGE_LED = 1,       // LED控制
        PAGE_CAMERA = 2,    // 摄像头
        PAGE_TEMP = 3,      // 温度传感器
        PAGE_CHART = 4,     // 数据图表
        PAGE_AUDIO = 5,     // 音频播放
        PAGE_NETWORK = 6,   // 网络通信
        PAGE_FILE = 7,      // 文件系统
        PAGE_SYSTEM = 8     // 系统信息
    };

private slots:
    void switchToPage(int pageIndex);
    void backToHome();

private:
    void setupUI();
    void registerPageFactory(int pageIndex, std::function<BasePage*()> factory);
    BasePage* getOrCreatePage(int pageIndex);

    Ui::MainWindow *ui;
    QStackedWidget *stackedWidget;
    HomePage *homePage;
    
    // 存储已创建的页面实例
    QMap<int, BasePage*> pageCache;
    
    // 页面工厂函数（用于惰性加载）
    QMap<int, std::function<BasePage*()>> pageFactories;
    
    // 记录当前页面索引
    int currentPageIndex;
};

#endif // MAINWINDOW_H
