#include "cputemp.h"
#include <comdef.h>
#include <Wbemidl.h>

CpuTemp::CpuTemp(RelayDevice* cpuFan) :
    m_thread(nullptr),
    m_cpuFan(cpuFan)

{
    m_thread = new QThread();
    m_thread->start();
    this->moveToThread(m_thread);
}
