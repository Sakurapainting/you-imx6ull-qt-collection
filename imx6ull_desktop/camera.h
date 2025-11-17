#ifndef CAMERA_H
#define CAMERA_H

#include <QWidget>
#include <QTimer>
#include <QImage>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class Camera : public QWidget
{
    Q_OBJECT

public:
    explicit Camera(QWidget *parent = nullptr);
    ~Camera();

    /* 开启或关闭摄像头 */
    bool cameraProcess(bool open);
    
    /* 选择摄像头设备 */
    void selectCameraDevice(int index);

signals:
    /* 准备好的图片 */
    void readyImage(QImage image);

private slots:
    /* 读取摄像头数据 */
    void readFrame();

private:
    QTimer *timer;
    int cameraIndex;  /* 当前使用的摄像头索引 */
    cv::VideoCapture *videoCapture;
};

#endif // CAMERA_H
