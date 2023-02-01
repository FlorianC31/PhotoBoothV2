#include "relay.h"
#include <QDebug>
#include <windows.h>
#include <cmath>

#define MAX_TRY_CONNECT 5       // Number of relay initial connection tries
#define WAIT_TIME 2             // (secondes) Delay between 2 tries
#define ELECTRICITY_FREQ 50     // (Hz) AC Electicity network frequency

Relay::Relay(unsigned int deviceNumber) :
    m_isConnected(false),
    m_ftHandle(nullptr),
    m_deviceNumber(deviceNumber)
{
}

Relay::~Relay()
{

}

bool Relay::connect()
{
    FT_STATUS ftStatus;


    ftStatus = FT_Open(m_deviceNumber, m_ftHandle);

    unsigned int nbTry = 1;
    while (ftStatus != FT_OK && nbTry < MAX_TRY_CONNECT) {
        Sleep(WAIT_TIME * 1000);
        ftStatus = FT_Open(m_deviceNumber, m_ftHandle);
        nbTry++;
    }

    if (ftStatus != FT_OK) {
        qDebug() << "Error: Impossible to connect to relay after" << MAX_TRY_CONNECT << "tries;";
        return false;
    }

    FT_SetBitMode(m_ftHandle, 0xFF, 0x01); // IMPORTANT TO HAVE: This sets up the FTDI device as "Bit Bang" mode

    return true;
}

void Relay::set(unsigned char slotId, bool transition)
{
    unsigned char ucEnable;

    if (transition)
        ucEnable = 8; // True -> 1111 -> 8
    else
        ucEnable = 0; // True -> 0000 -> 0

    FT_SetBitMode(m_ftHandle, slotId, ucEnable);
    // Sleep 1/4 of electricty AC cycle and redo action to be sure to avoid to activate relay when current = 0 which cause the non-locking of the relay (in millisecond)
    Sleep(1000 / ELECTRICITY_FREQ / 4);
    FT_SetBitMode(m_ftHandle, slotId, ucEnable);
}


RelayDevice::RelayDevice(Relay* relay, unsigned int port) :
    m_relay(relay),
    m_port(port)
{
    // Get bit from port:
    // 1 -> 0001 -> 1
    // 2 -> 0010 -> 2
    // 3 -> 0100 -> 4
    // 4 -> 1000 -> 8
    m_slotId = std::pow(2, port);
}

RelayDevice::~RelayDevice()
{

}

void RelayDevice::on() {m_relay->set(m_slotId, true);}
void RelayDevice::off() {m_relay->set(m_slotId, false);}
