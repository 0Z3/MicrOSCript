#include "MicrOSCriptESP32.h"

#include "ose_conf.h"
#include "libose/ose_assert.h"
#include "libose/ose_util.h"
#include "libose/ose_context.h"
#include "libose/ose_stackops.h"
#include "libose/ose_vm.h"
#include "libose/ose_print.h"
#include "o.se.stdlib/ose_stdlib.h"
#include "o.se.oscript/ose_oscript.h"

static const char *TAG = "MicrOSCriptESP32";

static void attachInterruptTo(ose_bundle osevm);
static void udpSendTo(ose_bundle osevm);

void MicrOSCriptESP32::init(int32_t vm_x_size)
{
    MicrOSCript::init(vm_x_size);
}

void MicrOSCriptESP32::init(const char * const ssid,
                            const char * const pass,
                            int32_t vm_x_size)
{
    MicrOSCript::init(vm_x_size);
    connectToWiFi(ssid, pass);
    ose_pushMessage(vm_x,
                    "/ipv4/address/string",
                    strlen("/ipv4/address/string"),
                    1,
                    OSETT_STRING,
                    wifi_ip_address.toString().c_str());
    ose_pushMessage(vm_x,
                    "/ipv4/address/int",
                    strlen("/ipv4/address/int"),
                    4,
                    OSETT_INT32, wifi_ip_address[0],
                    OSETT_INT32, wifi_ip_address[1],
                    OSETT_INT32, wifi_ip_address[2],
                    OSETT_INT32, wifi_ip_address[3]);
}

void MicrOSCriptESP32::init(const char * const ssid,
                            const char * const pass,
                            const unsigned int port,
                            int32_t udp_buffer_size,
                            int32_t vm_x_size)
{
    init(ssid, pass, vm_x_size);
    udpListen(port);
    udp_buffer =
        (char *)malloc(udp_buffer_size);
    ose_pushMessage(vm_x,
                    "/udp/port/listening",
                    strlen("/udp/port/listening"),
                    1, OSETT_INT32, port);
    ose_pushMessage(vm_x,
                    "/udp/sendto",
                    strlen("/udp/sendto"),
                    1, OSETT_ALIGNEDPTR, udpSendTo);
}

/************************************************************
	Interrupts
************************************************************/

#define GPIO_4_MASK 0x10ULL
#define GPIO_5_MASK 0x20ULL
#define GPIO_12_MASK 0x1000ULL
#define GPIO_13_MASK 0x2000ULL
#define GPIO_14_MASK 0x4000ULL
#define GPIO_15_MASK 0x8000ULL
#define GPIO_16_MASK 0x10000ULL
#define GPIO_17_MASK 0x20000ULL
#define GPIO_18_MASK 0x40000ULL
#define GPIO_19_MASK 0x80000ULL
#define GPIO_21_MASK 0x200000ULL
#define GPIO_22_MASK 0x400000ULL
#define GPIO_23_MASK 0x800000ULL
#define GPIO_25_MASK 0x2000000ULL
#define GPIO_26_MASK 0x4000000ULL
#define GPIO_27_MASK 0x8000000ULL
#define GPIO_32_MASK 0x10000000ULL
#define GPIO_33_MASK 0x20000000ULL
#define GPIO_34_MASK 0x40000000ULL
#define GPIO_36_MASK 0x100000000ULL
#define GPIO_39_MASK 0x8000000000ULL

uint64_t isr_mask;

#define def_isr(pin)                                       \
    void isr_gpio_##pin() { isr_mask |= GPIO_##pin##_MASK; }

def_isr(4)
def_isr(5)
def_isr(12)
def_isr(13)
def_isr(14)
def_isr(15)
def_isr(16)
def_isr(17)
def_isr(18)
def_isr(19)
def_isr(21)
def_isr(22)
def_isr(23)
def_isr(25)
def_isr(26)
def_isr(27)
def_isr(32)
def_isr(33)
def_isr(34)
def_isr(36)
def_isr(39)

void (*isr_tab[40])() = 
{
    NULL, 			NULL, 			NULL, 			NULL,
    isr_gpio_4, 	isr_gpio_5, 	NULL, 			NULL,
    NULL, 			NULL, 			NULL,			NULL,
    isr_gpio_12, 	isr_gpio_13, 	isr_gpio_14, 	isr_gpio_15,
    isr_gpio_16, 	isr_gpio_17, 	isr_gpio_18, 	isr_gpio_19,
    NULL, 			isr_gpio_21, 	isr_gpio_22, 	isr_gpio_23,
    NULL, 			isr_gpio_25, 	isr_gpio_26, 	isr_gpio_27,
    NULL, 			NULL, 			NULL, 			NULL,
    isr_gpio_32, 	isr_gpio_33, 	isr_gpio_34, 	NULL,
    isr_gpio_36, 	NULL, 			NULL, 			isr_gpio_39
};

#define isr_call(mask, pin)                      \
    if(mask & GPIO_##pin##_MASK)            \
    {                                           \
        ose_pushString(vm_i, "/!/_l/CLR");       \
        ose_pushString(vm_i, "/!/i/"#pin);      \
    }

uint64_t MicrOSCriptESP32::serviceInterrupts(void)
{
    if(isr_mask == 0)
    {
        return 0;
    }
    uint64_t mask;
    noInterrupts();
    mask = isr_mask;
    isr_mask = 0;
    interrupts();

    isr_call(mask, 39);
    isr_call(mask, 36);
    isr_call(mask, 34);
    isr_call(mask, 33);
    isr_call(mask, 32);
    isr_call(mask, 27);
    isr_call(mask, 26);
    isr_call(mask, 25);
    isr_call(mask, 23);
    isr_call(mask, 22);
    isr_call(mask, 21);
    isr_call(mask, 19);
    isr_call(mask, 18);
    isr_call(mask, 17);
    isr_call(mask, 16);
    isr_call(mask, 15);
    isr_call(mask, 14);
    isr_call(mask, 13);
    isr_call(mask, 12);
    isr_call(mask, 5);
    isr_call(mask, 4);

    return mask;
}

void MicrOSCriptESP32::attachInterruptTo(int pin, int direction)
{
    if(pin >= (sizeof(isr_tab) / sizeof(void*)))
    {
        return;
    }
    void (*fn)() = isr_tab[pin];
    if(!fn)
    {
        return;
    }
    attachInterrupt(digitalPinToInterrupt(pin), fn, direction);
}

static void attachInterruptTo(ose_bundle osevm)
{
    ose_bundle vm_s = OSEVM_STACK(osevm);
    ose_bundle vm_x = ose_enter(osevm, "/_x");
    MicrOSCriptESP32 *o =
        (MicrOSCriptESP32 *)ose_readAlignedPtr(vm_x,
                                               microscript_self_offset);
    int32_t pin = ose_popInt32(vm_s);
    int32_t direction = ose_popInt32(vm_s);
    switch(direction)
    {
    case 0:
        direction = LOW;
        break;
    case 1:
        direction = CHANGE;
        break;
    case 2:
        direction = RISING;
        break;
    case 3:
        direction = FALLING;
        break;
    }
    o->attachInterruptTo(pin, direction);
}

/************************************************************
	WiFi
************************************************************/

static void wiFiEventHandler(WiFiEvent_t e)
{
    switch(e)
    {
    case SYSTEM_EVENT_STA_GOT_IP:
    case SYSTEM_EVENT_STA_DISCONNECTED:
        break;
    }
}

void MicrOSCriptESP32::connectToWiFi(const char *ssid,
                                       const char *pass)
{
    WiFi.disconnect(true);
    wifi_connected = false;
    WiFi.onEvent(wiFiEventHandler);
    WiFi.begin(ssid, pass);
    while(WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        ESP_LOGE(TAG, "Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
    }
    wifi_ip_address = WiFi.localIP();
    wifi_connected = true;
    ESP_LOGI(TAG, "Connected! IP address: %s",
             wifi_ip_address.toString().c_str());
}

String MicrOSCriptESP32::wifiIPAddress(void)
{
    return wifi_ip_address.toString();
}

/************************************************************
	UDP
************************************************************/

void MicrOSCriptESP32::udpListen(const unsigned int port)
{
    udp_port = port;
    if(udp.begin(udp_port) > 0)
    {
        ESP_LOGI(TAG, "Listening for UDP packets on port %d",
                 udp_port);
    }
    else
    {
        ESP_LOGE(TAG, "Error opening udp port %d", udp_port);
    }
}

boolean MicrOSCriptESP32::udpRead(void)
{
    int32_t size = udp.parsePacket();
    if(size)
    {
        ESP_LOGI(TAG, "UDP: received %d bytes", size);
        {
            udp.read(udp_buffer,
                     MICROSCRIPT_UDP_BUFFER_SIZE);
            osevm_inputMessages(osevm, size, udp_buffer);
        }
        return true;
    }
    return false;
}

int32_t MicrOSCriptESP32::udpSendElemTo(IPAddress addr,
                                   uint16_t port,
                                   ose_bundle bundle)
{
    int32_t o = ose_getLastBundleElemOffset(bundle);
    int status = udp.beginPacket(addr, port);
    if(!status)
    {
        ESP_LOGE(TAG, "%s: error starting packet", __func__);
        return 0;
    }
    const uint8_t *b =
        (const uint8_t *)ose_getBundlePtr(bundle);
    int32_t n =
        udp.write(b + o + 4, ose_readInt32(bundle, o));
    status = udp.endPacket();
    if(!status)
    {
        ESP_LOGE(TAG, "%s: error sending packet", __func__);
        return 0;
    }
    return n;
}

static void udpSendTo(ose_bundle osevm)
{
    ose_bundle vm_s = OSEVM_STACK(osevm);
    ose_bundle vm_x = ose_enter(osevm, "/_x");
    MicrOSCriptESP32 *o =
        (MicrOSCriptESP32 *)ose_readAlignedPtr(vm_x,
                                               microscript_self_offset);
    ose_unpackDrop(vm_s);
    int32_t port = ose_popInt32(vm_s);
    IPAddress addr;
    addr.fromString(ose_peekString(vm_s));
    ose_drop(vm_s);
    o->udpSendElemTo(addr, (uint16_t)port, vm_s);
}

int32_t MicrOSCriptESP32::udpPort(void)
{
    return udp_port;
}

/************************************************************
	SLIPSerial
 ************************************************************/

int32_t MicrOSCriptESP32::slipSerialRead(void)
{
    return 0;
}
