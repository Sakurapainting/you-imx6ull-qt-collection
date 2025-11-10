#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "homepage.h"
#include "basepage.h"
#include "ledpage.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , stackedWidget(nullptr)
    , homePage(nullptr)
    , currentPageIndex(PAGE_HOME)
{
    ui->setupUi(this);
    setupUI();
    
    // 注册页面工厂（惰性加载）
    registerPageFactory(PAGE_LED, []() -> BasePage* {
        return new LedPage();
    });
    
    // TODO: 注册其他页面工厂
    // registerPageFactory(PAGE_CAMERA, []() -> BasePage* {
    //     return new CameraPage();
    // });
    
    qDebug() << "MainWindow initialized with lazy loading";
}

MainWindow::~MainWindow()
{
    // 清理所有缓存的页面
    for (auto page : pageCache) {
        delete page;
    }
    pageCache.clear();
    
    delete ui;
}

void MainWindow::setupUI()
{
    // 设置窗口标题和大小
    setWindowTitle("IMX6ULL 综合功能演示");
    resize(800, 480);  // 适配常见嵌入式屏幕尺寸
    
    // 创建 QStackedWidget
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);
    
    // 创建主页（总是创建，不惰性加载）
    homePage = new HomePage(this);
    stackedWidget->addWidget(homePage);  // index 0
    
    // 连接主页的功能按钮信号
    connect(homePage, &HomePage::functionButtonClicked, 
            this, &MainWindow::switchToPage);
    
    // 显示主页
    stackedWidget->setCurrentIndex(PAGE_HOME);
}

void MainWindow::registerPageFactory(int pageIndex, std::function<BasePage *()> factory)
{
    pageFactories[pageIndex] = factory;
    qDebug() << "Registered page factory for index:" << pageIndex;
}

BasePage* MainWindow::getOrCreatePage(int pageIndex)
{
    // 如果页面已经创建，直接返回
    if (pageCache.contains(pageIndex)) {
        qDebug() << "Page" << pageIndex << "found in cache";
        return pageCache[pageIndex];
    }
    
    // 如果没有工厂函数，返回 nullptr
    if (!pageFactories.contains(pageIndex)) {
        qDebug() << "No factory registered for page index:" << pageIndex;
        return nullptr;
    }
    
    // 使用工厂函数创建页面（惰性加载）
    qDebug() << "Creating page" << pageIndex << "using factory (lazy loading)";
    BasePage *page = pageFactories[pageIndex]();
    
    if (page) {
        // 连接返回主页信号
        connect(page, &BasePage::requestBackToHome, 
                this, &MainWindow::backToHome);
        
        // 添加到 stackedWidget
        stackedWidget->addWidget(page);
        
        // 缓存页面实例
        pageCache[pageIndex] = page;
        
        qDebug() << "Page" << pageIndex << "created and cached";
    }
    
    return page;
}

void MainWindow::switchToPage(int pageIndex)
{
    // 主页特殊处理
    if (pageIndex == PAGE_HOME) {
        backToHome();
        return;
    }
    
    // 获取或创建页面
    BasePage *page = getOrCreatePage(pageIndex);
    
    if (!page) {
        qDebug() << "Failed to create page:" << pageIndex;
        return;
    }
    
    // 调用当前页面的失活回调
    if (currentPageIndex != PAGE_HOME && pageCache.contains(currentPageIndex)) {
        pageCache[currentPageIndex]->onPageDeactivated();
    }
    
    // 切换到目标页面
    stackedWidget->setCurrentWidget(page);
    currentPageIndex = pageIndex;
    
    // 调用新页面的激活回调
    page->onPageActivated();
    
    qDebug() << "Switched to page:" << pageIndex;
}

void MainWindow::backToHome()
{
    // 调用当前页面的失活回调
    if (currentPageIndex != PAGE_HOME && pageCache.contains(currentPageIndex)) {
        pageCache[currentPageIndex]->onPageDeactivated();
    }
    
    // 切换到主页
    stackedWidget->setCurrentWidget(homePage);
    currentPageIndex = PAGE_HOME;
    
    qDebug() << "Back to home page";
}

