#ifndef ICONWIDGET_H
#define ICONWIDGET_H

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QVBoxLayout>
#include <QMouseEvent>

class IconWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IconWidget(const QString &iconPath, const QString &appName, QWidget *parent = nullptr);
    ~IconWidget();

    void setIcon(const QString &iconPath);
    void setAppName(const QString &appName);
    QString getAppName() const;

signals:
    void clicked(const QString &appName);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void setupUI();

private:
    QLabel *m_iconLabel;
    QLabel *m_nameLabel;
    QString m_appName;
    bool m_pressed;
};

#endif // ICONWIDGET_H
