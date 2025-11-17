#ifndef FACERECOGNITION_H
#define FACERECOGNITION_H

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QMap>
#include <QVector>

/* 前向声明 */
class Camera;
class VirtualKeyboard;

#ifdef __arm__
namespace seeta {
    namespace v2 {
        class FaceDetector;
        class FaceLandmarker;
        class FaceRecognizer;
    }
}
struct SeetaImageData;
struct SeetaFaceInfoArray;
#endif

class FaceRecognition : public QWidget
{
    Q_OBJECT

public:
    explicit FaceRecognition(QWidget *parent = nullptr);
    ~FaceRecognition();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void receiveImage(const QImage &image);
    void openCameraButtonClicked();
    void faceDetectButtonClicked();
    void registerFaceButtonClicked();
    void faceRecognizeButtonClicked();
    void onCameraDeviceChanged(const QString &deviceName);
    void showVirtualKeyboard();

private:
    void layoutInit();
    void scanCameraDevice();
    void initFaceDetector();
    QImage drawFaceRect(const QImage &image);
    bool extractFaceFeature(const QImage &image, QVector<float> &feature);
    QString recognizeFace(const QVector<float> &feature);
    void saveFaceDatabase();
    void loadFaceDatabase();

private:
    /* UI 组件 */
    QWidget *rightWidget;
    QComboBox *comboBox;
    QPushButton *openCameraButton;
    QPushButton *faceDetectButton;
    QPushButton *registerFaceButton;
    QPushButton *faceRecognizeButton;
    QLineEdit *nameLineEdit;
    QLabel *infoLabel;
    QScrollArea *scrollArea;
    QLabel *displayLabel;
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout;

    /* 摄像头 */
    Camera *camera;
    VirtualKeyboard *virtualKeyboard;

    /* 当前帧 */
    QImage currentFrame;

    /* 人脸检测器 */
#ifdef __arm__
    seeta::v2::FaceDetector *faceDetector;
    seeta::v2::FaceLandmarker *faceLandmarker;
    seeta::v2::FaceRecognizer *faceRecognizer;
#else
    void *faceDetector;
    void *faceLandmarker;
    void *faceRecognizer;
#endif

    /* 人脸数据库 */
    QMap<QString, QVector<float>> faceDatabase;

    /* 状态标志 */
    bool faceDetectEnabled;
    bool faceRecognizeEnabled;
};

#endif // FACERECOGNITION_H
