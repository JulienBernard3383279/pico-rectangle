#ifndef USB_CONFIGURATIONS__GCC_TO_USB_ADAPTER_HPP
#define USB_CONFIGURATIONS__GCC_TO_USB_ADAPTER_HPP

#include "communication_protocols/usb.hpp"
#include "communication_protocols/joybus/gcReport.hpp"

namespace USBConfigurations
{
namespace GccToUsbAdapter
{

struct __attribute__((packed)) AdapterPortReport {
    uint8_t controllerType;
    uint8_t a:1; uint8_t b:1; uint8_t x:1; uint8_t y:1; uint8_t dLeft:1; uint8_t dRight:1; uint8_t dDown:1; uint8_t dUp:1;
    uint8_t start : 1; uint8_t z:1; uint8_t r:1; uint8_t l : 1; uint8_t padding:4;
    uint8_t xStick;
    uint8_t yStick;
    uint8_t cxStick;
    uint8_t cyStick;
    uint8_t analogL;
    uint8_t analogR;
};

const AdapterPortReport defaultAdapterPortReport = {
    0x00, // 0x10 = connected
    0, 0, 0, 0, 0, 0, 0, 0,
    0,          0, 0, 0, 0,
    128,
    128,
    128,
    128,
    128,
    128
};

struct __attribute__((packed)) AdapterReport {
    uint8_t fixed;
    AdapterPortReport portReports[4];
};

const AdapterReport defaultAdapterReport = {
    33,
    { defaultAdapterPortReport, defaultAdapterPortReport, defaultAdapterPortReport, defaultAdapterPortReport }
};

extern AdapterReport hidReport;

void actuateReportFromGCState(GCReport);

void enterMode(void actuateReportFunc(void));

}
}

#endif
