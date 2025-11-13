#ifndef CDWIDGET_H
#define CDWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QTimer>

class CDWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CDWidget(QWidget *parent = nullptr);
    
    void startRotation();
    void stopRotation();
    void resetRotation();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void rotateCD();

private:
    QTimer *m_rotationTimer;
    int m_rotationAngle;
    bool m_isRotating;
    QPixmap m_cdImage;
};

#endif // CDWIDGET_H
