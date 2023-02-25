#pragma once
#ifndef PHOTO_H
#define PHOTO_H

#include <windows.h>

#include <QtCore>
#include <QPixmap>
#include <QPainter>

class Photo
{
public:
    Photo(QString photoFolder, uint isoMax, bool m_addWatermark);
    ~Photo();
    bool getLast(QPixmap &lastPhoto);
    bool checkIso();

private:
    void addWatermark(QPixmap &photo);
    QString getLastJpg();

    uint m_isoMax;
    QDir m_folder;
    bool m_addWatermark;
    QPixmap m_watermarkPng;
    QSize m_resizedPhotoSize;
    QPoint m_watermarkPos;
    QString oldPhotoPath;
};

#endif // PHOTO_H
