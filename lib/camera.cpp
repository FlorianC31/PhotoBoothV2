#include "Camera.h"
#include <QDebug>

#define RESOLUTION0_H 1920
#define RESOLUTION0_V 1080
#define RESOLUTION1_H 1280
#define RESOLUTION1_V 720
#define RESOLUTION2_H 640
#define RESOLUTION2_V 360
#define CROP_WIDTH 1620
#define CROP_HEIGHT 1080
#define RATIO_16_9 1.77777777777777777778
#define RATIO_3_2 1.5
#define MARGIN 1;

#define SLEEP_TIME 100 // Time in ms to wait between 2 tries to get the last photo
#define TRIES_NUMBER 50 // Number of tries to get the last photo

/**
 * @brief Camera::Camera Class to get frame from camera connected as a webcam on the PC
 * @param camView QLabel used by the UI to display the frames
 * @param camId camera ID to connect
 * @param resolutionMode 0: 1920-1080 - 1:1280-720 - 2 -640-320
 * @param upsideDown rotate the frame to 180°
 * @param mirror mirroring the frame
 */
Camera::Camera(QLabel* camView, uint camId, uint resolutionMode, bool upsideDown, bool mirror) :
    m_isRunning(false),
    m_camView(camView),
    m_camId(camId),
    m_cap(nullptr),
    m_cameraThread(nullptr),
    m_upsideDown(upsideDown),
    m_mirror(mirror)
{
    m_cameraThread = new QThread();
    m_cameraThread->start();
    this->moveToThread(m_cameraThread);

    int resTable[3][2];
    resTable[0][0] = RESOLUTION0_H;
    resTable[0][1] = RESOLUTION0_V;
    resTable[1][0] = RESOLUTION1_H;
    resTable[1][1] = RESOLUTION1_V;
    resTable[2][0] = RESOLUTION2_H;
    resTable[2][1] = RESOLUTION2_V;

    // if the value of resolutionMode is different to 0, 1 or 2, the default value 1 is automatically set
    if (resolutionMode > 2) {
        resolutionMode = 1;
    }
    m_resolution[0] = resTable[resolutionMode][0];
    m_resolution[1] = resTable[resolutionMode][1];

    qDebug() << "Resolution mode:" << resolutionMode << "- width=" << resTable[resolutionMode][0] << "- height=" << resTable[resolutionMode][1];

    m_cropLeft =  round((RATIO_16_9 - RATIO_3_2) * m_resolution[1] / 2) + MARGIN;
    m_cropWidth = RATIO_3_2 * m_resolution[1] -  2 * MARGIN;

    // Create a video capture object using the DirectShow backend
    int nbTries = TRIES_NUMBER;
    do {
        nbTries--;
        Sleep(SLEEP_TIME);

        m_cap = new cv::VideoCapture(1, cv::CAP_DSHOW);
        m_cap->open(m_camId);

        if (nbTries == 0) {
            qDebug() << "ERROR: Impossible to connect to camera";
            return;
        }

    } while(!m_cap->isOpened());

    // Set resolution
    m_cap->set(cv::CAP_PROP_FRAME_WIDTH, m_resolution[0]);
    m_cap->set(cv::CAP_PROP_FRAME_HEIGHT, m_resolution[1]);
}

/**
 * @brief Camera::~Camera destructor
 */
Camera::~Camera()
{
    m_isRunning = false;
    delete m_cameraThread;
    delete m_cap;
}

/**
 * @brief Camera::loop reading and processing each frame in the main loop
 */
void Camera::loop()
{
    // If camera is not running, return direclty
    if (!m_isRunning)
        return;

    // Create and get raw frame from webcam
    cv::Mat frame;
    (*m_cap) >> frame;

    // Mirror flip
    if (m_mirror) {
        cv::flip(frame, frame, 1);
    }

    // Rotation 180°
    if (m_upsideDown) {
        cv::rotate(frame, frame, cv::ROTATE_180);
    }

    // Crop & Resize
    cv::Rect rect(m_cropLeft, 0, m_cropWidth, m_resolution[1]);
    cv::Size newSize(CROP_WIDTH, CROP_HEIGHT);
    cv::resize(frame(rect), frame, newSize);

    // Transform frame for Qt and send it to the UI
    QImage qt_image;
    cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    qt_image = QImage((const unsigned char*) (frame.data), frame.cols, frame.rows, QImage::Format_RGB888);
    m_camView->setPixmap(QPixmap::fromImage(qt_image));
}

/**
 * @brief Camera::start start the camera
 */
void Camera::start()
{
    // If camera is already running, exit start function
    if (m_isRunning)
        return;

    m_isRunning = true;
}

/**
 * @brief Camera::stop Stop the camera
 */
void Camera::stop()
{
    m_isRunning = false;
}
