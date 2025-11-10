#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>

/**
 * @brief 主页面 - 显示功能菜单
 */
class HomePage : public QWidget
{
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);
    ~HomePage();

signals:
    // 当功能按钮被点击时发出信号，参数为页面索引
    void functionButtonClicked(int pageIndex);

private slots:
    void onButtonClicked();

private:
    void setupUI();
    QPushButton* createFunctionButton(const QString &text, const QString &icon = "");

    QScrollArea *scrollArea;
    QWidget *contentWidget;
    QVBoxLayout *mainLayout;
};

#endif // HOMEPAGE_H
