#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "iconwidget.h"
#include "appdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDateTime>
#include <QTimer>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    // 设置窗口属性
    setWindowTitle("IMX6ULL Desktop");
    
    // 初始化应用列表（8个应用）
    m_apps = {
        {"LED控制", ""},
        {"传感器", ""},
        {"网络设置", ""},
        {"系统设置", ""},
        {"多媒体", ""},
        {"文件管理", ""},
        {"系统信息", ""},
        {"关于", ""}
    };
    
    setupUI();
    createPages();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    // 创建中心部件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 创建状态栏
    m_statusBar = new QLabel(this);
    m_statusBar->setFixedHeight(40);
    m_statusBar->setStyleSheet(
        "QLabel {"
        "   background-color: #1976D2;"
        "   color: white;"
        "   padding-left: 20px;"
        "   font-size: 14px;"
        "}"
    );
    
    // 更新时间
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        m_statusBar->setText("IMX6ULL Desktop    " + timeStr);
    });
    timer->start(1000);
    m_statusBar->setText("IMX6ULL Desktop    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    
    // 创建滑动控件
    m_sliderWidget = new SliderWidget(this);
    connect(m_sliderWidget, &SliderWidget::pageChanged, this, &MainWindow::onPageChanged);
    
    // 创建页面指示器
    m_pageIndicator = new QLabel(this);
    m_pageIndicator->setFixedHeight(30);
    m_pageIndicator->setAlignment(Qt::AlignCenter);
    m_pageIndicator->setStyleSheet("QLabel { color: #666; font-size: 14px; background-color: #f5f5f5; }");
    
    mainLayout->addWidget(m_statusBar);
    mainLayout->addWidget(m_sliderWidget);
    mainLayout->addWidget(m_pageIndicator);
}

void MainWindow::createPages()
{
    // 第一页显示所有8个图标（2行4列）
    QWidget *page1 = createPage(0);
    m_sliderWidget->addPage(page1);
    
    // 第二页为空桌面
    QWidget *page2 = new QWidget();
    page2->setStyleSheet("background-color: #f0f0f0;");
    m_sliderWidget->addPage(page2);
    
    updatePageIndicator();
}

QWidget* MainWindow::createPage(int pageIndex)
{
    Q_UNUSED(pageIndex);  // 现在只创建第一页，忽略页码参数
    
    QWidget *page = new QWidget();
    page->setStyleSheet("background-color: #f0f0f0;");
    
    // 创建网格布局
    QGridLayout *gridLayout = new QGridLayout(page);
    gridLayout->setContentsMargins(20, 30, 20, 30);
    gridLayout->setSpacing(15);
    
    // 第一页显示所有8个图标（2行4列）
    int row = 0, col = 0;
    for (int i = 0; i < m_apps.count(); ++i) {
        IconWidget *icon = new IconWidget(m_apps[i].iconPath, m_apps[i].name, page);
        connect(icon, &IconWidget::clicked, this, &MainWindow::onIconClicked);
        
        gridLayout->addWidget(icon, row, col, Qt::AlignCenter);
        
        col++;
        if (col >= 4) {  // 每行4列
            col = 0;
            row++;
        }
    }
    
    // 添加弹性空间
    gridLayout->setRowStretch(2, 1);
    
    return page;
}

void MainWindow::onIconClicked(const QString &appName)
{
    qDebug() << "Opening app:" << appName;
    
    // 创建并显示全屏应用对话框
    AppDialog *dialog = new AppDialog(appName, this);
    dialog->showFullScreen();
}

void MainWindow::onPageChanged(int index)
{
    Q_UNUSED(index);  // 使用 Q_UNUSED 宏消除警告
    updatePageIndicator();
}

void MainWindow::updatePageIndicator()
{
    int current = m_sliderWidget->currentPage() + 1;
    int total = m_sliderWidget->pageCount();
    
    // 创建圆点指示器
    QString dots;
    for (int i = 0; i < total; ++i) {
        if (i == current - 1) {
            dots += "● ";  // 当前页用实心圆
        } else {
            dots += "○ ";  // 其他页用空心圆
        }
    }
    
    m_pageIndicator->setText(QString("第 %1/%2 页   %3").arg(current).arg(total).arg(dots));
}

