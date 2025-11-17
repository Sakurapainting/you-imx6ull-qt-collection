#include "facerecognition.h"
#include "camera.h"
#include "virtualkeyboard.h"

#include <QGuiApplication>
#include <QScreen>
#include <QDebug>
#include <QPainter>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QDataStream>
#include <QEvent>
#include <QHeaderView>
#include <QDateTime>

#ifdef __arm__
/* OpenCV 头文件 */
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

/* SeetaFace 头文件 */
#include "seeta/FaceDetector.h"
#include "seeta/FaceLandmarker.h"
#include "seeta/FaceRecognizer.h"
#endif

FaceRecognition::FaceRecognition(QWidget *parent)
    : QWidget(parent)
    , virtualKeyboard(nullptr)
    , faceDetector(nullptr)
    , faceLandmarker(nullptr)
    , faceRecognizer(nullptr)
    , faceDetectEnabled(false)
    , faceRecognizeEnabled(false)
{
    layoutInit();
    scanCameraDevice();
    initFaceDetector();
    loadFaceDatabase();

    /* 初始化虚拟键盘 */
    virtualKeyboard = new VirtualKeyboard(this);
    virtualKeyboard->setLineEdit(nameLineEdit);
    virtualKeyboard->hide();
}

FaceRecognition::~FaceRecognition()
{
#ifdef __arm__
    if (faceDetector) {
        delete faceDetector;
        faceDetector = nullptr;
    }
    if (faceLandmarker) {
        delete faceLandmarker;
        faceLandmarker = nullptr;
    }
    if (faceRecognizer) {
        delete faceRecognizer;
        faceRecognizer = nullptr;
    }
#endif
    if (virtualKeyboard) {
        delete virtualKeyboard;
        virtualKeyboard = nullptr;
    }
}

void FaceRecognition::layoutInit()
{
    /* 创建控件 */
    rightWidget = new QWidget();
    comboBox = new QComboBox();
    openCameraButton = new QPushButton();
    faceDetectButton = new QPushButton();
    registerFaceButton = new QPushButton();
    faceRecognizeButton = new QPushButton();
    manageDatabaseButton = new QPushButton();
    nameLineEdit = new QLineEdit();
    infoLabel = new QLabel();
    scrollArea = new QScrollArea();
    displayLabel = new QLabel(scrollArea);
    vboxLayout = new QVBoxLayout();
    hboxLayout = new QHBoxLayout();

    /* 创建堆叠窗口 */
    stackedWidget = new QStackedWidget();
    
    /* 创建摄像头视图 */
    cameraWidget = new QWidget();
    QVBoxLayout *cameraLayout = new QVBoxLayout(cameraWidget);
    cameraLayout->setContentsMargins(0, 0, 0, 0);
    cameraLayout->addWidget(scrollArea);
    
    /* 创建数据库管理视图 */
    createDatabaseManagementUI();
    
    /* 添加到堆叠窗口 */
    stackedWidget->addWidget(cameraWidget);      // 索引0：摄像头视图
    stackedWidget->addWidget(databaseWidget);    // 索引1：数据库管理视图
    stackedWidget->setCurrentIndex(0);

    /* 设置右侧控制面板布局 */
    vboxLayout->setContentsMargins(10, 10, 10, 10);
    vboxLayout->setSpacing(10);
    vboxLayout->addWidget(comboBox);
    vboxLayout->addWidget(openCameraButton);
    vboxLayout->addWidget(faceDetectButton);
    vboxLayout->addWidget(nameLineEdit);
    vboxLayout->addWidget(registerFaceButton);
    vboxLayout->addWidget(faceRecognizeButton);
    vboxLayout->addWidget(manageDatabaseButton);
    vboxLayout->addWidget(infoLabel);
    vboxLayout->addStretch();

    rightWidget->setLayout(vboxLayout);

    /* 设置主布局 */
    hboxLayout->setContentsMargins(0, 0, 0, 0);
    hboxLayout->setSpacing(10);
    hboxLayout->addWidget(stackedWidget);
    hboxLayout->addWidget(rightWidget);
    
    /* 将主布局设置给 FaceRecognition 本身 */
    this->setLayout(hboxLayout);

    openCameraButton->setMaximumHeight(40);
    openCameraButton->setMaximumWidth(200);

    faceDetectButton->setMaximumHeight(40);
    faceDetectButton->setMaximumWidth(200);

    registerFaceButton->setMaximumHeight(40);
    registerFaceButton->setMaximumWidth(200);

    faceRecognizeButton->setMaximumHeight(40);
    faceRecognizeButton->setMaximumWidth(200);
    
    manageDatabaseButton->setMaximumHeight(40);
    manageDatabaseButton->setMaximumWidth(200);

    nameLineEdit->setMaximumHeight(30);
    nameLineEdit->setMaximumWidth(200);
    nameLineEdit->setPlaceholderText("请输入姓名");
    nameLineEdit->setReadOnly(true);  /* 设置为只读，防止系统键盘弹出 */

    infoLabel->setMaximumWidth(200);
    infoLabel->setWordWrap(true);
    infoLabel->setAlignment(Qt::AlignTop);
    infoLabel->setText("待机中...");

    comboBox->setMaximumHeight(40);
    comboBox->setMaximumWidth(200);
    
    /* 设置滚动区域 */
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumWidth(400);

    /* 显示图像 */
    displayLabel->setMinimumSize(480, 360);
    displayLabel->setScaledContents(true);
    scrollArea->setWidget(displayLabel);

    /* 居中显示 */
    scrollArea->setAlignment(Qt::AlignCenter);

    /* 设置按钮文本 */
    openCameraButton->setText("开启摄像头");
    openCameraButton->setCheckable(true);
    
    faceDetectButton->setText("开启人脸检测");
    faceDetectButton->setCheckable(true);
    faceDetectButton->setEnabled(false);

    registerFaceButton->setText("录入人脸");
    registerFaceButton->setEnabled(false);

    faceRecognizeButton->setText("开启人脸识别");
    faceRecognizeButton->setCheckable(true);
    faceRecognizeButton->setEnabled(false);
    
    manageDatabaseButton->setText("数据库管理");
    manageDatabaseButton->setCheckable(true);

    /* 摄像头 */
    camera = new Camera(this);

    /* 连接信号和槽 */
    connect(camera, SIGNAL(readyImage(QImage)),
            this, SLOT(receiveImage(QImage)));
    connect(openCameraButton, SIGNAL(clicked()),
            this, SLOT(openCameraButtonClicked()));
    connect(faceDetectButton, SIGNAL(clicked()),
            this, SLOT(faceDetectButtonClicked()));
    connect(registerFaceButton, SIGNAL(clicked()),
            this, SLOT(registerFaceButtonClicked()));
    connect(faceRecognizeButton, SIGNAL(clicked()),
            this, SLOT(faceRecognizeButtonClicked()));
    connect(manageDatabaseButton, SIGNAL(clicked()),
            this, SLOT(manageDatabaseButtonClicked()));
    connect(comboBox, SIGNAL(currentTextChanged(QString)),
            this, SLOT(onCameraDeviceChanged(QString)));
    
    /* 使用事件过滤器来捕获点击事件 */
    nameLineEdit->installEventFilter(this);
}

bool FaceRecognition::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == nameLineEdit) {
        if (event->type() == QEvent::MouseButtonPress) {
            showVirtualKeyboard();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void FaceRecognition::scanCameraDevice()
{
    /* 扫描 /dev 目录下的 video* 设备 */
    QDir devDir("/dev");
    QStringList filters;
    filters << "video*";
    
    QStringList videoDevices = devDir.entryList(filters, QDir::System, QDir::Name);
    
    if (videoDevices.isEmpty()) {
        /* 如果没有找到设备，添加默认选项 */
        comboBox->addItem("未找到摄像头设备");
        qDebug() << "未在 /dev 目录下找到 video 设备";
    } else {
        /* 添加找到的设备 */
        for (int i = 0; i < videoDevices.size(); i++) {
            comboBox->addItem(videoDevices.at(i));
            qDebug() << "找到摄像头设备: /dev/" << videoDevices.at(i);
        }
    }
}

void FaceRecognition::initFaceDetector()
{
#ifdef __arm__
    /* 初始化人脸检测器 */
    seeta::ModelSetting fdSetting;
    fdSetting.append("/model/fd_2_00.dat");
    fdSetting.set_device(seeta::ModelSetting::CPU);
    faceDetector = new seeta::v2::FaceDetector(fdSetting);
    faceDetector->set(seeta::v2::FaceDetector::PROPERTY_MIN_FACE_SIZE, 40);
    qDebug() << "人脸检测器初始化完成";

    /* 初始化人脸特征点检测器 */
    seeta::ModelSetting pdSetting;
    pdSetting.append("/model/pd_2_00_pts5.dat");
    pdSetting.set_device(seeta::ModelSetting::CPU);
    faceLandmarker = new seeta::v2::FaceLandmarker(pdSetting);
    qDebug() << "人脸特征点检测器初始化完成";

    /* 初始化人脸识别器 */
    seeta::ModelSetting frSetting;
    frSetting.append("/model/fr_2_10.dat");
    frSetting.set_device(seeta::ModelSetting::CPU);
    faceRecognizer = new seeta::v2::FaceRecognizer(frSetting);
    qDebug() << "人脸识别器初始化完成";
#else
    qDebug() << "非ARM平台，人脸识别功能不可用";
#endif
}

void FaceRecognition::receiveImage(const QImage &image)
{
    /* 保存当前帧，用于录入 */
    currentFrame = image;

    if (faceDetectEnabled || faceRecognizeEnabled) {
        /* 进行人脸检测并绘制人脸框 */
        QImage processedImage = drawFaceRect(image);
        displayLabel->setPixmap(QPixmap::fromImage(processedImage));
    } else {
        /* 直接显示图像 */
        displayLabel->setPixmap(QPixmap::fromImage(image));
    }
}

void FaceRecognition::openCameraButtonClicked()
{
    if (openCameraButton->isChecked()) {
        /* 开启摄像头 */
        if (camera->cameraProcess(true)) {
            openCameraButton->setText("关闭摄像头");
            faceDetectButton->setEnabled(true);
            registerFaceButton->setEnabled(true);
            faceRecognizeButton->setEnabled(true);
        } else {
            openCameraButton->setChecked(false);
            qDebug() << "摄像头打开失败";
        }
    } else {
        /* 关闭摄像头 */
        camera->cameraProcess(false);
        openCameraButton->setText("开启摄像头");
        faceDetectButton->setEnabled(false);
        faceDetectButton->setChecked(false);
        faceDetectEnabled = false;
        registerFaceButton->setEnabled(false);
        faceRecognizeButton->setEnabled(false);
        faceRecognizeButton->setChecked(false);
        faceRecognizeEnabled = false;
    }
}

void FaceRecognition::faceDetectButtonClicked()
{
    if (faceDetectButton->isChecked()) {
        faceDetectEnabled = true;
        faceDetectButton->setText("关闭人脸检测");
    } else {
        faceDetectEnabled = false;
        faceDetectButton->setText("开启人脸检测");
    }
}

void FaceRecognition::onCameraDeviceChanged(const QString &deviceName)
{
    /* 从设备名称中提取编号，例如 "video0" -> 0 */
    QString numStr = deviceName;
    numStr.remove("video");
    
    bool ok;
    int deviceIndex = numStr.toInt(&ok);
    
    if (ok) {
        camera->selectCameraDevice(deviceIndex);
    } else {
        qDebug() << "无效的摄像头设备名称:" << deviceName;
    }
}

QImage FaceRecognition::drawFaceRect(const QImage &image)
{
#ifdef __arm__
    if (!faceDetector) {
        return image;
    }

    /* 创建QImage的副本用于绘制 */
    QImage resultImage = image.copy();

    /* 将QImage转换为SeetaImageData */
    QImage rgbImage = resultImage.convertToFormat(QImage::Format_RGB888);
    
    SeetaImageData seetaImg;
    seetaImg.width = rgbImage.width();
    seetaImg.height = rgbImage.height();
    seetaImg.channels = 3;
    seetaImg.data = rgbImage.bits();

    /* 检测人脸 */
    SeetaFaceInfoArray faces = faceDetector->detect(seetaImg);

    /* 在图像上绘制人脸框 */
    QPainter painter(&resultImage);
    painter.setPen(QPen(Qt::green, 2));

    for (int i = 0; i < faces.size; i++) {
        SeetaFaceInfo &face = faces.data[i];
        painter.drawRect(face.pos.x, face.pos.y, face.pos.width, face.pos.height);
        
        QString displayText;
        
        /* 如果启用了人脸识别，进行识别 */
        if (faceRecognizeEnabled && faceLandmarker && faceRecognizer) {
            /* 检测人脸特征点 */
            SeetaPointF points[5];
            faceLandmarker->mark(seetaImg, face.pos, points);

            /* 提取人脸特征 */
            int featureSize = faceRecognizer->GetExtractFeatureSize();
            QVector<float> feature(featureSize);
            faceRecognizer->Extract(seetaImg, points, feature.data());

            /* 识别人脸 */
            QString name = recognizeFace(feature);
            displayText = name;
            
            /* 更新信息标签 */
            infoLabel->setText(QString("识别: %1").arg(name));
        } else {
            /* 只显示置信度 */
            displayText = QString("Score: %1").arg(face.score, 0, 'f', 2);
        }
        
        painter.setPen(QPen(Qt::yellow, 2));
        painter.drawText(face.pos.x, face.pos.y - 5, displayText);
        painter.setPen(QPen(Qt::green, 2));
    }

    qDebug() << "检测到" << faces.size << "个人脸";

    return resultImage;
#else
    /* PC平台直接返回原图 */
    return image;
#endif
}

void FaceRecognition::registerFaceButtonClicked()
{
    QString name = nameLineEdit->text().trimmed();
    
    if (name.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先输入姓名！");
        return;
    }

    if (currentFrame.isNull()) {
        QMessageBox::warning(this, "警告", "没有可用的图像！");
        return;
    }

    /* 提取人脸特征 */
    QVector<float> feature;
    if (extractFaceFeature(currentFrame, feature)) {
        /* 保存到数据库 */
        faceDatabase[name] = feature;
        saveFaceDatabase();
        
        infoLabel->setText(QString("成功录入: %1\n数据库人数: %2")
                          .arg(name).arg(faceDatabase.size()));
        QMessageBox::information(this, "成功", QString("成功录入人脸: %1").arg(name));
        nameLineEdit->clear();
        
        /* 如果正在查看数据库管理界面，刷新视图 */
        if (stackedWidget->currentIndex() == 1) {
            refreshDatabaseView();
        }
    } else {
        QMessageBox::warning(this, "失败", "未检测到人脸或特征提取失败！");
    }
}

void FaceRecognition::faceRecognizeButtonClicked()
{
    if (faceRecognizeButton->isChecked()) {
        faceRecognizeEnabled = true;
        faceRecognizeButton->setText("关闭人脸识别");
        faceDetectEnabled = true;
        faceDetectButton->setChecked(true);
        infoLabel->setText("识别模式开启");
    } else {
        faceRecognizeEnabled = false;
        faceRecognizeButton->setText("开启人脸识别");
        infoLabel->setText("待机中...");
    }
}

bool FaceRecognition::extractFaceFeature(const QImage &image, QVector<float> &feature)
{
#ifdef __arm__
    if (!faceDetector || !faceLandmarker || !faceRecognizer) {
        return false;
    }

    /* 将QImage转换为SeetaImageData */
    QImage rgbImage = image.convertToFormat(QImage::Format_RGB888);
    
    SeetaImageData seetaImg;
    seetaImg.width = rgbImage.width();
    seetaImg.height = rgbImage.height();
    seetaImg.channels = 3;
    seetaImg.data = rgbImage.bits();

    /* 检测人脸 */
    SeetaFaceInfoArray faces = faceDetector->detect(seetaImg);
    
    if (faces.size == 0) {
        qDebug() << "未检测到人脸";
        return false;
    }

    /* 使用第一个检测到的人脸 */
    SeetaFaceInfo &face = faces.data[0];

    /* 检测人脸特征点 */
    SeetaPointF points[5];
    faceLandmarker->mark(seetaImg, face.pos, points);

    /* 提取人脸特征 */
    int featureSize = faceRecognizer->GetExtractFeatureSize();
    feature.resize(featureSize);
    faceRecognizer->Extract(seetaImg, points, feature.data());

    qDebug() << "成功提取人脸特征，特征维度:" << featureSize;
    return true;
#else
    Q_UNUSED(image);
    Q_UNUSED(feature);
    return false;
#endif
}

QString FaceRecognition::recognizeFace(const QVector<float> &feature)
{
#ifdef __arm__
    if (faceDatabase.isEmpty()) {
        return "未知";
    }

    QString bestMatch = "未知";
    float bestSimilarity = 0.0f;
    float threshold = 0.6f;  // 相似度阈值

    /* 遍历数据库中的所有人脸 */
    for (auto it = faceDatabase.begin(); it != faceDatabase.end(); ++it) {
        float similarity = faceRecognizer->CalculateSimilarity(feature.data(), it.value().data());
        
        if (similarity > bestSimilarity) {
            bestSimilarity = similarity;
            bestMatch = it.key();
        }
    }

    /* 如果最佳相似度低于阈值，返回未知 */
    if (bestSimilarity < threshold) {
        return QString("未知(%1)").arg(QString::number(bestSimilarity, 'f', 2));
    }

    return QString("%1(%2)").arg(bestMatch).arg(QString::number(bestSimilarity, 'f', 2));
#else
    Q_UNUSED(feature);
    return "未知";
#endif
}

void FaceRecognition::saveFaceDatabase()
{
    QFile file("face_database.dat");
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "无法保存人脸数据库";
        return;
    }

    QDataStream out(&file);
    out << faceDatabase.size();
    
    for (auto it = faceDatabase.begin(); it != faceDatabase.end(); ++it) {
        out << it.key();
        out << it.value().size();
        for (int i = 0; i < it.value().size(); i++) {
            out << it.value()[i];
        }
    }
    
    file.close();
    qDebug() << "人脸数据库已保存，共" << faceDatabase.size() << "人";
}

void FaceRecognition::loadFaceDatabase()
{
    QFile file("face_database.dat");
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        qDebug() << "人脸数据库文件不存在或无法打开";
        return;
    }

    QDataStream in(&file);
    int count;
    in >> count;
    
    faceDatabase.clear();
    for (int i = 0; i < count; i++) {
        QString name;
        int featureSize;
        in >> name >> featureSize;
        
        QVector<float> feature(featureSize);
        for (int j = 0; j < featureSize; j++) {
            in >> feature[j];
        }
        
        faceDatabase[name] = feature;
    }
    
    file.close();
    qDebug() << "人脸数据库已加载，共" << faceDatabase.size() << "人";
    
    if (!faceDatabase.isEmpty()) {
        infoLabel->setText(QString("数据库人数: %1").arg(faceDatabase.size()));
    }
    
    /* 如果数据库管理UI已创建，刷新视图 */
    if (databaseWidget) {
        refreshDatabaseView();
    }
}

void FaceRecognition::showVirtualKeyboard()
{
    if (virtualKeyboard) {
        /* 确保输入框可编辑 */
        nameLineEdit->setReadOnly(false);
        virtualKeyboard->showKeyboard();
    }
}

void FaceRecognition::manageDatabaseButtonClicked()
{
    if (manageDatabaseButton->isChecked()) {
        /* 切换到数据库管理视图 */
        stackedWidget->setCurrentIndex(1);
        manageDatabaseButton->setText("返回摄像头");
        refreshDatabaseView();
    } else {
        /* 切换回摄像头视图 */
        stackedWidget->setCurrentIndex(0);
        manageDatabaseButton->setText("数据库管理");
    }
}

void FaceRecognition::createDatabaseManagementUI()
{
    databaseWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(databaseWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);
    
    /* 标题 */
    QLabel *titleLabel = new QLabel("人脸数据库管理", databaseWidget);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2196F3;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    /* 数据表格 */
    QLabel *tableLabel = new QLabel("已录入人员列表", databaseWidget);
    tableLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333;");
    mainLayout->addWidget(tableLabel);
    
    tableWidget = new QTableWidget(databaseWidget);
    tableWidget->setColumnCount(3);
    tableWidget->setHorizontalHeaderLabels({"姓名", "特征维度", "操作"});
    
    /* 设置表格样式 */
    tableWidget->horizontalHeader()->setStretchLastSection(false);
    tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setSelectionBehavior(QTableWidget::SelectRows);
    tableWidget->setEditTriggers(QTableWidget::NoEditTriggers);
    tableWidget->setAlternatingRowColors(true);
    tableWidget->setStyleSheet(
        "QTableWidget {"
        "   background-color: white;"
        "   gridline-color: #ddd;"
        "   border: 1px solid #ccc;"
        "   border-radius: 5px;"
        "}"
        "QTableWidget::item {"
        "   padding: 5px;"
        "}"
        "QHeaderView::section {"
        "   background-color: #2196F3;"
        "   color: white;"
        "   padding: 8px;"
        "   border: none;"
        "   font-weight: bold;"
        "}"
    );
    
    mainLayout->addWidget(tableWidget);
    
    /* 统计信息 */
    QLabel *statsLabel = new QLabel("", databaseWidget);
    statsLabel->setObjectName("statsLabel");
    statsLabel->setStyleSheet("font-size: 13px; color: #666; padding: 10px;");
    statsLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statsLabel);
}

void FaceRecognition::updateDatabaseTable()
{
    if (!tableWidget) return;
    
    tableWidget->setRowCount(0);
    
    int row = 0;
    for (auto it = faceDatabase.begin(); it != faceDatabase.end(); ++it) {
        tableWidget->insertRow(row);
        
        /* 姓名 */
        QTableWidgetItem *nameItem = new QTableWidgetItem(it.key());
        nameItem->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 0, nameItem);
        
        /* 特征维度 */
        QTableWidgetItem *dimItem = new QTableWidgetItem(QString::number(it.value().size()));
        dimItem->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(row, 1, dimItem);
        
        /* 删除按钮 */
        QPushButton *deleteBtn = new QPushButton("删除");
        deleteBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #f44336;"
            "   color: white;"
            "   border: none;"
            "   padding: 5px 15px;"
            "   border-radius: 3px;"
            "   font-size: 12px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #d32f2f;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #b71c1c;"
            "}"
        );
        
        QString name = it.key();
        connect(deleteBtn, &QPushButton::clicked, this, [this, name]() {
            deleteFaceData(name);
        });
        
        tableWidget->setCellWidget(row, 2, deleteBtn);
        row++;
    }
}

void FaceRecognition::refreshDatabaseView()
{
    updateDatabaseTable();
    
    /* 更新统计信息 */
    QLabel *statsLabel = databaseWidget->findChild<QLabel*>("statsLabel");
    if (statsLabel) {
        int totalCount = faceDatabase.size();
        int totalFeatures = 0;
        for (auto it = faceDatabase.begin(); it != faceDatabase.end(); ++it) {
            totalFeatures += it.value().size();
        }
        
        statsLabel->setText(QString("📊 总人数: %1 | 总特征数: %2")
                           .arg(totalCount)
                           .arg(totalFeatures));
    }
}

void FaceRecognition::deleteFaceData(const QString &name)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除", 
                                  QString("确定要删除 '%1' 的人脸数据吗？").arg(name),
                                  QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        faceDatabase.remove(name);
        saveFaceDatabase();
        refreshDatabaseView();
        
        /* 更新信息标签 */
        if (faceDatabase.isEmpty()) {
            infoLabel->setText("数据库已清空");
        } else {
            infoLabel->setText(QString("数据库人数: %1").arg(faceDatabase.size()));
        }
        
        QMessageBox::information(this, "成功", QString("已删除 '%1' 的人脸数据").arg(name));
    }
}
