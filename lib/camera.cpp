#include "Camera.h"
#include <QDebug>
#include <QObject>

Camera::Camera(PhotoBooth* photoBooth, uint camId, uint fps) :
    m_isRunning(false),
    m_photoBooth(photoBooth),
    m_camId(camId),
    m_fps(fps),
    m_timer(nullptr)
{
    start();
}

Camera::~Camera()
{

}

void Camera::loop()
{
    while(m_isRunning){
        qDebug() << QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss,zzz");
    }
}

void Camera::start()
{
    m_cap.open(m_camId);
    if (!m_cap.isOpened()){
        qDebug() << "ERROR: Impossible to connect to camera" << m_camId;
        return;
    }
    m_isRunning = true;

    m_timer = new QTimer(m_photoBooth);
    //QObject::connect(m_timer, &QTimer::timeout, &Camera::loop);
    m_timer->start((int)(1000/m_fps));
}

void Camera::stop()
{
    m_isRunning = false;
}
