#pragma once
#ifndef RELAY_H
#define RELAY_H

#include <Windows.h>
#include <ftd2xx.h>
#include <QtCore>

#include <photobooth.h>

class PhotoBooth;

class Relay : public QObject
{
    Q_OBJECT

public:
    Relay(PhotoBooth* photoBooth, unsigned int deviceNumber);
    ~Relay();    

    bool isConnected() { return m_isConnected;};
    FT_HANDLE* getFtHandle() { return m_ftHandle;};
    void set(unsigned char slotId, bool state);
    bool connection();

private:
    PhotoBooth* m_photoBooth;
    QThread* m_thread;
    bool m_isConnected;
    FT_HANDLE* m_ftHandle;
    unsigned int m_deviceNumber;


signals:
    void endOfLoading();
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
