#include "ledpage.h"
#include <QDebug>

LedPage::LedPage(QWidget *parent)
    : BasePage(parent)
{
    setupUI();
    qDebug() << "LedPage created (lazy loading)";
}

LedPage::~LedPage()
{
    qDebug() << "LedPage destroyed";
}

void LedPage::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(20);
    layout->setContentsMargins(20, 20, 20, 20);

    // 标题
    QLabel *titleLabel = new QLabel("LED 控制", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 22px; font-weight: bold; color: #2c3e50; padding: 10px;"
    );
    layout->addWidget(titleLabel);

    // 状态标签
    statusLabel = new QLabel("LED 状态: 未知", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet(
        "font-size: 18px; padding: 15px; "
        "background-color: #ecf0f1; border-radius: 8px;"
    );
    layout->addWidget(statusLabel);

    // LED ON 按钮
    ledOnButton = new QPushButton("打开 LED", this);
    ledOnButton->setMinimumHeight(50);
    ledOnButton->setStyleSheet(
        "QPushButton {"
        "   font-size: 16px; background-color: #2ecc71; color: white;"
        "   border-radius: 8px; font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #27ae60; }"
        "QPushButton:pressed { background-color: #229954; }"
    );
    connect(ledOnButton, &QPushButton::clicked, this, [this]() {
        statusLabel->setText("LED 状态: 开启 ✓");
        statusLabel->setStyleSheet(
            "font-size: 18px; padding: 15px; "
            "background-color: #2ecc71; color: white; "
            "border-radius: 8px; font-weight: bold;"
        );
        // TODO: 实际的LED控制代码
        qDebug() << "LED ON";
    });
    layout->addWidget(ledOnButton);

    // LED OFF 按钮
    ledOffButton = new QPushButton("关闭 LED", this);
    ledOffButton->setMinimumHeight(50);
    ledOffButton->setStyleSheet(
        "QPushButton {"
        "   font-size: 16px; background-color: #e74c3c; color: white;"
        "   border-radius: 8px; font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #c0392b; }"
        "QPushButton:pressed { background-color: #a93226; }"
    );
    connect(ledOffButton, &QPushButton::clicked, this, [this]() {
        statusLabel->setText("LED 状态: 关闭 ✗");
        statusLabel->setStyleSheet(
            "font-size: 18px; padding: 15px; "
            "background-color: #95a5a6; color: white; "
            "border-radius: 8px; font-weight: bold;"
        );
        // TODO: 实际的LED控制代码
        qDebug() << "LED OFF";
    });
    layout->addWidget(ledOffButton);

    layout->addStretch();

    // 返回按钮
    backButton = new QPushButton("← 返回主页", this);
    backButton->setMinimumHeight(50);
    backButton->setStyleSheet(
        "QPushButton {"
        "   font-size: 16px; background-color: #95a5a6; color: white;"
        "   border-radius: 8px; font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #7f8c8d; }"
    );
    connect(backButton, &QPushButton::clicked, this, &LedPage::requestBackToHome);
    layout->addWidget(backButton);

    setLayout(layout);
    setStyleSheet("background-color: white;");
}

void LedPage::onPageActivated()
{
    qDebug() << "LedPage activated";
    // 页面激活时的初始化操作
}

void LedPage::onPageDeactivated()
{
    qDebug() << "LedPage deactivated";
    // 页面失活时的清理操作
}
