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
    Camera(PhotoBooth* photobooth, QLabel* camView, uint camId, uint resolutionMode, bool upsideDown, bool mirror);
    ~Camera();
    inline bool isLoaded() {return m_cap->isOpened();};

    void connection();
    void start();
    void stop();

private:
    PhotoBooth* m_photoBooth;
    bool m_isRunning;
    QLabel* m_camView;
    uint m_camId;
    cv::VideoCapture* m_cap;
    QThread* m_thread;
    int m_resolution[2];
    int m_cropLeft;
    int m_cropWidth;
    bool m_upsideDown;
    bool m_mirror;

public slots:
    void loop();

signals:
    void endOfLoading();
};

#endif // CAMERA_H
