#pragma once
#ifndef PhotoBooth_H
#define PhotoBooth_H

#include <QWidget>
#include <QtCore>
#include <QPushButton>

#include "lib/camera.h"
#include "lib/camtrigger.h"
#include "lib/photo.h"
#include "lib/relay.h"
#include "lib/printer.h"


QT_BEGIN_NAMESPACE
namespace Ui { class PhotoBooth; }
QT_END_NAMESPACE

class Camera;

class PhotoBooth : public QWidget
{
    Q_OBJECT

public:
    PhotoBooth(QWidget *parent = nullptr);
    ~PhotoBooth();

private:
    Ui::PhotoBooth* m_ui;
    Camera* m_camera;
    CamTrigger* m_camTrigger;
    Photo* m_photo;
    Printer* m_printer;

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
    bool m_upsideDown;
    bool m_mirror;
    bool m_addWatermark;
    bool m_modeDev;
    uint m_fps;
    uint m_relayDevice;
    uint m_cameraDevice;

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
