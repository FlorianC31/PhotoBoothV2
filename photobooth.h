#pragma once
#ifndef PhotoBooth_H
#define PhotoBooth_H

#include <QWidget>
#include <QtCore>
#include <QPushButton>

#include "lib/camera.h"
#include "lib/videoflow.h"
#include "lib/relay.h"
#include "lib/photo.h"
#include "lib/printer.h"


QT_BEGIN_NAMESPACE
namespace Ui { class PhotoBooth; }
QT_END_NAMESPACE

class VideoFlow;

class PhotoBooth : public QWidget
{
    Q_OBJECT

public:
    PhotoBooth(QWidget *parent = nullptr);
    ~PhotoBooth();

private:
    Ui::PhotoBooth *ui;
    Camera m_camera;
    VideoFlow* m_videoFlow;
    Photo m_photo;
    Printer m_printer;

    enum State{
        SLEEPING,
        SHOWING_CAM,
        DISPLAY_PIC,
        COUNTING_DOWN
    };

    QString m_settingFile;
    uint m_printCounter;
    State m_state;
    uint m_nbPrint;
    uint m_nbPrintMax;
    uint m_isoMax;
    int m_camId;
    bool m_upsideDown;
    bool m_modeDev;
    uint m_relayDevice;

    QString m_font;
    double m_fontSizeRatio;
    QMap<char, uint> m_backgroundColor;
    QMap<QString, uint> m_relaysConfig;

    bool readingSettingsFile();
    void settingDisplay();
    void settingRelayDevices();

    void showCam();
    void showPhoto();
    void goToSleep();
    void exit();

    void updateNbPrint(int increment);
    void btnActivate(QPushButton* button);
    void btnDisable(QPushButton* button);
    void print();
    void takePhoto();

    Relay* m_relay;
    RelayDevice* m_pcFan;
    RelayDevice* m_printerFan;
    RelayDevice* m_light;

};

#endif // PhotoBooth_H
