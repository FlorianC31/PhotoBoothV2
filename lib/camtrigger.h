#pragma once
#ifndef CAMTRIGGER_H
#define CAMTRIGGER_H

#include <windows.h>
#include <string>
#include <QtCore>

#include <photobooth.h>

class PhotoBooth;

struct Point{
    int x = 0;
    int y = 0;
};

class CamTrigger : public QObject
{
    Q_OBJECT

public:
    CamTrigger(PhotoBooth* photoBooth, bool secondScreen, int focusTime, int triggerTime);
    ~CamTrigger();
    inline bool isLoaded() {return m_state == State::RUNNING;};

private:
    PhotoBooth* m_photoBooth;
    QThread* m_thread;

    enum KEY{
        L,
        G,
        AND,
        CTRL,
        ENTER,
        DOWN
    };

    enum State{
        INIT,
        OPENING_REMOTE,
        REFRESHING,
        CAMERA_LOADING,
        RUNNING
    } m_state;

    std::string m_title;
    HWND m_handle;
    int m_initXPos;
    bool m_secondScreen;
    int m_focusTime;
    int m_triggerTime;
    int m_tempo;

    bool isPreRemote() {return checkSize(930, 376);};
    bool isWarningMsg() {return checkSize(427, 159);};
    bool isDisconnectMsg() {return checkSize(369, 159);};
    bool isLoading() {return checkSize(541, 245);};
    bool isFinalRemote() {return checkSize(325, 0);};
    bool isLiveView() {return isOpen() && !isPreRemote() && !isWarningMsg() && !isLoading() && !isDisconnectMsg() && !isFinalRemote();};
    bool isOpen();

    void loadCamera();
    void refresh();
    void openPreRemote();
    void raiseErrorMsg(std::string errorMsg);
    void hideLiveView();
    void okDisconnect();
    void closeLiveView();
    void click(int x, int y);
    void show() {activate();};
    void pressKey(KEY key) {keyboard(key, 0);};
    void releaseKey(KEY key) {keyboard(key, KEYEVENTF_KEYUP);};
    void move(bool back = false);
    void init();
    void keyboard(KEY key, DWORD action);


    bool isActivated() {return GetForegroundWindow() == m_handle;};
    void activate();
    Point getPos();
    bool checkSize(int w, int h);

public slots:
    void focus();
    void trigger();
    void checkLoop();

signals:
    void startLoading();
    void endOfLoading();
};

#endif // CAMTRIGGER_H
