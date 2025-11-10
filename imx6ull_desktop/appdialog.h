#ifndef APPDIALOG_H
#define APPDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>

class AppDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AppDialog(const QString &appName, QWidget *parent = nullptr);
    ~AppDialog();

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

private:
    QString m_appName;
    QLabel *m_titleLabel;
    QLabel *m_contentLabel;
    QPushButton *m_closeButton;
};

#endif // APPDIALOG_H
