#include "relay.h"
#include <QDebug>
#include <cmath>

#define MAX_TRY_CONNECT 5       // Number of relay initial connection tries
#define WAIT_TIME 5000          // (ms) Delay between 2 tries
#define ELECTRICITY_FREQ 50     // (Hz) AC Electicity network frequency

Relay::Relay(PhotoBooth* photoBooth, unsigned int deviceNumber) :
    m_photoBooth(photoBooth),
    m_isConnected(false),
    m_deviceNumber(deviceNumber)
{
    FT_STATUS ftStatus;

    ftStatus = FT_Open(0, &m_ftHandle);
    if(ftStatus == FT_OK){
        ftStatus = FT_SetBitMode(m_ftHandle, 0x0F, 1); // IMPORTANT TO HAVE: This sets up the FTDI device as "Bit Bang" mode
        if(ftStatus == FT_OK){
            qDebug() << "RELAY - Connection OK";
            m_isConnected = true;
        }
        else{
            qDebug() << "RELAY - Setting Bit Bang FAILED with code" << ftStatus;
            m_isConnected = false;
        }
    }
    else{
        qDebug() << "RELAY - Connection FAILED with code" << ftStatus;
        m_isConnected = false;
    }
}

Relay::~Relay(){
    unsigned char zero = 0;
    DWORD* lpdwBytesWritten = new DWORD;
    FT_Write(m_ftHandle, &zero, 1, lpdwBytesWritten);
}

void Relay::set(unsigned char slotId, bool on, bool AC)
{
    if(m_isConnected){
        unsigned char* data = new UCHAR;
        FT_STATUS ftStatus;
        ftStatus = FT_GetBitMode(m_ftHandle, data);

        if (ftStatus != FT_OK){
            return;
        }

        unsigned char newData;

        if (on) {
            newData = *data | slotId;
        }
        else {
            newData = *data & ~slotId;
        }

        DWORD* lpdwBytesWritten = new DWORD;

        FT_Write(m_ftHandle, &newData, 1, lpdwBytesWritten);
        if (AC){
            QThread::msleep(1000 / ELECTRICITY_FREQ / 4);
            // Sleep 1/4 of electricty AC cycle and redo action to be sure to avoid to activate relay when current = 0 which cause the non-locking of the relay (in millisecond)
            FT_Write(m_ftHandle, &newData, 1, lpdwBytesWritten);
        }
    }
}


RelayDevice::RelayDevice(Relay* relay, QString name, unsigned int port, bool AC) :
    m_relay(relay),
    m_port(port),
    m_name(name),
    m_AC(AC)
{
    // Get bit from port:
    // 1 -> 0001 -> 1
    // 2 -> 0010 -> 2
    // 3 -> 0100 -> 4
    // 4 -> 1000 -> 8
    m_slotId = std::pow(2, port - 1);
    qDebug() << "RELAY - Creation of" << m_name << "on port" << m_port << "- SlotId:" << m_slotId;
}

void RelayDevice::on()
{
    qDebug() << "RELAY - Setting" << m_name << "on";
    m_relay->set(m_slotId, true, m_AC);
}
void RelayDevice::off()
{
    qDebug() << "RELAY - Setting" << m_name << "off";
    m_relay->set(m_slotId, false, m_AC);
 }
