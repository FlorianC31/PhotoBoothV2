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
    Photo(QString photoFolder, uint isoMax, bool m_addWatermark, QSize viewerSize);
    ~Photo();
    void getLast(QPixmap &lastPhoto, QPixmap &lastPhoto2Print);
    bool isThereNew();
    bool checkIso();

private:
    QPixmap pasteWatermark(QPixmap &photo);
    QString getLastJpg();

    uint m_isoMax;
    QDir m_folder;
    bool m_addWatermark;
    QPixmap m_watermarkPng;
    QSize m_resizedPhotoSize;
    QPoint m_watermarkPos;
    QString m_pathToRecentFile;
    QSize m_viewerSize;
};

#endif // PHOTO_H
