#pragma once
#ifndef CAMTRIGGER_H
#define CAMTRIGGER_H

#include <windows.h>
#include <string>


struct Point{
    int x = 0;
    int y = 0;
};

class Window
{
private:

public:
    Window(std::string title) : m_title(title) {};
    void click(int x, int y, bool doubleClick);
    bool isOpen();
    void printSize();
    void show() {activate();};
    void pressKey(BYTE key, BYTE key2 = 0);

protected:
    std::string m_title;
    HWND m_handle;

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
    void loadCamera();
    void refresh();
    void openPreRemote();
    void raiseErrorMsg(std::string errorMsg);

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



class CamTrigger
{
public:
    CamTrigger();
    ~CamTrigger();


private:
    void loop();

    IedWindow* m_imagingEdgeDesktop;
    RemoteWindow* m_remote;
    int m_tempo;
    bool m_wait;

};

#endif // CAMTRIGGER_H
