#include "sliderwidget.h"
#include <QHBoxLayout>
#include <QScrollBar>
#include <QDebug>
#include <QTimer>

SliderWidget::SliderWidget(QWidget *parent)
    : QWidget(parent)
    , m_currentPage(0)
    , m_isSnapping(false)
{
    setupUI();
}

SliderWidget::~SliderWidget()
{
}

void SliderWidget::setupUI()
{
    // 创建主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 创建滚动区域
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setWidgetResizable(false);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    
    // 创建容器
    m_container = new QWidget();
    m_container->setStyleSheet("background-color: #f0f0f0;");
    
    m_scrollArea->setWidget(m_container);
    mainLayout->addWidget(m_scrollArea);
    
    // 启用触摸滑动
    m_scroller = QScroller::scroller(m_scrollArea->viewport());
    QScroller::grabGesture(m_scrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    
    // 设置滚动属性
    QScrollerProperties properties = m_scroller->scrollerProperties();
    properties.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, 
                              QScrollerProperties::OvershootAlwaysOff);
    properties.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, 
                              QScrollerProperties::OvershootAlwaysOff);
    properties.setScrollMetric(QScrollerProperties::OvershootDragResistanceFactor, 0.5);
    properties.setScrollMetric(QScrollerProperties::OvershootScrollDistanceFactor, 0.1);
    m_scroller->setScrollerProperties(properties);
    
    // 连接滚动状态变化信号
    connect(m_scroller, &QScroller::stateChanged, 
            this, &SliderWidget::onScrollerStateChanged);
}

void SliderWidget::addPage(QWidget *page)
{
    if (!page) return;
    
    page->setParent(m_container);
    m_pages.append(page);
    
    updatePagePositions();
}

void SliderWidget::setCurrentPage(int index)
{
    if (index < 0 || index >= m_pages.count()) return;
    if (index == m_currentPage) return;
    
    m_currentPage = index;
    snapToPage(index);
    emit pageChanged(index);
}

void SliderWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updatePagePositions();
}

void SliderWidget::updatePagePositions()
{
    if (m_pages.isEmpty()) return;
    
    int pageWidth = width();
    int pageHeight = height();
    
    // 设置容器大小
    m_container->setFixedSize(pageWidth * m_pages.count(), pageHeight);
    
    // 设置每一页的位置和大小
    for (int i = 0; i < m_pages.count(); ++i) {
        m_pages[i]->setGeometry(i * pageWidth, 0, pageWidth, pageHeight);
        m_pages[i]->show();
    }
    
    // 滚动到当前页
    m_scrollArea->horizontalScrollBar()->setValue(m_currentPage * pageWidth);
}

void SliderWidget::snapToPage(int pageIndex)
{
    if (pageIndex < 0 || pageIndex >= m_pages.count()) return;
    
    m_isSnapping = true;
    int targetX = pageIndex * width();
    
    // 使用动画滚动到目标页
    QPropertyAnimation *animation = new QPropertyAnimation(m_scrollArea->horizontalScrollBar(), "value");
    animation->setDuration(300);
    animation->setStartValue(m_scrollArea->horizontalScrollBar()->value());
    animation->setEndValue(targetX);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    
    connect(animation, &QPropertyAnimation::finished, this, [this]() {
        m_isSnapping = false;
    });
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void SliderWidget::onScrollerStateChanged(QScroller::State state)
{
    // 当滚动停止时，自动对齐到最近的页面
    if (state == QScroller::Inactive && !m_isSnapping) {
        QTimer::singleShot(50, this, &SliderWidget::updateCurrentPage);
    }
}

void SliderWidget::updateCurrentPage()
{
    if (m_isSnapping || m_pages.isEmpty()) return;
    
    int scrollX = m_scrollArea->horizontalScrollBar()->value();
    int pageWidth = width();
    
    // 计算最近的页面
    int nearestPage = qRound(static_cast<double>(scrollX) / pageWidth);
    nearestPage = qBound(0, nearestPage, m_pages.count() - 1);
    
    if (nearestPage != m_currentPage) {
        m_currentPage = nearestPage;
        emit pageChanged(m_currentPage);
    }
    
    // 对齐到页面
    snapToPage(nearestPage);
}
