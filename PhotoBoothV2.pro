QT       += core gui
QT += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    lib/cputemp.cpp \
    lib/exif.cpp \
    lib/camera.cpp \
    lib/camtrigger.cpp \
    lib/photo.cpp \
    lib/printer.cpp \
    lib/relay.cpp \
    lib/utils.cpp \
    main.cpp \
    photobooth.cpp

HEADERS += \
    lib/cputemp.h \
    lib/exif.h \
    lib/camera.h \
    lib/camtrigger.h \
    lib/photo.h \
    lib/printer.h \
    lib/relay.h \
    lib/utils.h \
    photobooth.h

FORMS += \
    photobooth.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# ftd2xx lib
win32: LIBS += -L$$PWD/lib/ftd2xx/ -lftd2xx

INCLUDEPATH += $$PWD/lib/ftd2xx
DEPENDPATH += $$PWD/lib/ftd2xx

DISTFILES += \
    lib/getCpuTemp.py \
    readme.MD \
    settingsDev.ini \
    settingsProd.ini


## Opencv lib

INCLUDEPATH += $$PWD/lib/opencv/include
DEPENDPATH += $$PWD/lib/opencv/include



win32-g++ {
    LIBS += -L$$PWD/lib/opencv/x64/mingw/lib/ -llibopencv_core470.dll
    LIBS += -L$$PWD/lib/opencv/x64/mingw/lib/ -llibopencv_imgcodecs470.dll
    LIBS += -L$$PWD/lib/opencv/x64/mingw/lib/ -llibopencv_imgproc470.dll
    LIBS += -L$$PWD/lib/opencv/x64/mingw/lib/ -llibopencv_videoio470.dll
}

win32-msvc* {
    OPENCV_PATH = $$PWD/lib/opencv/x64/msvc2019
    LIBS += -L$$OPENCV_PATH/lib/

    # Désactiver les warnings MSVC
    QMAKE_CXXFLAGS += /wd4828

    # Configuration selon le mode de compilation
    CONFIG(debug, debug|release) {
        # Versions Debug avec suffixe 'd'
        LIBS += -lopencv_core470d \
                -lopencv_imgcodecs470d \
                -lopencv_imgproc470d \
                -lopencv_videoio470d
    } else {
        # Versions Release
        LIBS += -lopencv_core470 \
                -lopencv_imgcodecs470 \
                -lopencv_imgproc470 \
                -lopencv_videoio470
    }
}

LIBS += -luser32 -lkernel32
