#include "camtrigger.h"
#include <cstdlib>
#include <qDebug>
#include <codecvt>
#include <locale>

#define MAX_TEMPO 15
#define REFRESH_TEMPO 20

CamTrigger::CamTrigger() :
    m_state(INIT)
{

    m_imagingEdgeDesktop = new Window("Imaging Edge Desktop");
    m_remote = new Window("Remote");

    while (true){
        loop();
        Sleep(1000);
    }
}

CamTrigger::~CamTrigger()
{
    delete m_imagingEdgeDesktop;
    delete m_remote;
}

void CamTrigger::loop()
{
    switch (m_state) {

    case INIT:
        qDebug() << "State: INIT";
        if (m_imagingEdgeDesktop->isOpen())
            openPreRemote();
        else
            openIed();
        break;

    case OPENING_IED:
        qDebug() << "State: OPENING_IED";
        if (m_tempo == MAX_TEMPO)
            error();
        else if (m_imagingEdgeDesktop->isOpen())
            openPreRemote();
        else
            m_tempo++;
        break;

    case OPENING_REMOTE:
        qDebug() << "State: OPENING_REMOTE";
        if (m_tempo == MAX_TEMPO)
            error();
        else if(!m_remote->isOpen())
            m_tempo++;
        else if (m_remote->isPreRemote())
            loadCamera();
        else if (m_remote->isWarningMsg())
            refresh();
        else if (m_remote->isLoading())
            m_state = CAMERA_LOADING;
        else
            m_state = NORMAL_RUN;
        break;

    case CAMERA_LOADING:
        qDebug() << "State: CAMERA_LOADING";
        if (m_tempo == MAX_TEMPO)
            error();
        else
            m_tempo++;
        break;

    case NORMAL_RUN:
        qDebug() << "State: NORMAL_RUN";
        if (m_remote->isWarningMsg())
            m_state = OPENING_REMOTE;
        break;

    case REFRESHING:
        qDebug() << "State: REFRESHING";
        if (m_tempo == REFRESH_TEMPO)
            m_state = OPENING_REMOTE;
        else
            m_tempo++;
        break;

    case FAIL:
        qDebug() << "State: FAIL";
        break;
    }

}


void CamTrigger::error()
{
    qDebug() << "Error";
    m_state = FAIL;
}

void CamTrigger::refresh()
{
    m_state = REFRESHING;
    m_tempo = 0;
}

void CamTrigger::loadCamera()
{
    m_state = CAMERA_LOADING;
    m_tempo = 0;
    m_remote->click(120, 75, true);
}


void CamTrigger::openIed()
{
    m_state = OPENING_IED;
    m_tempo = 0;
    qDebug() << "Opening Imaging Edge Desktop";
    std::string imagingPath = "\"C:\\Program Files\\Sony\\Imaging Edge Desktop\\ied.exe\"";
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wfilePath = converter.from_bytes(imagingPath);
    ShellExecute(NULL, L"open", wfilePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    system(imagingPath.c_str());
}


void CamTrigger::openPreRemote()
{
    m_state = OPENING_REMOTE;
    m_tempo = 0;
    qDebug() << "Opening PreRemote";
    // Click on "Démarrer" button on remote section
    m_imagingEdgeDesktop->click(670, 160, false);
}


Window::Window(std::string title) :
    m_title(title)
{}


bool Window::isOpen()
{
    m_handle = FindWindowA(NULL, m_title.c_str());
    if (m_handle == NULL) {
        qDebug() << "The window" << m_title.c_str() << "is not open";
        return false;
    }
    else {
        qDebug() << "The window" << m_title.c_str() << "is open and it's not a Remote";
        return true;
    }
}

bool Window::checkSize(int w, int h)
{
    RECT windowRect;
    GetWindowRect(m_handle, &windowRect);
    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;
    return windowWidth == w && windowHeight == h;
}


void Window::click(int x, int y, bool doubleClick)
{
    if (!isOpen())
        return;

    SetForegroundWindow(m_handle);
    Sleep(10);
    qDebug() << "Placing focus on" << m_title.c_str();

    Point pos = getPos();

    SetCursorPos(x + pos.x, y + pos.y);
    qDebug() << "Clicking in (" << x << "+" << pos.x << "," << y << "+" << pos.y;

    mouse_event(MOUSEEVENTF_LEFTDOWN, x + pos.x, y + pos.y, 0, 0);
    mouse_event(MOUSEEVENTF_LEFTUP, x + pos.x, y + pos.y, 0, 0);

    if (doubleClick)
    {
        Sleep(100);
        mouse_event(MOUSEEVENTF_LEFTDOWN, x + pos.x, y + pos.y, 0, 0);
        mouse_event(MOUSEEVENTF_LEFTUP, x + pos.x, y + pos.y, 0, 0);
    }
}


Point Window::getPos()
{
    RECT windowRect;
    GetWindowRect(m_handle, &windowRect);
    Point pos = {windowRect.left, windowRect.top};
    return pos;
}

