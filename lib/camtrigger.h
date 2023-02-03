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
    Window(std::string title);
    void click(int x, int y, bool doubleClick);
    bool isOpen();
    bool isPreRemote() {return checkSize(930, 376);};
    bool isWarningMsg() {return checkSize(427, 173);};
    bool isLoading() {return checkSize(427, 173);}; // TODO à modifier

private:
    std::string m_title;
    HWND m_handle;

private:
    Point getPos();
    bool checkSize(int w, int h);
};



class CamTrigger
{
public:
    CamTrigger();
    ~CamTrigger();


private:

    enum State{
        INIT,
        OPENING_IED,
        OPENING_REMOTE,
        CAMERA_LOADING,
        NORMAL_RUN,
        REFRESHING,
        FAIL
    };

    // State machine actions
    void openIed();
    void openPreRemote();
    void loadCamera();
    void refresh();

    bool isOpen(HWND window) {return window == NULL;};
    void error();
    void loop();

    Window* m_imagingEdgeDesktop;
    Window* m_remote;

    State m_state;
    int m_tempo;

};

#endif // CAMTRIGGER_H
