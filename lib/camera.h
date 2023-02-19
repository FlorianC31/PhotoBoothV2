#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <QtCore>
#include <QLabel>

#include <photobooth.h>

class PhotoBooth;

class Camera : public QObject
{
    Q_OBJECT

public:
    Camera(QLabel* camView, uint camId, uint fps);
    ~Camera();

    void start();
    void stop();

private:
    bool m_isRunning;
    QLabel* m_camView;
    uint m_camId;
    uint m_fps;
    cv::VideoCapture m_cap;
    QTimer* m_timer;
    QThread* m_cameraThread;

private slots:
    void loop();

};

#endif // CAMERA_H
