#pragma once
#ifndef PHOTO_H
#define PHOTO_H

#include <windows.h>

#include <QtCore>
#include <QPixmap>
#include <QPainter>

#include <photobooth.h>

class PhotoBooth;

class Photo : public QObject
{
    Q_OBJECT

public:
    Photo(PhotoBooth* photoBooth, QString photoFolder, uint isoMax, bool m_addWatermark, QSize viewerSize);
    ~Photo();
    bool checkIso();
    inline bool isInitialized() {return m_intialized;};

private:
    QPixmap pasteWatermark(QPixmap &photo);
    QString getLastJpg();

    PhotoBooth* m_photoBooth;
    QThread* m_thread;
    uint m_isoMax;
    QDir m_folder;
    bool m_addWatermark;
    QPixmap m_watermarkPng;
    QSize m_resizedPhotoSize;
    QPoint m_watermarkPos;
    QString m_pathToRecentFile;
    QSize m_viewerSize;
    QPixmap m_lastPhoto2Print;
    QPixmap m_lastPhoto;
    bool m_intialized;

public slots:
    void loadLast();

signals:
    void sendNewPhoto();
};

#endif // PHOTO_H
