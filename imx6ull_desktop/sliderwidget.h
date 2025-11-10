#ifndef SLIDERWIDGET_H
#define SLIDERWIDGET_H

#include <QWidget>
#include <QList>
#include <QScrollArea>
#include <QPropertyAnimation>
#include <QScroller>

class SliderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SliderWidget(QWidget *parent = nullptr);
    ~SliderWidget();

    // 添加一页内容
    void addPage(QWidget *page);
    
    // 设置当前页
    void setCurrentPage(int index);
    
    // 获取当前页索引
    int currentPage() const { return m_currentPage; }
    
    // 获取总页数
    int pageCount() const { return m_pages.count(); }

signals:
    void pageChanged(int index);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onScrollerStateChanged(QScroller::State state);
    void updateCurrentPage();

private:
    void setupUI();
    void updatePagePositions();
    void snapToPage(int pageIndex);

private:
    QScrollArea *m_scrollArea;
    QWidget *m_container;
    QList<QWidget*> m_pages;
    int m_currentPage;
    QScroller *m_scroller;
    bool m_isSnapping;
};

#endif // SLIDERWIDGET_H
