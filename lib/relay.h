#pragma once
#ifndef RELAY_H
#define RELAY_H

#include <Windows.h>
#include <ftd2xx.h>


class Relay
{
public:
    Relay(unsigned int deviceNumber);
    ~Relay();

    bool connect();
    bool isConnected() { return m_isConnected;};
    FT_HANDLE* getFtHandle() { return m_ftHandle;};
    void set(unsigned char slotId, bool state);

private:
    bool m_isConnected;
    FT_HANDLE* m_ftHandle;
    unsigned int m_deviceNumber;
};


class RelayDevice
{
public:
    RelayDevice(Relay* m_Relay, unsigned int port);
    ~RelayDevice();
    void on();
    void off();

private:
    Relay* m_relay;
    unsigned int m_port;
    unsigned char m_slotId;
};

#endif // RELAY_H
