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
    Camera(QLabel* camView, uint camId);
    ~Camera();

    void start();
    void stop();

private:
    bool m_isRunning;
    QLabel* m_camView;
    uint m_camId;
    cv::VideoCapture m_cap;
    QThread* m_cameraThread;

public slots:
    void loop();

};

#endif // CAMERA_H
