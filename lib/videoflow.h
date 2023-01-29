#pragma once
#ifndef VIDEOFLOW_H
#define VIDEOFLOW_H

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <QtCore>

#include <photobooth.h>

class PhotoBooth;

class VideoFlow
{
public:
    VideoFlow(PhotoBooth* photoBooth, int camId);
    ~VideoFlow();

    void loop();
    void start();
    void stop();

private:
    bool m_isRunning;
    PhotoBooth* m_photoBooth;
    int m_camId;

    //cv::VideoCapture* m_cap;

    //cv::Mat frame;
    //QImage qt_image;
};

#endif // VIDEOFLOW_H
