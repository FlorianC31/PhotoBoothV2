#include "photo.h"
#include "exif.h"

#include <QDebug>

#define WATERMARK_RIGHT 20 // Distance in px from the right of the watermark to the right of the photo
#define WATERMARK_BOTTOM 40 // Distance in px from the bottom of the watermark to the bottom of the photo

#define SLEEP_TIME 100 // Time in ms to wait between 2 tries to get the last photo
#define TRIES_NUMBER 50 // Number of tries to get the last photo


Photo::Photo(PhotoBooth* photoBooth, QString photoFolder, uint isoMax, QSize viewerSize, bool rotate) :
    m_photoBooth(photoBooth),
    m_isoMax(isoMax),
    m_viewerSize(viewerSize),
    m_intialized(false),
    m_rotate(rotate)
{
    m_thread = new QThread();
    m_thread->start();
    this->moveToThread(m_thread);

    connect(this, &Photo::sendNewPhoto, m_photoBooth, [this]() {
        m_photoBooth->loadNewPhoto(&m_lastPhoto, m_pathToRecentFile);
    });

    m_folder = QDir(photoFolder);
    m_pathToRecentFile = getLastJpg();
}


Photo::~Photo()
{
    m_thread->quit();
    m_thread->wait();
    delete m_thread;
}

void Photo::loadLast()
{
    int nbTries = TRIES_NUMBER;

    QString oldPhotoPath = m_pathToRecentFile;
    m_pathToRecentFile = getLastJpg();

    do {
        nbTries--;
        QThread::msleep(SLEEP_TIME);

        if (nbTries == 0) {
            qDebug() << "PHOTO - ERROR: no new photo has been detected";
            return;
        }

        m_pathToRecentFile = getLastJpg();
    } while(m_pathToRecentFile == oldPhotoPath || m_pathToRecentFile == "");

    // Get the last photo
    QPixmap newPhoto(m_pathToRecentFile);

    // Return the final photo
    if (m_rotate) {
        QTransform transform;
        transform.rotate(180);
        m_lastPhoto = newPhoto.scaled(m_viewerSize).transformed(QTransform().rotate(180));
    }
    else {
        m_lastPhoto = newPhoto.scaled(m_viewerSize);
    }

    m_intialized = true;
    emit sendNewPhoto();
}

/**
 * @brief Photo::getLastJpg get the path to last jpg file in the photos folder
 * @return path to the last jpg file in the photos folder
 */
QString Photo::getLastJpg()
{
    // get the list of picture .jpg sorted by time
    QFileInfoList fileList = m_folder.entryInfoList(QStringList() << "*.JPG", QDir::Files, QDir::Time);

    if (!fileList.isEmpty()) {
        // Get the path to the most recent file in the list:
        qDebug() << "PHOTO - Last JPG:" << fileList.front().absoluteFilePath();
        return fileList.front().absoluteFilePath();
    }
    qDebug() << "PHOTO - Photo Folder is empty";
    return "";
 }

/**
 * @brief Photo::checkIso check if Iso are higher than max
 * @return true if iso are higher than the max
 */
bool Photo::checkIso()
{
    // Read the JPEG file into a buffer
    FILE *fp = std::fopen(m_pathToRecentFile.toStdString().c_str(), "rb");
    if (!fp) {
      qDebug() << "PHOTO - ERROR: CheckISO - Can't open file.";
      return false;
    }
    fseek(fp, 0, SEEK_END);
    unsigned long fsize = ftell(fp);
    rewind(fp);
    unsigned char *buf = new unsigned char[fsize];
    if (fread(buf, 1, fsize, fp) != fsize) {
      qDebug() << "PHOTO - ERROR: CheckISO - Can't read file.";
      delete[] buf;
      return false;
    }
    fclose(fp);

    // Parse EXIF
    easyexif::EXIFInfo result;
    int code = result.parseFrom(buf, fsize);
    delete[] buf;
    if (code) {
      qDebug() << "PHOTO - ERROR: CheckISO - Error parsing EXIF: code" << code;
      return -3;
    }

    return result.ISOSpeedRatings > m_isoMax;
}
