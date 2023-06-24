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


QT_BEGIN_NAMESPACE
namespace Ui { class PhotoBooth; }
QT_END_NAMESPACE

class Camera;
class CamTrigger;
class Relay;
class RelayDevice;
class Photo;


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
        RELAY,
        PHOTO
    };

private:
    Ui::PhotoBooth* m_ui;
    Camera* m_camera;
    CamTrigger* m_camTrigger;
    Photo* m_photo;
    Printer* m_printer;
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
    uint m_isoMax;
    bool m_upsideDown;
    bool m_mirror;
    bool m_addWatermark;
    bool m_modeDev;
    bool m_fullScreen;
    bool m_secondScreen;
    QString m_photoFolder;
    uint m_fps;
    uint m_relayDevice;
    uint m_cameraDevice;
    uint m_resolutionMode;
    QPixmap* m_lastPhoto;
    QPixmap* m_lastPhoto2Print;

    QTimer* m_countDownTimer;
    QTimer* m_sleepTimer;
    QTimer* m_remoteTimer;
    QTimer* m_cameraTimer;
    QTimer* m_loadingTimer;
    int m_count;

    QString m_font;
    double m_fontSizeRatio;
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
    void updateFont(QLabel* label);
    void updateFont(QPushButton* button);
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

public slots:
    void startLoading();
    void stopLoading();
    void endOfModuleLoading(PhotoBooth::Module module);
    void loadNewPhoto(QPixmap* lastPhoto, QPixmap* lastPhoto2Print);

signals:
    void initSignal(bool secondScreen);
    void focusSignal();
    void triggerSignal();
    void flashSignal();
    void showLookUpSignal();
    void hideLookUpSignal();
    void loadLastPhoto();
};

#endif // PhotoBooth_H
