#include "appdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

AppDialog::AppDialog(const QString &appName, QWidget *parent)
    : QDialog(parent)
    , m_appName(appName)
{
    setupUI(appName);
    
    // 根据不同的应用创建不同的界面
    if (appName == "LED控制") {
        createLEDApp();
    } else if (appName == "传感器") {
        createSensorApp();
    } else if (appName == "网络设置") {
        createNetworkApp();
    } else if (appName == "系统设置") {
        createSettingsApp();
    } else if (appName == "多媒体") {
        createMediaApp();
    } else if (appName == "文件管理") {
        createFileApp();
    } else if (appName == "系统信息") {
        createSystemApp();
    } else if (appName == "关于") {
        createAboutApp();
    }
}

AppDialog::~AppDialog()
{
}

void AppDialog::setupUI(const QString &appName)
{
    // 设置为全屏
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);
    
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 创建标题栏
    QWidget *titleBar = new QWidget(this);
    titleBar->setFixedHeight(60);
    titleBar->setStyleSheet("background-color: #2196F3;");
    
    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(20, 0, 20, 0);
    
    m_titleLabel = new QLabel(appName, titleBar);
    m_titleLabel->setStyleSheet("color: white; font-size: 20px; font-weight: bold;");
    
    m_closeButton = new QPushButton("返回", titleBar);
    m_closeButton->setFixedSize(80, 40);
    m_closeButton->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(255, 255, 255, 0.2);"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:pressed {"
        "   background-color: rgba(255, 255, 255, 0.3);"
        "}"
    );
    
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::close);
    
    titleLayout->addWidget(m_titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(m_closeButton);
    
    // 创建内容区域
    QWidget *contentWidget = new QWidget(this);
    contentWidget->setStyleSheet("background-color: white;");
    
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    
    m_contentLabel = new QLabel("应用内容区域", contentWidget);
    m_contentLabel->setAlignment(Qt::AlignCenter);
    m_contentLabel->setStyleSheet("font-size: 16px; color: #666;");
    
    contentLayout->addWidget(m_contentLabel);
    
    mainLayout->addWidget(titleBar);
    mainLayout->addWidget(contentWidget);
}

void AppDialog::createLEDApp()
{
    // 首先关闭心跳灯
    system("echo none > /sys/class/leds/red/trigger");
    
    m_contentLabel->setText("LED 控制应用\n\n控制板载 RED LED 灯的开关");
    
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(m_contentLabel->parentWidget()->layout());
    if (layout) {
        // 创建状态显示标签
        QLabel *statusLabel = new QLabel("LED 状态: --", this);
        statusLabel->setAlignment(Qt::AlignCenter);
        statusLabel->setStyleSheet("font-size: 16px; color: #333; padding: 10px;");
        
        // 创建控制按钮
        QPushButton *ledOnBtn = new QPushButton("打开 LED", this);
        QPushButton *ledOffBtn = new QPushButton("关闭 LED", this);
        
        ledOnBtn->setStyleSheet("QPushButton { padding: 15px; font-size: 16px; background-color: #4CAF50; color: white; border: none; border-radius: 5px; }");
        ledOffBtn->setStyleSheet("QPushButton { padding: 15px; font-size: 16px; background-color: #f44336; color: white; border: none; border-radius: 5px; }");
        
        // 连接按钮信号
        connect(ledOnBtn, &QPushButton::clicked, this, [statusLabel]() {
            QFile file("/sys/devices/platform/dtsleds/leds/red/brightness");
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << "1";
                file.close();
                statusLabel->setText("LED 状态: 开启");
                statusLabel->setStyleSheet("font-size: 16px; color: #4CAF50; padding: 10px; font-weight: bold;");
                qDebug() << "LED turned ON";
            } else {
                statusLabel->setText("LED 状态: 操作失败");
                statusLabel->setStyleSheet("font-size: 16px; color: #f44336; padding: 10px;");
                qDebug() << "Failed to open LED device for writing";
            }
        });
        
        connect(ledOffBtn, &QPushButton::clicked, this, [statusLabel]() {
            QFile file("/sys/devices/platform/dtsleds/leds/red/brightness");
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << "0";
                file.close();
                statusLabel->setText("LED 状态: 关闭");
                statusLabel->setStyleSheet("font-size: 16px; color: #666; padding: 10px;");
                qDebug() << "LED turned OFF";
            } else {
                statusLabel->setText("LED 状态: 操作失败");
                statusLabel->setStyleSheet("font-size: 16px; color: #f44336; padding: 10px;");
                qDebug() << "Failed to open LED device for writing";
            }
        });
        
        layout->addWidget(statusLabel);
        layout->addSpacing(20);
        layout->addWidget(ledOnBtn);
        layout->addWidget(ledOffBtn);
        layout->addStretch();
    }
}

void AppDialog::createSensorApp()
{
    m_contentLabel->setText("传感器数据监控\n\n温度：-- °C\n湿度：-- %\n光照：-- lux");
}

void AppDialog::createNetworkApp()
{
    m_contentLabel->setText("网络设置\n\nIP 地址：192.168.1.100\n子网掩码：255.255.255.0\n网关：192.168.1.1");
}

void AppDialog::createSettingsApp()
{
    m_contentLabel->setText("系统设置\n\n亮度调节\n音量调节\n语言设置\n日期时间");
}

void AppDialog::createMediaApp()
{
    m_contentLabel->setText("多媒体播放\n\n支持音频和视频播放\n图片浏览");
}

void AppDialog::createFileApp()
{
    m_contentLabel->setText("文件管理器\n\n浏览系统文件\n文件操作（复制、删除、移动）");
}

void AppDialog::createSystemApp()
{
    m_contentLabel->setText("系统信息\n\nCPU：NXP i.MX6ULL\n内存：512MB\n存储：8GB eMMC");
}

void AppDialog::createAboutApp()
{
    m_contentLabel->setText("关于\n\nIMX6ULL 综合应用平台\n版本：1.0.0\n\n基于 Qt 开发的嵌入式应用系统");
}
