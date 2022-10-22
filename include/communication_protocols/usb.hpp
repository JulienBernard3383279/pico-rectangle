#ifndef COMMUNICATION_PROTOCOLS__USB_HPP
#define COMMUNICATION_PROTOCOLS__USB_HPP

#include "usb/common.hpp"

namespace CommunicationProtocols
{
namespace USB
{

struct ConfigurationNoFunc {
    uint16_t inEpMaxPacketSize;
    uint16_t inEpActualPacketSize;
    uint16_t outEpMaxPacketSize;
    uint8_t epOutId; // 1 or 2
    const char **descriptorStrings;
    uint16_t descriptorStringsLen;
    bool hid;
    uint16_t bcdHID;
    uint8_t* hidReportDescriptor;
    uint16_t hidReportDescriptorLen;
    bool useWinUSB;
    uint16_t VID;
    uint16_t PID;
    uint16_t bcdDevice;

    uint8_t* hidReportPtr;
};

struct Configuration {
    ConfigurationNoFunc configNoFunc;
    void (*reportActuationFunc)(void);
};

struct FuncsDOP {
    void (*reportActuationFuncPC)(void);
    void (*reportActuationFuncSwitch)(void);
};

void enterMode(Configuration, int headroomUs = 120);
void enterMode(ConfigurationNoFunc, FuncsDOP, int headroomUs = 120);

}
}

#endif