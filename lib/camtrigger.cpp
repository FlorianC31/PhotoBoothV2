#include "camtrigger.h"
#include <cstdlib>
#include <qDebug>
#include <codecvt>
#include <locale>

#define MAX_OPEN_TEMPO 10
#define MAX_LOAD_TEMPO 10
#define REFRESH_TEMPO 10

#define SIZE_MARGIN 10
#define MAX_NB_ACTIVATE_TRIES 20
#define ACTIVATE_TRIES_DURATION 100 // ms

#define KEY_L 0x31
#define KEY_G 0x47
#define KEY_1 0x26
#define KEY_CTRL 0x11
#define KEY_ENTER 0x0D

CamTrigger::CamTrigger() :
    m_wait(false)
{

    m_imagingEdgeDesktop = new IedWindow("Imaging Edge Desktop");
    m_remote = new RemoteWindow("Remote", m_imagingEdgeDesktop);

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
    if (m_wait)
        return;

    if (!m_remote->isOpen()) {
        if (!m_imagingEdgeDesktop->isOpen())
            m_imagingEdgeDesktop->open();
        m_remote->open();
    }
}


// ###################### Window Class ################################

bool Window::isOpen()
{
    m_handle = FindWindowA(NULL, m_title.c_str());
    return m_handle != NULL;
}

bool Window::checkSize(int w, int h)
{
    if (!isOpen())
        return false;
    RECT windowRect;
    GetWindowRect(m_handle, &windowRect);
    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;
    return abs(windowWidth - w) <= SIZE_MARGIN && abs(windowHeight - h) <= SIZE_MARGIN;
}

void Window::printSize()
{
    if (!isOpen())
        return;
    RECT windowRect;
    GetWindowRect(m_handle, &windowRect);
    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;
    qDebug() << "Size of" << m_title.c_str() << ": width =" << windowWidth << "- height =" << windowHeight;
}

void Window::click(int x, int y, bool doubleClick)
{
    if (!isOpen())
        return;

    activate();

    Point pos = getPos();

    SetCursorPos(x + pos.x, y + pos.y);

    mouse_event(MOUSEEVENTF_LEFTDOWN, x + pos.x, y + pos.y, 0, 0);
    mouse_event(MOUSEEVENTF_LEFTUP, x + pos.x, y + pos.y, 0, 0);

    if (doubleClick)
    {
        Sleep(100);
        mouse_event(MOUSEEVENTF_LEFTDOWN, x + pos.x, y + pos.y, 0, 0);
        mouse_event(MOUSEEVENTF_LEFTUP, x + pos.x, y + pos.y, 0, 0);
    }
}

void Window::activate()
{
    SetForegroundWindow(m_handle);
    int nbTries = 0;
    while (!isActivated() && nbTries < MAX_NB_ACTIVATE_TRIES) {
        Sleep(ACTIVATE_TRIES_DURATION);
        nbTries++;
    }
    Sleep(500);
}


Point Window::getPos()
{
    RECT windowRect;
    GetWindowRect(m_handle, &windowRect);
    Point pos = {windowRect.left, windowRect.top};
    return pos;
}

void Window::pressKey(BYTE key, BYTE key2) {

    activate();

    keybd_event(key, 0, 0, 0);
    if (key2 != 0) {
        Sleep(5);
        keybd_event(key2, 0, 0, 0);
    }

    Sleep(200);

    keybd_event(key, 0, KEYEVENTF_KEYUP, 0);
    if (key2 != 0) {
        Sleep(5);
        keybd_event(key2, 0, KEYEVENTF_KEYUP, 0);
    }
}


// ###################### IedWindow Class ################################

void IedWindow::open()
{
    qDebug() << "Open Image Edge Desktop";

    if (isOpen()) {
        qDebug() << "Image Edge Desktop is already open";
        return;
    }

    std::string imagingPath = "\"C:\\Program Files\\Sony\\Imaging Edge Desktop\\ied.exe\"";
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wfilePath = converter.from_bytes(imagingPath);
    ShellExecute(NULL, L"open", wfilePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    system(imagingPath.c_str());
}



// ###################### RemoteWindow Class ################################

RemoteWindow::RemoteWindow(std::string title, IedWindow* iedWindow) :
    Window(title),
    m_iedWindow(iedWindow)
{};



void RemoteWindow::open()
{
    m_state = INIT;
    m_tempo = 0;

    while (m_state != RUNING) {

        switch (m_state){
        case INIT:
            qDebug() << "State: INIT";
            if (!isOpen())
                openPreRemote();
            else if (isPreRemote())
                loadCamera();
            else if (isWarningMsg())
                refresh();
            else if (isLoading())
                m_state = CAMERA_LOADING;
            else if (isFinalRemote())
                m_state = RUNING;
            else
                raiseErrorMsg("initializing Remote window");
            break;

        case OPENING_REMOTE:
            qDebug() << "State: OPENING_REMOTE";
            if (isPreRemote())
                loadCamera();
            else if (isWarningMsg())
                refresh();
            else if (m_tempo == MAX_OPEN_TEMPO)
                raiseErrorMsg("opening Remote window");
            else
                m_tempo++;
            break;

        case REFRESHING:
            qDebug() << "State: REFRESHING";
            if (isPreRemote() && m_tempo == REFRESH_TEMPO)
                loadCamera();
            else if (isWarningMsg())
                refresh();
            else
                m_tempo++;
            break;

        case CAMERA_LOADING:
            qDebug() << "State: CAMERA_LOADING";
            if (isFinalRemote())
                m_state = RUNING;
            else if (m_tempo == MAX_LOAD_TEMPO)
                raiseErrorMsg("loading camera");
            else
                m_tempo++;
            break;

        case RUNING:
            qDebug() << "State: RUNING";
            break;
        }

        Sleep(1000);
    }
    qDebug() << "State: RUNING";
    pressKey(KEY_CTRL, KEY_L);
}

void RemoteWindow::raiseErrorMsg(std::string errorMsg) {
    qDebug() << "ERROR: Error while" << errorMsg.c_str();
    m_state = INIT;
}


void RemoteWindow::refresh()
{
    qDebug() << "Refresh";

    // Click OK on warning msgBox
    pressKey(KEY_ENTER);

    int nbTries = 0;
    while (!isPreRemote() && nbTries < MAX_NB_ACTIVATE_TRIES) {
        Sleep(ACTIVATE_TRIES_DURATION);
        nbTries++;
    }

    click(714, 341, false);

    m_state = REFRESHING;
    m_tempo = 0;
}

void RemoteWindow::loadCamera()
{
    qDebug() << "Load camera";
    m_tempo = 0;
    click(120, 75, true);
    m_state = CAMERA_LOADING;
}


void RemoteWindow::openPreRemote()
{
    m_tempo = 0;
    qDebug() << "Open PreRemote";

    //m_wait = true;
    // Click on "Accueil" button
    m_iedWindow->click(55, 90, false);
    // Click on "Démarrer" button on remote section
    m_iedWindow->click(670, 160, false);
    //m_wait = false;

    m_state = OPENING_REMOTE;
}

