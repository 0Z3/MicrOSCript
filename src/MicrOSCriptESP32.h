#ifndef MICROSCRIPTESP32_H
#define MICROSCRIPTESP32_H

#include "MicrOSCript.h"
#include "Arduino.h"
#include "esp_system.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include "ose_conf.h"
#include "libose/ose.h"

#define MICROSCRIPT_UDP_BUFFER_SIZE 65536

class MicrOSCriptESP32 : public MicrOSCript
{
private:
    char *udp_buffer;

    IPAddress wifi_ip_address;
    unsigned int udp_port;

public:
    WiFiUDP udp;

    void init(int32_t vm_x_size
              = MICROSCRIPT_VMX_SIZE);
    void init(const char * const ssid,
              const char * const pass,
              int32_t vm_x_size
              = MICROSCRIPT_VMX_SIZE);
    void init(const char * const ssid,
              const char * const pass,
              unsigned int port,
              int32_t udp_buffer_size
              = MICROSCRIPT_UDP_BUFFER_SIZE,
              int32_t vm_x_size
              = MICROSCRIPT_VMX_SIZE);

    uint64_t serviceInterrupts(void);
    void attachInterruptTo(int pin, int direction);

    boolean udpRead(void);
    int32_t udpSendElemTo(IPAddress addr,
                          uint16_t port,
                          ose_bundle bundle);

    int32_t slipSerialRead(void);
    
private:
    void connectToWiFi(const char * const ssid,
                         const char * const pass);
    void udpListen(unsigned int port);

};

#endif
