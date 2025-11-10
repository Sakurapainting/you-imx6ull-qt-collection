#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "sliderwidget.h"
#include <QLabel>
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onIconClicked(const QString &appName);
    void onPageChanged(int index);

private:
    void setupUI();
    void createPages();
    QWidget* createPage(int pageIndex);
    void updatePageIndicator();

private:
    Ui::MainWindow *ui;
    SliderWidget *m_sliderWidget;
    QLabel *m_pageIndicator;
    QLabel *m_statusBar;
    
    // 应用图标数据
    struct AppInfo {
        QString name;
        QString iconPath;
    };
    QList<AppInfo> m_apps;
};
#endif // MAINWINDOW_H
