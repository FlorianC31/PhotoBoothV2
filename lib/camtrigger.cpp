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
#define LOOP_PERIOD 1000 //ms

#define MOVE_DISTANCE 2000 // px

CamTrigger::CamTrigger() :
    m_triggerThread(nullptr)
{
    // Creation and initialisation of trigger thread
    m_triggerThread = new QThread();
    m_triggerThread->start();
    this->moveToThread(m_triggerThread);

    // Creation of both window objects
    m_imagingEdgeDesktop = new IedWindow("Imaging Edge Desktop");
    m_remote = new RemoteWindow("Remote", m_imagingEdgeDesktop);

    // Open remote windows
    m_imagingEdgeDesktop->open();
    m_remote->open();

    // launch a useless trigger to disable hasardous previous focus locked
    trigger();

}


CamTrigger::~CamTrigger()
{
    delete m_imagingEdgeDesktop;
    delete m_remote;
}

void CamTrigger::check()
{
    m_remote->open();
}


void CamTrigger::focus()
{
    if (!m_secondScreen)
        m_remote->move();

    m_remote->pressKey(G);
    Sleep(200);
    m_remote->releaseKey(G);

    if (!m_secondScreen)
        m_remote->move(true);
}

void CamTrigger::trigger()
{
    if (!m_secondScreen)
        m_remote->move();
    m_remote->pressKey(AND);
    Sleep(800);
    m_remote->releaseKey(AND);
    if (!m_secondScreen)
        m_remote->move(true);
}


// ###################### Window Class ################################

void Window::init()
{
    m_handle = FindWindowA(NULL, m_title.c_str());
    RECT windowRect;
    GetWindowRect(m_handle, &windowRect);
    m_initXPos = windowRect.left;
}

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
    return (w == 0 || abs(windowWidth - w) <= SIZE_MARGIN) && (h == 0 || abs(windowHeight - h) <= SIZE_MARGIN);
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
    if (isActivated())
        return;

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


void Window::keyboard(KEY key, DWORD action)
{
    if (!isOpen())
        return;

    activate();

    switch (key) {
    case L:
        keybd_event(VkKeyScan('l'), 0x31, action ,0);
        break;
    case G:
        keybd_event(VkKeyScan('G'), 0x47, action ,0);
        break;
    case AND:
        keybd_event(VkKeyScan('1'), 0x26, action ,0);
        break;
    case CTRL:
        keybd_event(VK_CONTROL, 0x9d, action ,0);
        break;
    case ENTER:
        keybd_event(VK_RETURN, 0x9c, action ,0);
        break;
    }
}

void Window::move(bool back)
{
    RECT windowRect;
    GetWindowRect(m_handle, &windowRect);

    if (back)
    {
        SetWindowPos(m_handle, 0, m_initXPos, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_NOZORDER | SWP_NOACTIVATE);
        qDebug() << m_title.c_str() << "moved back to x =" << m_initXPos;
    }
    else
    {
        SetWindowPos(m_handle, 0, m_initXPos + MOVE_DISTANCE, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_NOZORDER | SWP_NOACTIVATE);
        qDebug() << m_title.c_str() << "moved to x =" << m_initXPos + MOVE_DISTANCE;
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
    if (isFinalRemote())
    {
        qDebug() << "LOOP - Camera Final Remote already open";
        return;
    }

    if (!isFinalRemote()){
        m_state = INIT;
        m_tempo = 0;
    }

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
            else if (isDisconnectMsg())
                okDisconnect();
            else if (isLiveView())
                closeLiveView();
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
            else if (isFinalRemote())
                m_state = RUNING;
            else if (isLiveView())
                closeLiveView();
            else
                m_tempo++;
            break;

        case CAMERA_LOADING:
            qDebug() << "State: CAMERA_LOADING";
            if (isFinalRemote())
                m_state = RUNING;
            else if (isLiveView())
                closeLiveView();
            else if (m_tempo == MAX_LOAD_TEMPO)
                refresh();
            else
                m_tempo++;
            break;

        case RUNING:
            qDebug() << "State: RUNING";
            break;
        }

        Sleep(1000);
    }

}

void RemoteWindow::raiseErrorMsg(std::string errorMsg) {
    qDebug() << "ERROR: Error while" << errorMsg.c_str();
    m_state = INIT;
}

void RemoteWindow::okDisconnect()
{
    // Click OK on warning msgBox
    pressKey(ENTER);
    Sleep(10);
    releaseKey(ENTER);

    openPreRemote();
}

void RemoteWindow::refresh()
{
    qDebug() << "Refresh";

    // Click OK on warning msgBox
    pressKey(ENTER);
    Sleep(10);
    releaseKey(ENTER);

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
    Sleep(2000); // Wait 2000ms to let time to PreRemote to show
    m_tempo = 0;
    click(120, 75, true);
    m_state = CAMERA_LOADING;
}


void RemoteWindow::openPreRemote()
{
    m_tempo = 0;
    qDebug() << "Open PreRemote";

    // Click on "Accueil" button
    m_iedWindow->click(55, 90, false);
    // Click on "Démarrer" button on remote section
    m_iedWindow->click(670, 160, false);

    m_state = OPENING_REMOTE;
}


void RemoteWindow::closeLiveView() {
    if (!isFinalRemote()) {
        pressKey(CTRL);
        Sleep(5);
        pressKey(L);
        Sleep(200);
        releaseKey(L);
        Sleep(5);
        releaseKey(CTRL);
    }
    m_state = RUNING;
}
