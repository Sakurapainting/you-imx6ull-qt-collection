#ifndef BASEPAGE_H
#define BASEPAGE_H

#include <QWidget>

/**
 * @brief 功能页面基类
 * 所有功能页面都继承自这个基类
 */
class BasePage : public QWidget
{
    Q_OBJECT

public:
    explicit BasePage(QWidget *parent = nullptr);
    virtual ~BasePage();

    // 页面激活时调用（可选重写）
    virtual void onPageActivated() {}
    
    // 页面失活时调用（可选重写）
    virtual void onPageDeactivated() {}

signals:
    // 请求返回主页面
    void requestBackToHome();
};

#endif // BASEPAGE_H
