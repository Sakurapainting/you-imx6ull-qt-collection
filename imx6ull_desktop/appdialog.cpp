#include "appdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFrame>

AppDialog::AppDialog(const QString &appName, QWidget *parent)
    : QDialog(parent)
    , m_appName(appName)
    , m_sensorTimer(nullptr)
    , m_adcRawLabel(nullptr)
    , m_adcVoltageLabel(nullptr)
    , m_adcScaleLabel(nullptr)
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
    if (m_sensorTimer) {
        m_sensorTimer->stop();
    }
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
    m_contentLabel->setText("ADC 传感器数据监控");
    m_contentLabel->setStyleSheet("font-size: 18px; color: #333; font-weight: bold;");
    
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(m_contentLabel->parentWidget()->layout());
    if (layout) {
        // 创建数据显示区域
        QWidget *dataWidget = new QWidget(this);
        dataWidget->setStyleSheet("background-color: #f5f5f5; border-radius: 10px;");
        
        QVBoxLayout *dataLayout = new QVBoxLayout(dataWidget);
        dataLayout->setContentsMargins(20, 20, 20, 20);
        dataLayout->setSpacing(15);
        
        // ADC 原始值
        QFrame *rawFrame = new QFrame(this);
        rawFrame->setStyleSheet("background-color: white; border-radius: 8px; padding: 15px;");
        QVBoxLayout *rawLayout = new QVBoxLayout(rawFrame);
        QLabel *rawTitle = new QLabel("ADC 原始值", this);
        rawTitle->setStyleSheet("font-size: 14px; color: #888;");
        m_adcRawLabel = new QLabel("-- ", this);
        m_adcRawLabel->setStyleSheet("font-size: 28px; color: #2196F3; font-weight: bold;");
        rawLayout->addWidget(rawTitle);
        rawLayout->addWidget(m_adcRawLabel);
        
        // 电压值
        QFrame *voltageFrame = new QFrame(this);
        voltageFrame->setStyleSheet("background-color: white; border-radius: 8px; padding: 15px;");
        QVBoxLayout *voltageLayout = new QVBoxLayout(voltageFrame);
        QLabel *voltageTitle = new QLabel("电压值", this);
        voltageTitle->setStyleSheet("font-size: 14px; color: #888;");
        m_adcVoltageLabel = new QLabel("-- V", this);
        m_adcVoltageLabel->setStyleSheet("font-size: 28px; color: #4CAF50; font-weight: bold;");
        voltageLayout->addWidget(voltageTitle);
        voltageLayout->addWidget(m_adcVoltageLabel);
        
        // 比例系数
        QFrame *scaleFrame = new QFrame(this);
        scaleFrame->setStyleSheet("background-color: white; border-radius: 8px; padding: 15px;");
        QVBoxLayout *scaleLayout = new QVBoxLayout(scaleFrame);
        QLabel *scaleTitle = new QLabel("Scale 系数", this);
        scaleTitle->setStyleSheet("font-size: 14px; color: #888;");
        m_adcScaleLabel = new QLabel("--", this);
        m_adcScaleLabel->setStyleSheet("font-size: 24px; color: #FF9800; font-weight: bold;");
        scaleLayout->addWidget(scaleTitle);
        scaleLayout->addWidget(m_adcScaleLabel);
        
        dataLayout->addWidget(rawFrame);
        dataLayout->addWidget(voltageFrame);
        dataLayout->addWidget(scaleFrame);
        
        // 提示信息
        QLabel *infoLabel = new QLabel("数据每500ms更新一次", this);
        infoLabel->setAlignment(Qt::AlignCenter);
        infoLabel->setStyleSheet("font-size: 12px; color: #999; margin-top: 10px;");
        
        layout->addWidget(dataWidget);
        layout->addWidget(infoLabel);
        layout->addStretch();
        
        // 创建定时器更新传感器数据
        m_sensorTimer = new QTimer(this);
        connect(m_sensorTimer, &QTimer::timeout, this, &AppDialog::updateSensorData);
        m_sensorTimer->start(500);  // 500ms 更新一次
        
        // 立即读取一次数据
        updateSensorData();
    }
}

void AppDialog::updateSensorData()
{
    int raw = 0;
    float scale = 0.0f;
    float voltage = 0.0f;
    
    int ret = readAdcData(raw, scale, voltage);
    
    if (ret == 0) {
        m_adcRawLabel->setText(QString::number(raw));
        m_adcVoltageLabel->setText(QString::number(voltage, 'f', 3) + " V");
        m_adcScaleLabel->setText(QString::number(scale, 'f', 6));
    } else {
        m_adcRawLabel->setText("读取失败");
        m_adcVoltageLabel->setText("-- V");
        m_adcScaleLabel->setText("--");
    }
}

int AppDialog::readAdcData(int &raw, float &scale, float &voltage)
{
    // 读取 scale 值
    QString scaleStr = readFileContent("/sys/bus/iio/devices/iio:device0/in_voltage_scale");
    if (scaleStr.isEmpty()) {
        qDebug() << "Failed to read ADC scale";
        return -1;
    }
    scale = scaleStr.toFloat();
    
    // 读取原始值
    QString rawStr = readFileContent("/sys/bus/iio/devices/iio:device0/in_voltage1_raw");
    if (rawStr.isEmpty()) {
        qDebug() << "Failed to read ADC raw value";
        return -1;
    }
    raw = rawStr.toInt();
    
    // 计算实际电压值（mV 转 V）
    voltage = (scale * raw) / 1000.0f;
    
    return 0;
}

QString AppDialog::readFileContent(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open file:" << filePath;
        return QString();
    }
    
    QTextStream in(&file);
    QString content = in.readLine().trimmed();
    file.close();
    
    return content;
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
