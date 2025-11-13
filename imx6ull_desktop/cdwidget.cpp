#include "cdwidget.h"

CDWidget::CDWidget(QWidget *parent)
    : QWidget(parent)
    , m_rotationTimer(nullptr)
    , m_rotationAngle(0)
    , m_isRotating(false)
{
    // 加载CD图片
    m_cdImage.load(":/image/cd.png");
    
    // 初始化旋转定时器
    m_rotationTimer = new QTimer(this);
    connect(m_rotationTimer, &QTimer::timeout, this, &CDWidget::rotateCD);
    
    // 设置固定大小
    setFixedSize(280, 280);
}

void CDWidget::startRotation()
{
    m_isRotating = true;
    m_rotationTimer->start(50); // 每50ms旋转一次
}

void CDWidget::stopRotation()
{
    m_isRotating = false;
    m_rotationTimer->stop();
}

void CDWidget::resetRotation()
{
    m_isRotating = false;
    m_rotationTimer->stop();
    m_rotationAngle = 0;
    update();
}

void CDWidget::rotateCD()
{
    // 每次增加角度
    if (m_rotationAngle >= 360)
        m_rotationAngle = 0;
    
    m_rotationAngle += 3; // 每次旋转3度
    
    // 触发重绘
    update();
}

void CDWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    if (m_cdImage.isNull()) {
        return;
    }
    
    QPainter painter(this);
    
    // 设置抗锯齿，使旋转更流畅
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    
    // CD的矩形区域
    QRectF rect(0, 0, width(), height());
    
    // 设置旋转中心为CD的中心
    painter.translate(rect.width() / 2, rect.height() / 2);
    
    // 旋转
    painter.rotate(m_rotationAngle);
    
    // 恢复坐标系
    painter.translate(-rect.width() / 2, -rect.height() / 2);
    
    // 绘制CD图片
    painter.drawPixmap(rect.toRect(), m_cdImage);
}
