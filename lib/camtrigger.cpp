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

#define FOCUS_TIME 200
#define TRIGGER_TIME 800


CamTrigger::CamTrigger(PhotoBooth* photoBooth, bool secondScreen) :
    m_photoBooth(photoBooth),
    m_state(INIT),
    m_title("Remote"),
    m_initXPos(500),
    m_secondScreen(secondScreen)
{
    // Creation and initialisation of trigger thread
    m_thread = new QThread();
    m_thread->start();
    this->moveToThread(m_thread);

    connect(this, &CamTrigger::startLoading, m_photoBooth, &PhotoBooth::startLoading);
    connect(this, &CamTrigger::endOfLoading, m_photoBooth, [this]() {
        m_photoBooth->endOfModuleLoading(PhotoBooth::CAM_TRIGGER);
    });
}

CamTrigger::~CamTrigger()
{
    m_thread->quit();
    m_thread->wait();
    delete m_thread;
}

void CamTrigger::focus()
{
    RECT windowRect;
    GetWindowRect(m_handle, &windowRect);
    m_initXPos = windowRect.left;
    if (!m_secondScreen) {
        move();
    }
    pressKey(G);

    QThread::msleep(FOCUS_TIME);

    releaseKey(G);
    if (!m_secondScreen) {
        move(true);
    }
}


void CamTrigger::trigger()
{
    RECT windowRect;
    GetWindowRect(m_handle, &windowRect);
    m_initXPos = windowRect.left;
    if (!m_secondScreen) {
        move();
    }
    pressKey(AND);

    QThread::msleep(TRIGGER_TIME);

    releaseKey(AND);
    if (!m_secondScreen) {
        move(true);
    }

    if (m_state != RUNNING){
        emit endOfLoading();
        m_state = RUNNING;
    }
}

bool CamTrigger::isOpen()
{
    m_handle = FindWindowA(NULL, m_title.c_str());
    return m_handle != NULL;
}

bool CamTrigger::checkSize(int w, int h)
{
    if (!isOpen())
        return false;
    RECT windowRect;
    GetWindowRect(m_handle, &windowRect);
    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;
    return (w == 0 || abs(windowWidth - w) <= SIZE_MARGIN) && (h == 0 || abs(windowHeight - h) <= SIZE_MARGIN);
}

void CamTrigger::printSize()
{
    if (!isOpen())
        return;
    RECT windowRect;
    GetWindowRect(m_handle, &windowRect);
    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;
    qDebug() << "Size of" << m_title.c_str() << ": width =" << windowWidth << "- height =" << windowHeight;
}

void CamTrigger::click(int x, int y)
{
    if (!isOpen())
        return;

    activate();

    Point pos = getPos();

    SetCursorPos(x + pos.x, y + pos.y);

    mouse_event(MOUSEEVENTF_LEFTDOWN, x + pos.x, y + pos.y, 0, 0);
    mouse_event(MOUSEEVENTF_LEFTUP, x + pos.x, y + pos.y, 0, 0);
}

void CamTrigger::activate()
{
    if (isActivated())
        return;

    SetForegroundWindow(m_handle);
    int nbTries = 0;
    while (!isActivated() && nbTries < MAX_NB_ACTIVATE_TRIES) {
        QThread::msleep(ACTIVATE_TRIES_DURATION);
        nbTries++;
    }
    QThread::msleep(500);
}


Point CamTrigger::getPos()
{
    RECT windowRect;
    GetWindowRect(m_handle, &windowRect);
    Point pos = {windowRect.left, windowRect.top};
    return pos;
}


void CamTrigger::keyboard(KEY key, DWORD action)
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

void CamTrigger::move(bool back)
{
    RECT windowRect;
    GetWindowRect(m_handle, &windowRect);

    if (back)
    {
        qDebug() << m_title.c_str() << "moving back to x =" << m_initXPos;
        SetWindowPos(m_handle, 0, m_initXPos, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_NOZORDER | SWP_NOACTIVATE);
        qDebug() << m_title.c_str() << "moved back to x =" << m_initXPos;
    }
    else
    {
        SetWindowPos(m_handle, 0, m_initXPos + MOVE_DISTANCE, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_NOZORDER | SWP_NOACTIVATE);
        qDebug() << m_title.c_str() << "moved to x =" << m_initXPos + MOVE_DISTANCE;
    }
}


void CamTrigger::checkLoop()
{
    if (isFinalRemote() && m_state == RUNNING)
    {
        qDebug() << "LOOP - Camera Final Remote already open";
        return;
    }

    emit startLoading();

    if (!isFinalRemote()){
        m_state = INIT;
        m_tempo = 0;
    }

    while (m_state != RUNNING) {

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
                init();
            else if (isDisconnectMsg())
                okDisconnect();
            else if (isLiveView())
                closeLiveView();
            else
                raiseErrorMsg("initializing Remote window");
            break;

        case OPENING_REMOTE:
            qDebug() << "State: OPENING_REMOTE -" << m_tempo << "/" << MAX_OPEN_TEMPO;
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
            qDebug() << "State: REFRESHING -" << m_tempo << "/" << REFRESH_TEMPO;
            if (isPreRemote() && m_tempo == REFRESH_TEMPO)
                loadCamera();
            else if (isWarningMsg())
                refresh();
            else if (isFinalRemote())
                init();
            else if (isLiveView())
                closeLiveView();
            else if (m_tempo == REFRESH_TEMPO)
                raiseErrorMsg("refreshing");
            else
                m_tempo++;
            break;

        case CAMERA_LOADING:
            qDebug() << "State: CAMERA_LOADING -" << m_tempo << "/" << MAX_LOAD_TEMPO;
            if (isFinalRemote())
                init();
            else if (isLiveView())
                closeLiveView();
            else if (isWarningMsg() && m_tempo == MAX_LOAD_TEMPO)
                refresh();
            else if (m_tempo == MAX_LOAD_TEMPO)
                refresh();
            else
                m_tempo++;
            break;

        case RUNNING:
            qDebug() << "State: RUNNING";
            break;
        default:
            raiseErrorMsg("initalizing state");
            break;
        }

        QThread::msleep(1000);
    }

}

void CamTrigger::init()
{
    m_handle = FindWindowA(NULL, m_title.c_str());

    // launch a useless trigger to disable hasardous previous focus locked
    trigger();
}

void CamTrigger::raiseErrorMsg(std::string errorMsg) {
    qDebug() << "ERROR: Error while" << errorMsg.c_str();
    m_state = INIT;
}

void CamTrigger::okDisconnect()
{
    // Click OK on warning msgBox
    pressKey(ENTER);
    releaseKey(ENTER);

    openPreRemote();
}

void CamTrigger::refresh()
{
    qDebug() << "Refresh";

    // Click OK on warning msgBox
    pressKey(ENTER);
    releaseKey(ENTER);

    int nbTries = 0;
    while (!isPreRemote() && nbTries < MAX_NB_ACTIVATE_TRIES) {
        QThread::msleep(ACTIVATE_TRIES_DURATION);
        nbTries++;
    }

    click(714, 341);

    m_state = REFRESHING;
    m_tempo = 0;
}

void CamTrigger::loadCamera()
{
    qDebug() << "Load camera";
    QThread::msleep(2000); // Wait 2000ms to let time to PreRemote to show
    m_tempo = 0;
    click(120, 75);
    pressKey(ENTER);
    m_state = CAMERA_LOADING;
}


void CamTrigger::openPreRemote()
{
    m_tempo = 0;
    qDebug() << "Open PreRemote";

    if (isPreRemote()) {
        qDebug() << "Remote is already open";
        return;
    }

    std::string remotePath = "\"C:\\Program Files\\Sony\\Imaging Edge\\Remote.exe\"";
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wfilePath = converter.from_bytes(remotePath);
    ShellExecute(NULL, L"open", wfilePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    system(remotePath.c_str());

    m_state = OPENING_REMOTE;
}


void CamTrigger::closeLiveView()
{
    if (!isFinalRemote()) {
        pressKey(CTRL);
        pressKey(L);
        releaseKey(L);
        releaseKey(CTRL);
    }
}
