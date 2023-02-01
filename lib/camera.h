#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <QtCore>
#include <QObject>

#include <photobooth.h>

class PhotoBooth;

class Camera
{
public:
    Camera(PhotoBooth* photoBooth, uint camId, uint fps);
    ~Camera();

    void loop();
    void start();
    void stop();

private:
    bool m_isRunning;
    PhotoBooth* m_photoBooth;
    uint m_camId;
    uint m_fps;
    cv::VideoCapture m_cap;
    QTimer* m_timer;

    cv::Mat frame;
    QImage qt_image;
};

#endif // CAMERA_H
