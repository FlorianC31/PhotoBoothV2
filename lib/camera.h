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
    Camera(QLabel* camView, uint camId, uint resolutionMode, bool upsideDown, bool mirror);
    ~Camera();

    void start();
    void stop();

private:
    bool m_isRunning;
    QLabel* m_camView;
    uint m_camId;
    cv::VideoCapture* m_cap;
    QThread* m_cameraThread;
    int m_resolution[2];
    int m_cropLeft;
    int m_cropWidth;
    bool m_upsideDown;
    bool m_mirror;

public slots:
    void loop();

};

#endif // CAMERA_H
