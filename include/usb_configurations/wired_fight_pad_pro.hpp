#ifndef USB_CONFIGURATIONS__WIRED_FIGHT_PAD_PRO_HPP
#define USB_CONFIGURATIONS__WIRED_FIGHT_PAD_PRO_HPP

#include "communication_protocols/usb.hpp"
#include "communication_protocols/joybus/gcReport.hpp"

namespace USBConfigurations
{
namespace WiredFightPadPro
{

struct __attribute__((packed)) WFPPReport {
    uint8_t y:1; uint8_t b:1; uint8_t a:1; uint8_t x:1; uint8_t l:1; uint8_t r:1; uint8_t zl:1; uint8_t zr:1;
    uint8_t minus:1; uint8_t plus:1; uint8_t pad: 2; uint8_t home:1; uint8_t photo:1; uint8_t pad2:2;
    uint8_t hat;
    uint8_t xStick;
    uint8_t yStick;
    uint8_t cxStick;
    uint8_t cyStick;
    uint8_t pad3;
};

const WFPPReport defaultWFPPReport = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0,    0, 0,    0, 0, 0,
    15,
    0x80,
    0x80,
    0x80,
    0x80,
    0
};

extern WFPPReport hidReport;

void actuateReportFromGCState(GCReport);

void enterMode(void actuateReportFunc(void));

}
}

#endif
