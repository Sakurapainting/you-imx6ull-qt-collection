#ifndef LEDPAGE_H
#define LEDPAGE_H

#include "basepage.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

/**
 * @brief LED控制页面示例
 */
class LedPage : public BasePage
{
    Q_OBJECT

public:
    explicit LedPage(QWidget *parent = nullptr);
    ~LedPage();

    void onPageActivated() override;
    void onPageDeactivated() override;

private:
    void setupUI();

    QPushButton *backButton;
    QPushButton *ledOnButton;
    QPushButton *ledOffButton;
    QLabel *statusLabel;
};

#endif // LEDPAGE_H
