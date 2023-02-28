QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    lib/camera.cpp \
    lib/camtrigger.cpp \
    lib/photo.cpp \
    lib/printer.cpp \
    lib/relay.cpp \
    lib/utils.cpp \
    main.cpp \
    photobooth.cpp

HEADERS += \
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
    readme.MD \
    settings.ini

# Opencv lib
LIBS += -L$$PWD/lib/opencv/x64/mingw/lib/ -llibopencv_core470.dll
LIBS += -L$$PWD/lib/opencv/x64/mingw/lib/ -llibopencv_imgcodecs470.dll
LIBS += -L$$PWD/lib/opencv/x64/mingw/lib/ -llibopencv_imgproc470.dll
LIBS += -L$$PWD/lib/opencv/x64/mingw/lib/ -llibopencv_videoio470.dll

INCLUDEPATH += $$PWD/lib/opencv/include
DEPENDPATH += $$PWD/lib/opencv/include
