#include "cputemp.h"
#include <QDebug>
#include <QCoreApplication>
#include <QProcess>

/**
 * @brief CpuTemp::CpuTemp
 * @param cpuFan pointer to the cpu fan relay device
 * @param minTemp (int) minimal temperature to turn off the pc fan
 * @param maxTemp (int) maximal temperature to turn on the pc fan
 */
CpuTemp::CpuTemp(RelayDevice* cpuFan, int minTemp, int maxTemp) :
    m_thread(nullptr),
    m_cpuFan(cpuFan),
    m_fanRunning(false),
    m_minTemp(minTemp),
    m_maxTemp(maxTemp)

{
    // Move this object in a thread
    m_thread = new QThread();
    m_thread->start();
    this->moveToThread(m_thread);

    // If Open Hardware Monitor is not open, open it
    HWND handle = FindWindowA(NULL, "Open Hardware Monitor");
    if (handle == NULL) {
        qDebug() << "CPUTEMP - ERROR: Open Hardware is not open";
    }
}

/**
 * @brief CpuTemp::~CpuTemp destructor
 */
CpuTemp::~CpuTemp()
{
    m_thread->quit();
    m_thread->wait();
    delete m_thread;
}

/**
 * @brief CpuTemp::loop cpuTemp loop to periodic check the CPU temperature and turn on the PcFan if necessary
 */
void CpuTemp::loop()
{
    // Execute Python script and get the output
    QProcess* process = new QProcess;
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->start("python", QStringList("getCpuTemp.py"));
    process->waitForFinished();
    QByteArray output = process->readAllStandardOutput();
    delete process;

    bool isDouble;
    double cpuTemp = output.toDouble(&isDouble);

    if (isDouble){
        if(cpuTemp > m_maxTemp && !m_fanRunning){
            m_fanRunning = true;
            m_cpuFan->on();
            qDebug() << "CPUTEMP - CpuTemperature =" << cpuTemp << " (superior to" << m_maxTemp <<") -> Turning PC fan on.";
        }
        else if(cpuTemp < m_minTemp && m_fanRunning){
            m_fanRunning = false;
            m_cpuFan->off();
            qDebug() << "CPUTEMP - CpuTemperature =" << cpuTemp << " (inferior to" << m_minTemp <<") -> Turning PC fan off.";
        }
    }
    else{
        qDebug() << "CPUTEMP - ERROR while executing python file";
    }
}
