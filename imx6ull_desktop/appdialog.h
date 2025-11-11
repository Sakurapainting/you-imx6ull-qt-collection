#ifndef APPDIALOG_H
#define APPDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

class AppDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AppDialog(const QString &appName, QWidget *parent = nullptr);
    ~AppDialog();

private slots:
    void updateSensorData();

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
};

#endif // APPDIALOG_H
