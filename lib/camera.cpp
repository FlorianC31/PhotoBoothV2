#include "Camera.h"
#include <QDebug>

Camera::Camera(QLabel* camView, uint camId, uint fps) :
    m_isRunning(false),
    m_camView(camView),
    m_camId(camId),
    m_fps(fps),
    m_timer(nullptr),
    m_cameraThread(nullptr)
{
    m_cameraThread = new QThread();
    m_cameraThread->start();
    this->moveToThread(m_cameraThread);
    start();
}

Camera::~Camera()
{
    m_isRunning = false;
    m_timer->stop();
    delete m_timer;
}

void Camera::loop()
{
    if (!m_isRunning)
        return;

    cv::Mat frame;
    QImage qt_image;
    m_cap >> frame;
    cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    qt_image = QImage((const unsigned char*) (frame.data), frame.cols, frame.rows, QImage::Format_RGB888);

    m_camView->setPixmap(QPixmap::fromImage(qt_image));

}

void Camera::start()
{
    if (m_isRunning)
        return;

    m_cap.open(m_camId);
    if (!m_cap.isOpened()){
        qDebug() << "ERROR: Impossible to connect to camera" << m_camId;
        return;
    }
    m_isRunning = true;

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &Camera::loop);        // Problème ici
    m_timer->start(1000/m_fps);
}

void Camera::stop()
{
    m_isRunning = false;
}
