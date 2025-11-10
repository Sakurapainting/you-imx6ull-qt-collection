#include "iconwidget.h"
#include <QPixmap>
#include <QPainter>
#include <QDebug>

IconWidget::IconWidget(const QString &iconPath, const QString &appName, QWidget *parent)
    : QWidget(parent)
    , m_appName(appName)
    , m_pressed(false)
{
    setupUI();
    setIcon(iconPath);
    setAppName(appName);
}

IconWidget::~IconWidget()
{
}

void IconWidget::setupUI()
{
    // 设置固定大小
    setFixedSize(100, 120);
    
    // 创建布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);
    
    // 创建图标标签
    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(80, 80);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setStyleSheet("QLabel { background-color: #4CAF50; border-radius: 15px; }");
    
    // 创建名称标签
    m_nameLabel = new QLabel(this);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setWordWrap(true);
    m_nameLabel->setStyleSheet("QLabel { color: #333; font-size: 12px; }");
    
    layout->addWidget(m_iconLabel, 0, Qt::AlignHCenter);
    layout->addWidget(m_nameLabel);
    
    setLayout(layout);
    
    // 设置鼠标追踪
    setMouseTracking(true);
}

void IconWidget::setIcon(const QString &iconPath)
{
    QPixmap pixmap(iconPath);
    if (pixmap.isNull()) {
        // 如果没有图标，创建一个默认图标
        QPixmap defaultPixmap(60, 60);
        defaultPixmap.fill(Qt::white);
        QPainter painter(&defaultPixmap);
        painter.setPen(QPen(Qt::white, 2));
        painter.drawRect(0, 0, 59, 59);
        m_iconLabel->setPixmap(defaultPixmap);
    } else {
        m_iconLabel->setPixmap(pixmap.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void IconWidget::setAppName(const QString &appName)
{
    m_appName = appName;
    m_nameLabel->setText(appName);
}

QString IconWidget::getAppName() const
{
    return m_appName;
}

void IconWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        // 添加按下效果
        setStyleSheet("IconWidget { background-color: rgba(0, 0, 0, 0.1); border-radius: 10px; }");
    }
    QWidget::mousePressEvent(event);
}

void IconWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_pressed) {
        m_pressed = false;
        setStyleSheet("");
        
        // 检查是否在控件范围内释放
        if (rect().contains(event->pos())) {
            qDebug() << "Icon clicked:" << m_appName;
            emit clicked(m_appName);
        }
    }
    QWidget::mouseReleaseEvent(event);
}

void IconWidget::enterEvent(QEvent *event)
{
    // 鼠标悬停效果（主要用于桌面版测试）
    if (!m_pressed) {
        setStyleSheet("IconWidget { background-color: rgba(0, 0, 0, 0.05); border-radius: 10px; }");
    }
    QWidget::enterEvent(event);
}

void IconWidget::leaveEvent(QEvent *event)
{
    if (!m_pressed) {
        setStyleSheet("");
    }
    QWidget::leaveEvent(event);
}
