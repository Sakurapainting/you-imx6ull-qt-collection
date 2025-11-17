QT       += core gui charts network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

TARGET_ARCH = $${QT_ARCH}
contains(TARGET_ARCH, arm){
    # OpenCV 库配置（ARM平台）
    INCLUDEPATH += /home/you/opencv-3.4.1/install/include
    LIBS += -L/home/you/opencv-3.4.1/install/lib \
            -lopencv_core \
            -lopencv_highgui \
            -lopencv_imgproc \
            -lopencv_videoio \
            -lopencv_imgcodecs \
            -lopencv_objdetect

    # SeetaFace 库配置
    INCLUDEPATH += /home/you/SeetaFace2/build-arm/install/usr/local/include
    LIBS += -L/home/you/SeetaFace2/build-arm/install/usr/local/lib \
            -lSeetaFaceDetector \
            -lSeetaFaceLandmarker \
            -lSeetaFaceRecognizer \
            -lSeetaNet
    
    # 设置运行时库搜索路径（开发板上的库路径）
    QMAKE_LFLAGS += -Wl,-rpath=/usr/local/lib
} else {
    # OpenCV 库配置（PC平台 - 使用系统安装的OpenCV）
    # 如果使用pkg-config
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv4
    
    # 或者手动指定路径（根据你的PC上OpenCV安装位置调整）
    # INCLUDEPATH += /usr/include/opencv4
    # LIBS += -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_videoio -lopencv_imgcodecs
}

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    iconwidget.cpp \
    sliderwidget.cpp \
    appdialog.cpp \
    musicplayer.cpp \
    cdwidget.cpp \
    camera.cpp \
    virtualkeyboard.cpp \
    facerecognition.cpp

HEADERS += \
    mainwindow.h \
    iconwidget.h \
    sliderwidget.h \
    appdialog.h \
    musicplayer.h \
    cdwidget.h \
    camera.h \
    virtualkeyboard.h \
    facerecognition.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
