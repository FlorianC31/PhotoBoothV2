#include "videoflow.h"

VideoFlow::VideoFlow(PhotoBooth* photoBooth, int camId) :
    m_isRunning(false),
    m_photoBooth(photoBooth),
    m_camId(camId)
{
    start();
}

void VideoFlow::loop()
{
    while(m_isRunning){

    }
}

void VideoFlow::start()
{
    cv::VideoCapture cap;
    cap.open(m_camId);
    m_isRunning = true;
}

void VideoFlow::stop()
{
    m_isRunning = false;
}
