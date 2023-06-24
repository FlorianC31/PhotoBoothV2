#pragma once
#ifndef PRINTER_H
#define PRINTER_H

#include <QtCore>

class Printer
{
public:
    Printer(bool rotate);
    void print(QPixmap* photo, unsigned int nbPrint);

private:
    bool m_rotate;
};

#endif // PRINTER_H
