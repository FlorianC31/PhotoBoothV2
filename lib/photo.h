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
    Photo(PhotoBooth* photoBooth, QString photoFolder, uint isoMax, QSize viewerSize, bool rotate);
    ~Photo();
    bool checkIso();
    inline bool isInitialized() {return m_intialized;};

private:
    QString getLastJpg();

    PhotoBooth* m_photoBooth;
    QThread* m_thread;
    uint m_isoMax;
    QDir m_folder;
    QString m_pathToRecentFile;
    QSize m_viewerSize;
    QPixmap m_lastPhoto;
    bool m_intialized;
    bool m_rotate;
    bool m_lastPhotoFailed;

public slots:
    void loadLast();

signals:
    void sendNewPhoto();
};

#endif // PHOTO_H
