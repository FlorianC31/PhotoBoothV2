#pragma once
#ifndef CAMTRIGGER_H
#define CAMTRIGGER_H

#include <windows.h>
#include <string>
#include <QtCore>

enum KEY{
    L,
    G,
    AND,
    CTRL,
    ENTER
};


struct Point{
    int x = 0;
    int y = 0;
};

class Window
{
public:
    Window(std::string title) : m_title(title) {};
    void click(int x, int y, bool doubleClick);
    bool isOpen();
    void printSize();
    void show() {activate();};
    void pressKey(KEY key) {keyboard(key, 0);};
    void releaseKey(KEY key) {keyboard(key, KEYEVENTF_KEYUP);};
    void move(bool back = false);

private:
    void keyboard(KEY key, DWORD action);

protected:
    std::string m_title;
    HWND m_handle;
    int m_initXPos;

    void init();
    bool isActivated() {return GetForegroundWindow() == m_handle;};
    void activate();
    Point getPos();
    bool checkSize(int w, int h);
};


class IedWindow : public Window
{
public:
    IedWindow(std::string title) : Window(title) {};
    void open();
};


class RemoteWindow : public Window
{
public:
    RemoteWindow(std::string title, IedWindow* iedWindow);
    void open();

private:
    bool isPreRemote() {return checkSize(930, 376);};
    bool isWarningMsg() {return checkSize(427, 159);};
    bool isLoading() {return checkSize(541, 245);};
    bool isFinalRemote() {return isOpen() && !isPreRemote() && !isWarningMsg() && !isLoading();};
    bool liveViewIsClose() {return checkSize(325, 0);};
    void loadCamera();
    void refresh();
    void openPreRemote();
    void raiseErrorMsg(std::string errorMsg);
    void hideLiveView();

    enum State{
        INIT,
        OPENING_REMOTE,
        REFRESHING,
        CAMERA_LOADING,
        RUNING
    } m_state;

    int m_tempo;
    IedWindow* m_iedWindow;
};



class CamTrigger : public QObject
{
    Q_OBJECT

public:
    CamTrigger();
    ~CamTrigger();

    void focus(bool preShoot = false);
    void trigger();

private:
    IedWindow* m_imagingEdgeDesktop;
    RemoteWindow* m_remote;
    QTimer* m_timer;

private slots:
    void loop();

};

#endif // CAMTRIGGER_H
