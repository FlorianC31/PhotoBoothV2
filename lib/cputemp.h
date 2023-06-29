#pragma once
#ifndef CPUTEMP_H
#define CPUTEMP_H

#include <QtCore>
#include "relay.h"

class RelayDevice;

class CpuTemp : public QObject
{
    Q_OBJECT

public:
    CpuTemp(RelayDevice* cpuFan, int minTemp, int maxTemp);
    ~CpuTemp();

private:
    QThread* m_thread;
    RelayDevice* m_cpuFan;
    bool m_fanRunning;
    int m_minTemp;
    int m_maxTemp;

public slots:
    void loop();
};

#endif // CPUTEMP_H
