#-------------------------------------------------
#
# Project created by QtCreator 2013-11-16T08:24:34
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VideoLab
TEMPLATE = app

OpenCV_DIR = C:/Programs/OpenCV/opencv-246-src/build/install

INCLUDEPATH +=  $${OpenCV_DIR}/include

LIBS    +=  $${OpenCV_DIR}/bin/libopencv_core246.dll \
            $${OpenCV_DIR}/bin/libopencv_highgui246.dll \
            $${OpenCV_DIR}/bin/libopencv_imgproc246.dll \
            $${OpenCV_DIR}/bin/libopencv_video246.dll \
            $${OpenCV_DIR}/bin/opencv_ffmpeg246.dll

SOURCES += main.cpp\
        mainwindow.cpp \
    mainwork/mainwork.cpp \
    mainwork/modules/gim.cpp \
    #mainwork/myfunctions/myvisionfunctions.cpp \
    mainwork/wmodules/wtracker.cpp

HEADERS  += mainwindow.h \
    mainwork/mainwork.h \
    mainwork/modules/gim.h \
    mainwork/wmodules/wtracker.h

FORMS    += mainwindow.ui
