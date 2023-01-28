#include "photobooth.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PhotoBooth photoBooth;
    photoBooth.show();
    return a.exec();
}
