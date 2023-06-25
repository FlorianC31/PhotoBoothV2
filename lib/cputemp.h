#ifndef CPUTEMP_H
#define CPUTEMP_H

#include <QtCore>
#include "relay.h"

class RelayDevice;

class CpuTemp : public QObject
{
    Q_OBJECT

public:
    CpuTemp(RelayDevice* cpuFan);
    ~CpuTemp();

private:
    QThread* m_thread;
    RelayDevice* m_cpuFan;

public slots:
    void loop();
};

#endif // CPUTEMP_H
