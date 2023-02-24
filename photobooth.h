#pragma once
#ifndef PhotoBooth_H
#define PhotoBooth_H

#include <QWidget>
#include <QtCore>
#include <QPushButton>
#include <QThread>
#include <QMovie>

#include <atomic>

#include "lib/camera.h"
#include "lib/camtrigger.h"
#include "lib/photo.h"
#include "lib/relay.h"
#include "lib/printer.h"


QT_BEGIN_NAMESPACE
namespace Ui { class PhotoBooth; }
QT_END_NAMESPACE

class Camera;
class CamTrigger;

class PhotoBooth : public QWidget
{
    Q_OBJECT

public:
    PhotoBooth(QWidget *parent = nullptr);
    ~PhotoBooth();

private:
    QWidget* m_application;
    Ui::PhotoBooth* m_ui;
    Camera* m_camera;
    CamTrigger* m_camTrigger;
    Photo* m_photo;
    Printer* m_printer;
    QMovie* m_movie;

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
    bool m_fullScreen;
    bool m_secondScreen;
    uint m_fps;
    uint m_relayDevice;
    uint m_cameraDevice;
    std::atomic_bool m_isCameraLoading;
    std::atomic_bool m_isRelayLoading;
    bool m_isLoading;


    QTimer* m_countDownTimer;
    QTimer* m_sleepTimer;
    QTimer* m_remoteTimer;
    QTimer* m_cameraTimer;
    int m_count;

    QString m_font;
    double m_fontSizeRatio;
    QMap<char, uint> m_backgroundColor;
    QMap<QString, uint> m_relaysConfig;

    bool readingSettingsFile();
    void settingDisplay();
    void settingRelayDevices();

    void showCam();
    void showPhoto();
    void exit();

    void updateNbPrint(int increment);
    void btnActivate(QPushButton* button);
    void btnDisable(QPushButton* button);
    void print();
    void takePhoto();
    void treatPhoto();
    void reinitSleep();
    void startLoading();
    void stopLoading();

    Relay* m_relay;
    RelayDevice* m_pcFan;
    RelayDevice* m_printerFan;
    RelayDevice* m_light;

public:
    void setIsCameraLoading(bool value) {m_isCameraLoading = value;};
    void setIsRelayLoading(bool value) {m_isRelayLoading = value;};

private slots:
    void countDown();
    void goToSleep();
    void checkSystems();
    void CameraLoop();

signals:
    void checkLoopSignal();
    void focusSignal();
    void triggerSignal();
};

#endif // PhotoBooth_H
