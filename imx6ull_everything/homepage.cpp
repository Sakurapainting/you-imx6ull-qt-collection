#include "homepage.h"
#include <QDebug>

HomePage::HomePage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

HomePage::~HomePage()
{
}

void HomePage::setupUI()
{
    // 主布局
    QVBoxLayout *pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    // 标题
    QLabel *titleLabel = new QLabel("IMX6ULL 功能演示", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   color: #2c3e50;"
        "   background-color: #3498db;"
        "   color: white;"
        "   padding: 20px;"
        "}"
    );
    pageLayout->addWidget(titleLabel);

    // 滚动区域
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: #ecf0f1; }");

    // 内容widget
    contentWidget = new QWidget();
    contentWidget->setStyleSheet("background-color: #ecf0f1;");
    
    mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 添加功能按钮（示例）
    // 注意：这里的索引要与MainWindow中的页面索引对应
    mainLayout->addWidget(createFunctionButton("🔆 LED 控制", "1"));
    mainLayout->addWidget(createFunctionButton("📷 摄像头", "2"));
    mainLayout->addWidget(createFunctionButton("🌡️ 温度传感器", "3"));
    mainLayout->addWidget(createFunctionButton("📊 数据图表", "4"));
    mainLayout->addWidget(createFunctionButton("🔊 音频播放", "5"));
    mainLayout->addWidget(createFunctionButton("📡 网络通信", "6"));
    mainLayout->addWidget(createFunctionButton("💾 文件系统", "7"));
    mainLayout->addWidget(createFunctionButton("⚙️ 系统信息", "8"));

    // 添加弹簧，将按钮推到顶部
    mainLayout->addStretch();

    scrollArea->setWidget(contentWidget);
    pageLayout->addWidget(scrollArea);

    setLayout(pageLayout);
}

QPushButton* HomePage::createFunctionButton(const QString &text, const QString &icon)
{
    QPushButton *button = new QPushButton(text, contentWidget);
    button->setMinimumHeight(60);
    button->setCursor(Qt::PointingHandCursor);
    button->setProperty("pageIndex", icon.toInt());
    
    button->setStyleSheet(
        "QPushButton {"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   color: #2c3e50;"
        "   background-color: white;"
        "   border: 2px solid #bdc3c7;"
        "   border-radius: 8px;"
        "   padding: 10px;"
        "   text-align: left;"
        "   padding-left: 20px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #3498db;"
        "   color: white;"
        "   border-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #2980b9;"
        "}"
    );

    connect(button, &QPushButton::clicked, this, &HomePage::onButtonClicked);

    return button;
}

void HomePage::onButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (button) {
        int pageIndex = button->property("pageIndex").toInt();
        qDebug() << "Function button clicked, page index:" << pageIndex;
        emit functionButtonClicked(pageIndex);
    }
}
