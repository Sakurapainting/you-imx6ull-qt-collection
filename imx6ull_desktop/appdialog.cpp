#include "appdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFrame>
#include <QScrollArea>
#include <QSlider>
#include <QButtonGroup>

AppDialog::AppDialog(const QString &appName, QWidget *parent)
    : QDialog(parent)
    , m_appName(appName)
    , m_sensorTimer(nullptr)
    , m_adcRawLabel(nullptr)
    , m_adcVoltageLabel(nullptr)
    , m_adcScaleLabel(nullptr)
    , m_sensorStackedWidget(nullptr)
    , m_modeSwitchButton(nullptr)
    , m_yAxisModeButton(nullptr)
    , m_isChartMode(false)
    , m_isFixedYAxis(true)  // 默认使用固定Y轴
    , m_chartView(nullptr)
    , m_chart(nullptr)
    , m_series(nullptr)
    , m_axisX(nullptr)
    , m_axisY(nullptr)
    , m_startTime(0)
    , m_maxDataPoints(60)  // 保留最近60个数据点（30秒的数据）
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
        // 创建模式切换按钮
        m_modeSwitchButton = new QPushButton("切换到图表模式", this);
        m_modeSwitchButton->setFixedHeight(45);
        m_modeSwitchButton->setStyleSheet(
            "QPushButton {"
            "   background-color: #FF9800;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 8px;"
            "   font-size: 16px;"
            "   font-weight: bold;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #F57C00;"
            "}"
        );
        connect(m_modeSwitchButton, &QPushButton::clicked, this, &AppDialog::switchSensorMode);
        
        // 创建堆叠窗口用于切换模式
        m_sensorStackedWidget = new QStackedWidget(this);
        
        // ====== 数据模式页面 ======
        QWidget *dataWidget = new QWidget(this);
        dataWidget->setStyleSheet("background-color: #f5f5f5;");
        
        QVBoxLayout *dataLayout = new QVBoxLayout(dataWidget);
        dataLayout->setContentsMargins(15, 15, 15, 15);
        dataLayout->setSpacing(12);
        
        // ADC 原始值
        QFrame *rawFrame = new QFrame(this);
        rawFrame->setStyleSheet("background-color: white; border-radius: 8px;");
        rawFrame->setMinimumHeight(90);
        QVBoxLayout *rawLayout = new QVBoxLayout(rawFrame);
        rawLayout->setContentsMargins(15, 10, 15, 10);
        rawLayout->setSpacing(5);
        QLabel *rawTitle = new QLabel("ADC 原始值", this);
        rawTitle->setStyleSheet("font-size: 13px; color: #888;");
        rawTitle->setFixedHeight(20);
        m_adcRawLabel = new QLabel("-- ", this);
        m_adcRawLabel->setStyleSheet("font-size: 26px; color: #2196F3; font-weight: bold;");
        m_adcRawLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        rawLayout->addWidget(rawTitle);
        rawLayout->addWidget(m_adcRawLabel);
        rawLayout->addStretch();
        
        // 电压值
        QFrame *voltageFrame = new QFrame(this);
        voltageFrame->setStyleSheet("background-color: white; border-radius: 8px;");
        voltageFrame->setMinimumHeight(90);
        QVBoxLayout *voltageLayout = new QVBoxLayout(voltageFrame);
        voltageLayout->setContentsMargins(15, 10, 15, 10);
        voltageLayout->setSpacing(5);
        QLabel *voltageTitle = new QLabel("电压值", this);
        voltageTitle->setStyleSheet("font-size: 13px; color: #888;");
        voltageTitle->setFixedHeight(20);
        m_adcVoltageLabel = new QLabel("-- V", this);
        m_adcVoltageLabel->setStyleSheet("font-size: 26px; color: #4CAF50; font-weight: bold;");
        m_adcVoltageLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        voltageLayout->addWidget(voltageTitle);
        voltageLayout->addWidget(m_adcVoltageLabel);
        voltageLayout->addStretch();
        
        // 比例系数
        QFrame *scaleFrame = new QFrame(this);
        scaleFrame->setStyleSheet("background-color: white; border-radius: 8px;");
        scaleFrame->setMinimumHeight(90);
        QVBoxLayout *scaleLayout = new QVBoxLayout(scaleFrame);
        scaleLayout->setContentsMargins(15, 10, 15, 10);
        scaleLayout->setSpacing(5);
        QLabel *scaleTitle = new QLabel("Scale 系数", this);
        scaleTitle->setStyleSheet("font-size: 13px; color: #888;");
        scaleTitle->setFixedHeight(20);
        m_adcScaleLabel = new QLabel("--", this);
        m_adcScaleLabel->setStyleSheet("font-size: 22px; color: #FF9800; font-weight: bold;");
        m_adcScaleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        scaleLayout->addWidget(scaleTitle);
        scaleLayout->addWidget(m_adcScaleLabel);
        scaleLayout->addStretch();
        
        dataLayout->addWidget(rawFrame);
        dataLayout->addWidget(voltageFrame);
        dataLayout->addWidget(scaleFrame);
        dataLayout->addStretch();
        
        // ====== 图表模式页面 ======
        QWidget *chartWidget = new QWidget(this);
        QVBoxLayout *chartLayout = new QVBoxLayout(chartWidget);
        chartLayout->setContentsMargins(5, 5, 5, 5);
        chartLayout->setSpacing(5);
        
        setupSensorChart();
        
        // 创建Y轴模式切换按钮
        m_yAxisModeButton = new QPushButton("Y轴: 固定 (0-4096)", this);
        m_yAxisModeButton->setFixedHeight(35);
        m_yAxisModeButton->setStyleSheet(
            "QPushButton {"
            "   background-color: #9C27B0;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 5px;"
            "   font-size: 13px;"
            "   font-weight: bold;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #7B1FA2;"
            "}"
        );
        connect(m_yAxisModeButton, &QPushButton::clicked, this, &AppDialog::toggleYAxisMode);
        
        chartLayout->addWidget(m_yAxisModeButton);
        chartLayout->addWidget(m_chartView);
        
        // 添加到堆叠窗口
        m_sensorStackedWidget->addWidget(dataWidget);  // 索引 0：数据模式
        m_sensorStackedWidget->addWidget(chartWidget); // 索引 1：图表模式
        m_sensorStackedWidget->setCurrentIndex(0);
        m_sensorStackedWidget->setMinimumHeight(300);  // 设置最小高度确保内容完整显示
        
        // 提示信息
        QLabel *infoLabel = new QLabel("数据每500ms更新一次", this);
        infoLabel->setAlignment(Qt::AlignCenter);
        infoLabel->setStyleSheet("font-size: 12px; color: #999;");
        infoLabel->setFixedHeight(25);
        
        layout->addWidget(m_modeSwitchButton);
        layout->addWidget(m_sensorStackedWidget, 1);  // 添加拉伸因子，让它占据剩余空间
        layout->addWidget(infoLabel);
        
        // 创建定时器更新传感器数据
        m_sensorTimer = new QTimer(this);
        connect(m_sensorTimer, &QTimer::timeout, this, &AppDialog::updateSensorData);
        m_sensorTimer->start(500);  // 500ms 更新一次
        
        // 记录起始时间
        m_startTime = QDateTime::currentMSecsSinceEpoch();
        
        // 立即读取一次数据
        updateSensorData();
    }
}

void AppDialog::setupSensorChart()
{
    // 创建图表
    m_chart = new QChart();
    m_chart->setTitle("ADC 原始值实时曲线");
    m_chart->setTitleFont(QFont("Arial", 12, QFont::Bold));
    m_chart->setAnimationOptions(QChart::NoAnimation);  // 禁用动画以提高性能
    m_chart->setMargins(QMargins(5, 5, 5, 5));  // 减小图表边距
    
    // 创建数据系列
    m_series = new QLineSeries();
    m_series->setName("ADC Raw");
    QPen pen(QColor("#2196F3"));
    pen.setWidth(2);
    m_series->setPen(pen);
    
    m_chart->addSeries(m_series);
    
    // 创建 X 轴（时间轴，单位：秒）
    m_axisX = new QValueAxis();
    m_axisX->setTitleText("时间 (秒)");
    m_axisX->setLabelFormat("%.0f");  // 改为整数显示，节省空间
    m_axisX->setRange(0, 30);  // 显示最近30秒
    m_axisX->setTickCount(7);
    m_axisX->setLabelsFont(QFont("Arial", 9));
    m_axisX->setTitleFont(QFont("Arial", 10));
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_series->attachAxis(m_axisX);
    
    // 创建 Y 轴（ADC 原始值）
    m_axisY = new QValueAxis();
    m_axisY->setTitleText("ADC");
    m_axisY->setLabelFormat("%d");
    m_axisY->setRange(0, 4096);  // 12位ADC，范围0-4095
    m_axisY->setTickCount(6);  // 减少刻度数量
    m_axisY->setLabelsFont(QFont("Arial", 9));
    m_axisY->setTitleFont(QFont("Arial", 10));
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisY);
    
    // 创建图表视图
    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMinimumHeight(280);  // 设置最小高度，确保图表有足够的显示空间
}

void AppDialog::switchSensorMode()
{
    m_isChartMode = !m_isChartMode;
    
    if (m_isChartMode) {
        // 切换到图表模式
        m_sensorStackedWidget->setCurrentIndex(1);
        m_modeSwitchButton->setText("切换到数据模式");
        m_modeSwitchButton->setStyleSheet(
            "QPushButton {"
            "   background-color: #4CAF50;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 8px;"
            "   font-size: 16px;"
            "   font-weight: bold;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #388E3C;"
            "}"
        );
    } else {
        // 切换到数据模式
        m_sensorStackedWidget->setCurrentIndex(0);
        m_modeSwitchButton->setText("切换到图表模式");
        m_modeSwitchButton->setStyleSheet(
            "QPushButton {"
            "   background-color: #FF9800;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 8px;"
            "   font-size: 16px;"
            "   font-weight: bold;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #F57C00;"
            "}"
        );
    }
}

void AppDialog::updateChartData(int rawValue)
{
    if (!m_series) return;
    
    // 计算相对时间（秒）
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    double timeInSeconds = (currentTime - m_startTime) / 1000.0;
    
    // 添加新数据点
    m_dataPoints.append(QPointF(timeInSeconds, rawValue));
    
    // 限制数据点数量
    if (m_dataPoints.size() > m_maxDataPoints) {
        m_dataPoints.removeFirst();
    }
    
    // 更新系列数据
    m_series->replace(m_dataPoints);
    
    // 动态调整 X 轴范围
    if (timeInSeconds > 30) {
        m_axisX->setRange(timeInSeconds - 30, timeInSeconds);
    }
    
    // 根据模式调整 Y 轴范围
    if (m_isFixedYAxis) {
        // 固定Y轴模式：始终 0-4096
        m_axisY->setRange(0, 4096);
    } else {
        // 自动Y轴模式：根据当前数据动态调整
        if (!m_dataPoints.isEmpty()) {
            int minVal = 4096, maxVal = 0;
            for (const QPointF &point : m_dataPoints) {
                int val = static_cast<int>(point.y());
                minVal = qMin(minVal, val);
                maxVal = qMax(maxVal, val);
            }
            
            // 添加10%的边距
            int margin = qMax(100, (maxVal - minVal) / 10);
            m_axisY->setRange(qMax(0, minVal - margin), qMin(4096, maxVal + margin));
        }
    }
}

void AppDialog::toggleYAxisMode()
{
    m_isFixedYAxis = !m_isFixedYAxis;
    
    if (m_isFixedYAxis) {
        m_yAxisModeButton->setText("Y轴: 固定 (0-4096)");
        m_yAxisModeButton->setStyleSheet(
            "QPushButton {"
            "   background-color: #9C27B0;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 5px;"
            "   font-size: 13px;"
            "   font-weight: bold;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #7B1FA2;"
            "}"
        );
        // 立即应用固定范围
        if (m_axisY) {
            m_axisY->setRange(0, 4096);
        }
    } else {
        m_yAxisModeButton->setText("Y轴: 自动");
        m_yAxisModeButton->setStyleSheet(
            "QPushButton {"
            "   background-color: #00BCD4;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 5px;"
            "   font-size: 13px;"
            "   font-weight: bold;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #0097A7;"
            "}"
        );
        // 立即重新计算范围
        if (!m_dataPoints.isEmpty() && m_axisY) {
            int minVal = 4096, maxVal = 0;
            for (const QPointF &point : m_dataPoints) {
                int val = static_cast<int>(point.y());
                minVal = qMin(minVal, val);
                maxVal = qMax(maxVal, val);
            }
            int margin = qMax(100, (maxVal - minVal) / 10);
            m_axisY->setRange(qMax(0, minVal - margin), qMin(4096, maxVal + margin));
        }
    }
}

void AppDialog::updateSensorData()
{
    int raw = 0;
    float scale = 0.0f;
    float voltage = 0.0f;
    
    int ret = readAdcData(raw, scale, voltage);
    
    if (ret == 0) {
        // 更新数据模式的显示
        m_adcRawLabel->setText(QString::number(raw));
        m_adcVoltageLabel->setText(QString::number(voltage, 'f', 3) + " V");
        m_adcScaleLabel->setText(QString::number(scale, 'f', 6));
        
        // 更新图表数据
        updateChartData(raw);
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
    m_contentLabel->setText("网络信息");
    m_contentLabel->setStyleSheet("font-size: 18px; color: #333; font-weight: bold;");
    
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(m_contentLabel->parentWidget()->layout());
    if (layout) {
        // 创建滚动区域以防止内容过多
        QScrollArea *scrollArea = new QScrollArea(this);
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setStyleSheet("QScrollArea { background-color: #f5f5f5; }");
        
        QWidget *contentWidget = new QWidget();
        contentWidget->setStyleSheet("background-color: #f5f5f5;");
        QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
        contentLayout->setContentsMargins(15, 15, 15, 15);
        contentLayout->setSpacing(10);
        
        // 获取所有网络接口
        QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
        
        for (const QNetworkInterface &interface : interfaces) {
            // 跳过回环接口
            if (interface.flags() & QNetworkInterface::IsLoopBack)
                continue;
            
            // 跳过未激活的接口
            if (!(interface.flags() & QNetworkInterface::IsUp))
                continue;
            
            // 创建接口信息卡片
            QFrame *ifaceFrame = new QFrame(this);
            ifaceFrame->setStyleSheet("background-color: white; border-radius: 8px;");
            QVBoxLayout *ifaceLayout = new QVBoxLayout(ifaceFrame);
            ifaceLayout->setContentsMargins(15, 15, 15, 15);
            ifaceLayout->setSpacing(8);
            
            // 接口名称
            QLabel *nameLabel = new QLabel(QString("<b>接口: %1</b>").arg(interface.name()), this);
            nameLabel->setStyleSheet("font-size: 16px; color: #2196F3;");
            ifaceLayout->addWidget(nameLabel);
            
            // MAC 地址
            QString mac = interface.hardwareAddress();
            if (!mac.isEmpty()) {
                QLabel *macLabel = new QLabel(QString("MAC 地址: %1").arg(mac), this);
                macLabel->setStyleSheet("font-size: 13px; color: #666;");
                ifaceLayout->addWidget(macLabel);
            }
            
            // 获取所有IP地址
            QList<QNetworkAddressEntry> entries = interface.addressEntries();
            for (const QNetworkAddressEntry &entry : entries) {
                QHostAddress ip = entry.ip();
                
                // 只显示 IPv4 地址
                if (ip.protocol() == QAbstractSocket::IPv4Protocol) {
                    QLabel *ipLabel = new QLabel(QString("IP 地址: <b>%1</b>").arg(ip.toString()), this);
                    ipLabel->setStyleSheet("font-size: 14px; color: #4CAF50;");
                    ifaceLayout->addWidget(ipLabel);
                    
                    QLabel *netmaskLabel = new QLabel(QString("子网掩码: %1").arg(entry.netmask().toString()), this);
                    netmaskLabel->setStyleSheet("font-size: 13px; color: #666;");
                    ifaceLayout->addWidget(netmaskLabel);
                    
                    QLabel *broadcastLabel = new QLabel(QString("广播地址: %1").arg(entry.broadcast().toString()), this);
                    broadcastLabel->setStyleSheet("font-size: 13px; color: #666;");
                    ifaceLayout->addWidget(broadcastLabel);
                }
            }
            
            // 接口状态
            QStringList flags;
            if (interface.flags() & QNetworkInterface::IsUp) flags << "运行中";
            if (interface.flags() & QNetworkInterface::IsRunning) flags << "激活";
            if (interface.flags() & QNetworkInterface::CanBroadcast) flags << "支持广播";
            if (interface.flags() & QNetworkInterface::CanMulticast) flags << "支持组播";
            
            if (!flags.isEmpty()) {
                QLabel *statusLabel = new QLabel(QString("状态: %1").arg(flags.join(", ")), this);
                statusLabel->setStyleSheet("font-size: 12px; color: #999;");
                ifaceLayout->addWidget(statusLabel);
            }
            
            contentLayout->addWidget(ifaceFrame);
        }
        
        // 如果没有找到有效的网络接口
        if (contentLayout->count() == 0) {
            QLabel *noNetLabel = new QLabel("未找到活动的网络接口", this);
            noNetLabel->setAlignment(Qt::AlignCenter);
            noNetLabel->setStyleSheet("font-size: 16px; color: #999; padding: 30px;");
            contentLayout->addWidget(noNetLabel);
        }
        
        contentLayout->addStretch();
        
        scrollArea->setWidget(contentWidget);
        layout->addWidget(scrollArea);
    }
}

QString AppDialog::getNetworkInfo()
{
    QString info;
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    
    for (const QNetworkInterface &interface : interfaces) {
        if (interface.flags() & QNetworkInterface::IsLoopBack)
            continue;
            
        if (!(interface.flags() & QNetworkInterface::IsUp))
            continue;
        
        info += QString("接口: %1\n").arg(interface.name());
        info += QString("MAC: %1\n").arg(interface.hardwareAddress());
        
        QList<QNetworkAddressEntry> entries = interface.addressEntries();
        for (const QNetworkAddressEntry &entry : entries) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                info += QString("IP: %1\n").arg(entry.ip().toString());
                info += QString("子网掩码: %1\n").arg(entry.netmask().toString());
                info += QString("广播: %1\n").arg(entry.broadcast().toString());
            }
        }
        info += "\n";
    }
    
    return info;
}

void AppDialog::createSettingsApp()
{
    m_contentLabel->setText("系统设置");
    m_contentLabel->setStyleSheet("font-size: 18px; color: #333; font-weight: bold;");
    
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(m_contentLabel->parentWidget()->layout());
    if (layout) {
        // 创建亮度调节区域
        QFrame *brightnessFrame = new QFrame(this);
        brightnessFrame->setStyleSheet("background-color: white; border-radius: 8px;");
        QVBoxLayout *brightnessLayout = new QVBoxLayout(brightnessFrame);
        brightnessLayout->setContentsMargins(20, 15, 20, 15);
        brightnessLayout->setSpacing(15);
        
        // 标题
        QLabel *titleLabel = new QLabel("屏幕亮度调节", this);
        titleLabel->setStyleSheet("font-size: 16px; color: #2196F3; font-weight: bold;");
        brightnessLayout->addWidget(titleLabel);
        
        // 亮度档位说明
        QLabel *infoLabel = new QLabel("共7个亮度档位 (1-7)：4, 8, 16, 32, 64, 128, 255", this);
        infoLabel->setStyleSheet("font-size: 12px; color: #999;");
        brightnessLayout->addWidget(infoLabel);
        
        // 当前亮度显示
        int currentLevel = getCurrentBrightness();  // 读取的是档位编号 1-7
        const int brightnessValues[] = {0, 4, 8, 16, 32, 64, 128, 255};  // 索引0不用，1-7对应档位1-7
        QString currentBrightnessText = (currentLevel >= 1 && currentLevel <= 7) 
            ? QString("当前档位: <b>%1</b> (亮度值: %2)").arg(currentLevel).arg(brightnessValues[currentLevel])
            : QString("当前档位: <b>未知</b>");
        QLabel *currentLabel = new QLabel(currentBrightnessText, this);
        currentLabel->setObjectName("currentBrightnessLabel");
        currentLabel->setStyleSheet("font-size: 14px; color: #4CAF50;");
        brightnessLayout->addWidget(currentLabel);
        
        // 创建按钮组用于选择亮度档位
        QWidget *buttonWidget = new QWidget(this);
        QGridLayout *buttonLayout = new QGridLayout(buttonWidget);
        buttonLayout->setSpacing(10);
        
        // 7个亮度档位（档位编号 1-7）
        const QString levelNames[] = {"1", "2", "3", "4", "5", "6", "7\n最亮"};
        
        for (int i = 0; i < 7; ++i) {
            int level = i + 1;  // 档位编号从1开始
            QPushButton *btn = new QPushButton(levelNames[i], this);
            btn->setFixedSize(80, 50);
            btn->setProperty("level", level);  // 存储档位编号 1-7
            btn->setProperty("brightness", brightnessValues[level]);  // 存储对应亮度值
            
            // 如果是当前档位，高亮显示
            if (level == currentLevel) {
                btn->setStyleSheet(
                    "QPushButton {"
                    "   background-color: #4CAF50;"
                    "   color: white;"
                    "   border: none;"
                    "   border-radius: 5px;"
                    "   font-size: 14px;"
                    "   font-weight: bold;"
                    "}"
                );
            } else {
                btn->setStyleSheet(
                    "QPushButton {"
                    "   background-color: #E0E0E0;"
                    "   color: #333;"
                    "   border: none;"
                    "   border-radius: 5px;"
                    "   font-size: 14px;"
                    "}"
                    "QPushButton:pressed {"
                    "   background-color: #BDBDBD;"
                    "}"
                );
            }
            
            // 连接信号
            connect(btn, &QPushButton::clicked, this, [this, brightnessValues, level, currentLabel]() {
                setBrightness(level);  // 传入档位编号 1-7
                currentLabel->setText(QString("当前档位: <b>%1</b> (亮度值: %2)").arg(level).arg(brightnessValues[level]));
                
                // 更新所有按钮的样式
                QWidget *buttonWidget = qobject_cast<QWidget*>(sender()->parent());
                if (buttonWidget) {
                    QList<QPushButton*> buttons = buttonWidget->findChildren<QPushButton*>();
                    for (QPushButton *b : buttons) {
                        int btnLevel = b->property("level").toInt();
                        if (btnLevel == level) {
                            b->setStyleSheet(
                                "QPushButton {"
                                "   background-color: #4CAF50;"
                                "   color: white;"
                                "   border: none;"
                                "   border-radius: 5px;"
                                "   font-size: 14px;"
                                "   font-weight: bold;"
                                "}"
                            );
                        } else {
                            b->setStyleSheet(
                                "QPushButton {"
                                "   background-color: #E0E0E0;"
                                "   color: #333;"
                                "   border: none;"
                                "   border-radius: 5px;"
                                "   font-size: 14px;"
                                "}"
                                "QPushButton:pressed {"
                                "   background-color: #BDBDBD;"
                                "}"
                            );
                        }
                    }
                }
            });
            
            // 网格布局：第一行4个，第二行3个
            buttonLayout->addWidget(btn, i / 4, i % 4);
        }
        
        brightnessLayout->addWidget(buttonWidget);
        
        // 添加说明
        QLabel *noteLabel = new QLabel(
            "提示：点击按钮调节屏幕亮度\n"
            "档位 1-7 对应亮度值 4, 8, 16, 32, 64, 128, 255\n"
            "档位1为最暗，档位7为最亮",
            this
        );
        noteLabel->setStyleSheet("font-size: 12px; color: #666; padding: 10px;");
        noteLabel->setWordWrap(true);
        brightnessLayout->addWidget(noteLabel);
        
        layout->addWidget(brightnessFrame);
        
        // 其他设置选项（占位）
        QFrame *otherFrame = new QFrame(this);
        otherFrame->setStyleSheet("background-color: white; border-radius: 8px;");
        QVBoxLayout *otherLayout = new QVBoxLayout(otherFrame);
        otherLayout->setContentsMargins(20, 15, 20, 15);
        
        QLabel *otherLabel = new QLabel("其他设置", this);
        otherLabel->setStyleSheet("font-size: 16px; color: #2196F3; font-weight: bold;");
        otherLayout->addWidget(otherLabel);
        
        QLabel *placeholderLabel = new QLabel("音量调节、语言设置、日期时间等功能待开发...", this);
        placeholderLabel->setStyleSheet("font-size: 13px; color: #999;");
        otherLayout->addWidget(placeholderLabel);
        
        layout->addWidget(otherFrame);
        layout->addStretch();
    }
}

int AppDialog::getCurrentBrightness()
{
    QString filePath = "/sys/devices/platform/backlight/backlight/backlight/brightness";
    QFile file(filePath);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open brightness file:" << filePath;
        return -1;
    }
    
    QTextStream in(&file);
    QString line = in.readLine().trimmed();
    file.close();
    
    bool ok;
    int level = line.toInt(&ok);  // 读取的是档位编号 1-7
    if (ok && level >= 1 && level <= 7) {
        qDebug() << "Current brightness level:" << level;
        return level;
    }
    
    return -1;
}

bool AppDialog::writeBrightness(int value)
{
    QString filePath = "/sys/devices/platform/backlight/backlight/backlight/brightness";
    QFile file(filePath);
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open brightness file for writing:" << filePath;
        return false;
    }
    
    QTextStream out(&file);
    out << value;  // 写入档位编号 1-7
    file.close();
    
    qDebug() << "Brightness level set to:" << value;
    return true;
}

void AppDialog::setBrightness(int level)
{
    // level 是档位编号 1-7
    if (level < 1 || level > 7) {
        qDebug() << "Invalid brightness level:" << level;
        return;
    }
    
    if (writeBrightness(level)) {
        qDebug() << "Successfully set brightness to level:" << level;
    } else {
        qDebug() << "Failed to set brightness to level:" << level;
        QMessageBox::warning(this, "错误", 
            QString("设置亮度失败！\n请确保有足够的权限访问:\n%1")
            .arg("/sys/devices/platform/backlight/backlight/backlight/brightness"));
    }
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
