#ifndef APPDIALOG_H
#define APPDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QStackedWidget>
#include <QVector>
#include <QDateTime>

QT_CHARTS_USE_NAMESPACE

class AppDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AppDialog(const QString &appName, QWidget *parent = nullptr);
    ~AppDialog();

private slots:
    void updateSensorData();
    void switchSensorMode();
    void toggleYAxisMode();

private:
    void setupUI(const QString &appName);
    void createLEDApp();
    void createSensorApp();
    void createNetworkApp();
    void createSettingsApp();
    void createMediaApp();
    void createFileApp();
    void createSystemApp();
    void createAboutApp();
    
    // ADC 读取相关
    int readAdcData(int &raw, float &scale, float &voltage);
    QString readFileContent(const QString &filePath);
    
    // 传感器图表相关
    void setupSensorChart();
    void updateChartData(int rawValue);

private:
    QString m_appName;
    QLabel *m_titleLabel;
    QLabel *m_contentLabel;
    QPushButton *m_closeButton;
    
    // 传感器相关
    QTimer *m_sensorTimer;
    QLabel *m_adcRawLabel;
    QLabel *m_adcVoltageLabel;
    QLabel *m_adcScaleLabel;
    
    // 传感器模式切换
    QStackedWidget *m_sensorStackedWidget;
    QPushButton *m_modeSwitchButton;
    QPushButton *m_yAxisModeButton;
    bool m_isChartMode;
    bool m_isFixedYAxis;
    
    // 图表相关
    QChartView *m_chartView;
    QChart *m_chart;
    QLineSeries *m_series;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    QVector<QPointF> m_dataPoints;
    qint64 m_startTime;
    int m_maxDataPoints;
};

#endif // APPDIALOG_H
