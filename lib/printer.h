#pragma once
#ifndef PRINTER_H
#define PRINTER_H

#include <QtCore>

class Printer : public QObject
{
    Q_OBJECT

public:
    Printer(bool rotate, bool watermark, QString settingFile);
    ~Printer();

private:
    QThread* m_thread;
    bool m_rotate;
    bool m_watermark;
    QString m_settingFile;
    QFile* m_logFile;

public slots:
    void print(QString photoPath, unsigned int nbPrint, unsigned int printCounter);
};

#endif // PRINTER_H
