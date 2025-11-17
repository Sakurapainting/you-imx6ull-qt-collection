#include "camera.h"
#include <QDebug>

Camera::Camera(QWidget *parent)
    : QWidget(parent)
    , timer(nullptr)
    , cameraIndex(0)
    , videoCapture(nullptr)
{
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(readFrame()));
}

Camera::~Camera()
{
    if (videoCapture && videoCapture->isOpened()) {
        videoCapture->release();
        delete videoCapture;
        videoCapture = nullptr;
    }
}

bool Camera::cameraProcess(bool open)
{
    if (open) {
        /* 使用 OpenCV */
        if (!videoCapture) {
            videoCapture = new cv::VideoCapture(cameraIndex);
        }
        
        if (!videoCapture->isOpened()) {
            qDebug() << "无法打开摄像头" << cameraIndex;
            return false;
        }
        
        /* 设置摄像头参数 */
        videoCapture->set(cv::CAP_PROP_FRAME_WIDTH, 640);
        videoCapture->set(cv::CAP_PROP_FRAME_HEIGHT, 480);
        
        timer->start(33);  /* 30fps */
        qDebug() << "摄像头已打开（OpenCV）:" << cameraIndex;
        return true;
    } else {
        timer->stop();
        if (videoCapture && videoCapture->isOpened()) {
            videoCapture->release();
        }
        qDebug() << "摄像头已关闭";
        return true;
    }
}

void Camera::selectCameraDevice(int index)
{
    /* 如果正在使用，先关闭 */
    bool wasOpen = false;
    if (videoCapture && videoCapture->isOpened()) {
        wasOpen = true;
        cameraProcess(false);
    }
    
    cameraIndex = index;
    qDebug() << "切换到摄像头" << cameraIndex;
    
    /* 如果之前是打开的，重新打开 */
    if (wasOpen) {
        cameraProcess(true);
    }
}

void Camera::readFrame()
{
    if (!videoCapture || !videoCapture->isOpened()) {
        return;
    }
    
    cv::Mat frame;
    if (!videoCapture->read(frame)) {
        qDebug() << "读取帧失败";
        return;
    }
    
    if (frame.empty()) {
        return;
    }
    
    /* 转换为 RGB 格式 */
    cv::Mat rgbFrame;
    cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);
    
    /* 转换为 QImage */
    QImage image(rgbFrame.data, 
                 rgbFrame.cols, 
                 rgbFrame.rows, 
                 rgbFrame.step,
                 QImage::Format_RGB888);
    
    /* 发送图像副本 */
    emit readyImage(image.copy());
}
