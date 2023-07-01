#pragma once
#ifndef PRINTER_H
#define PRINTER_H

#include <QtCore>

class Printer : public QObject
{
    Q_OBJECT

public:
    Printer(bool rotate, bool watermark);
    ~Printer();

private:
    QThread* m_thread;
    bool m_rotate;
    bool m_watermark;

public slots:
    void print(QString photoPath, unsigned int nbPrint);
};

#endif // PRINTER_H
