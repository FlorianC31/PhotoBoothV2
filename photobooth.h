#pragma once
#ifndef PhotoBooth_H
#define PhotoBooth_H

#include <QWidget>
#include <QtCore>
#include <QPushButton>
#include <QMovie>

#include "lib/camera.h"
#include "lib/camtrigger.h"
#include "lib/photo.h"
#include "lib/relay.h"
#include "lib/printer.h"
#include "lib/cputemp.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PhotoBooth; }
QT_END_NAMESPACE

class Camera;
class CamTrigger;
class Relay;
class RelayDevice;
class Photo;
class CpuTemp;
class Printer;

class PhotoBooth : public QWidget
{
    Q_OBJECT

public:
    PhotoBooth(QWidget *parent = nullptr);
    ~PhotoBooth();
    void showFlash(bool show);

    enum Module{
        CAM_TRIGGER,
        CAMERA,
        PHOTO
    };

private:
    Ui::PhotoBooth* m_ui;
    Camera* m_camera;
    CamTrigger* m_camTrigger;
    Photo* m_photo;
    Printer* m_printer;
    CpuTemp* m_cpuTemp;
    QMovie* m_movie;

    enum State{
        INIT,
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
    uint m_fanTime;
    uint m_isoMax;
    bool m_upsideDown;
    bool m_mirror;
    bool m_addWatermark;
    bool m_modeDev;
    bool m_fullScreen;
    bool m_secondScreen;
    int m_maxTemp;
    int m_minTemp;
    double m_cpuTempPeriod;
    double m_startTime;
    double m_preFocusTime;
    double m_loopUpTime;
    QString m_photoFolder;
    uint m_fps;
    uint m_relayDevice;
    uint m_cameraDevice;
    uint m_resolutionMode;
    double m_triggerPeriod;
    QPixmap* m_lastPhoto;
    QString m_lastPhoto2Print;

    QTimer* m_countDownTimer;
    QTimer* m_sleepTimer;
    QTimer* m_remoteTimer;
    QTimer* m_cameraTimer;
    QTimer* m_cpuTimer;
    QTimer* m_printTimer;
    int m_count;

    QString m_font1;
    QString m_font2;
    double m_font1SizeRatio;
    double m_font1SizeRatio2;
    double m_font2SizeRatio;
    QString m_styleSheet;
    QMap<QString, uint> m_relaysConfig;

    bool m_lightOn;

    bool readingSettingsFile();
    void settingDisplay();
    void settingRelayDevices();

    void showCam();
    void showPhoto();
    void exit();

    bool generateStyleSheet(QString backGroundColor1, QString backGroundColor2, QString fontColor);
    void updateFont(QLabel* label, QString fontLabel, double fontSizeRatio);
    void updateFont(QPushButton* button, QString fontLabel, double fontSizeRatio);
    void updateNbPrint(int increment);
    void btnActivate(QPushButton* button);
    void btnDisable(QPushButton* button);
    void print();
    void takePhoto();
    void checkIso();
    void reinitSleep();

    bool allModulesLoaded();

    Relay* m_relay;
    RelayDevice* m_pcFan;
    RelayDevice* m_printerFan;
    RelayDevice* m_light;

private slots:
    void countDown();
    void goToSleep();
    void endOfPrintFan();

public slots:
    void startLoading();
    void stopLoading();
    void endOfModuleLoading(PhotoBooth::Module module);
    void loadNewPhoto(QPixmap* lastPhoto, QString lastPhoto2Print);

signals:
    void initSignal(bool secondScreen);
    void focusSignal();
    void triggerSignal();
    void flashSignal();
    void showLookUpSignal();
    void hideLookUpSignal();
    void loadLastPhoto();
    void send2printer();
};

#endif // PhotoBooth_H
