QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32: LIBS += -L$$PWD/../../../Softwares/vcpkg/packages/opencv4_x64-windows/lib/ -lopencv_core4 -lopencv_imgproc4 -lopencv_highgui4 -lopencv_imgcodecs4 -lopencv_video4 -lopencv_videoio4

INCLUDEPATH += $$PWD/../../../Softwares/vcpkg/packages/opencv4_x64-windows/include
DEPENDPATH += $$PWD/../../../Softwares/vcpkg/packages/opencv4_x64-windows/include
