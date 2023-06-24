#include "printer.h"
#include <QDebug>

#include <QPainter>
#include <QPrinter>

Printer::Printer(bool rotate) : m_rotate(rotate)
{
}

void Printer::print(QPixmap* photo, unsigned int nbPrint){
    qDebug() << "nbPrint:" << nbPrint;

    QImage image = photo->toImage();
    QImage* finalImage = &image;

    if (m_rotate) {
        QImage imageRotated = image.mirrored(true, true);
        finalImage = &imageRotated;
    }

    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QSize(6, 4), QPageSize::Inch));

    for (unsigned int i = 0; i < nbPrint; i++){
        QPainter painter(&printer);
        painter.drawImage(QPointF(0, 0), *finalImage);
        painter.end();
    }

}
