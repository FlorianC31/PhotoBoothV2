#include "photo.h"
#include "exif.h"

#include <QDebug>

#define PRINT_SIZE_W 6 // in inch
#define PRINT_SIZE_H 4 // in inch
#define RESOLUTION 300 // in dpi
#define WATERMARK_RIGHT 20 // Distance in px from the right of the watermark to the right of the photo
#define WATERMARK_BOTTOM 40 // Distance in px from the bottom of the watermark to the bottom of the photo

#define SLEEP_TIME 100 // Time in ms to wait between 2 tries to get the last photo
#define TRIES_NUMBER 50 // Number of tries to get the last photo


Photo::Photo(QString photoFolder, uint isoMax, bool addWatermark, QSize viewerSize) :
    m_isoMax(isoMax),
    m_addWatermark(addWatermark),
    m_viewerSize(viewerSize)
{
    m_folder = QDir(photoFolder);
    m_pathToRecentFile = getLastJpg();

    if (m_addWatermark) {
        // Load the watermark image into another QPixmap object
        m_watermarkPng = QPixmap(QString::fromUtf8("ressources/logo_blanc_sur_transparent.png"));

        // Calculation of the watermak position
        int posX = PRINT_SIZE_W * RESOLUTION - m_watermarkPng.size().width() - WATERMARK_RIGHT;
        int posY = PRINT_SIZE_H * RESOLUTION - m_watermarkPng.size().height() - WATERMARK_BOTTOM;
        m_watermarkPos = QPoint(posX, posY);
    }

    m_resizedPhotoSize = QSize(PRINT_SIZE_W * RESOLUTION, PRINT_SIZE_H * RESOLUTION);
}


Photo::~Photo()
{
}

bool Photo::loadLast()
{
    int nbTries = TRIES_NUMBER;

    QString oldPhotoPath = m_pathToRecentFile;

    do {
        nbTries--;
        Sleep(SLEEP_TIME);

        if (nbTries == 0) {
            qDebug() << "ERROR: no new photo has been detected";
            return false;
        }

        m_pathToRecentFile = getLastJpg();
    } while(m_pathToRecentFile == oldPhotoPath || m_pathToRecentFile == "");

    return true;
}


/**
 * @brief Photo::getLast get the last photo from the photo folder
 * @param lastPhoto reference to the destination photo QPixmap for display
 * @param lastPhoto2Print reference to the destination photo QPixmap for printing
 * @return true if the last photo has been loaded and false if no new photo have been detected after 50 tries of 100ms
 */
bool Photo::getLast(QPixmap &lastPhoto, QPixmap &lastPhoto2Print)
{
    if (!loadLast())
        return false;

    // Get the last photo
    QPixmap newPhoto = QPixmap(m_pathToRecentFile);

    // Resize the photo
    lastPhoto2Print = newPhoto.scaled(m_resizedPhotoSize);

    if (m_addWatermark) {
        lastPhoto2Print = pasteWatermark(lastPhoto2Print);
    }

    // Return the final photo
    lastPhoto = newPhoto.scaled(m_viewerSize);
    return true;
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
        return fileList.front().absoluteFilePath();
    }
    return "";
 }

/**
 * @brief Photo::addWatermark add the watermark on the photo
 * @param photo reference to the photo where the watermak has to be added
 */
QPixmap Photo::pasteWatermark(QPixmap &photo)
{
    QPixmap result(photo.width(), photo.height());
    result.fill(Qt::transparent); // force alpha channel

    // Create a QPainter object to draw on the image
    QPainter painter(&result);

    // Draw the watermark image on the original image
    painter.drawPixmap(0, 0, photo);
    painter.drawPixmap(m_watermarkPos, m_watermarkPng);
    painter.end();
    return result;
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
      qDebug() << "ERROR: CheckISO - Can't open file.";
      return false;
    }
    fseek(fp, 0, SEEK_END);
    unsigned long fsize = ftell(fp);
    rewind(fp);
    unsigned char *buf = new unsigned char[fsize];
    if (fread(buf, 1, fsize, fp) != fsize) {
      qDebug() << "ERROR: CheckISO - Can't read file.";
      delete[] buf;
      return false;
    }
    fclose(fp);

    // Parse EXIF
    easyexif::EXIFInfo result;
    int code = result.parseFrom(buf, fsize);
    delete[] buf;
    if (code) {
      qDebug() << "ERROR: CheckISO - Error parsing EXIF: code" << code;
      return -3;
    }

    return result.ISOSpeedRatings > m_isoMax;
}
