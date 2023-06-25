#pragma once
#ifndef RELAY_H
#define RELAY_H

#include <ftd2xx.h>

#include <photobooth.h>

class PhotoBooth;

class Relay
{

public:
    Relay(PhotoBooth* photoBooth, unsigned int deviceNumber);
    ~Relay();

    bool isConnected() { return m_isConnected;};
    //FT_HANDLE* getFtHandle() { return m_ftHandle;};
    void set(unsigned char slotId, bool on, bool AC);

private:
    PhotoBooth* m_photoBooth;
    bool m_isConnected;
    FT_HANDLE m_ftHandle;
    unsigned int m_deviceNumber;
};


class RelayDevice
{
public:
    RelayDevice(Relay* m_Relay, QString name, unsigned int port, bool AC = false);
    void on();
    void off();

private:
    Relay* m_relay;
    unsigned int m_port;
    unsigned char m_slotId;
    QString m_name;
    bool m_AC;
};

#endif // RELAY_H
