#ifndef USB_CONFIGURATIONS__HID_WITH_TRIGGERS_HPP
#define USB_CONFIGURATIONS__HID_WITH_TRIGGERS_HPP

#include "communication_protocols/usb.hpp"
#include "communication_protocols/joybus/gcReport.hpp"

namespace USBConfigurations
{
namespace HidWithTriggers
{

struct __attribute__((packed)) ControllerReport {
    uint8_t reportId;
    uint8_t a:1; uint8_t b:1; uint8_t x:1; uint8_t y:1; uint8_t start:1; uint8_t padding:3;
    uint8_t dLeft : 1; uint8_t dRight:1; uint8_t dDown:1; uint8_t dUp : 1; uint8_t z:1; uint8_t r:1; uint8_t l:1; uint8_t padding2:1;
    uint16_t xStick;
    uint16_t yStick;
    uint16_t cxStick;
    uint16_t cyStick;
    uint16_t analogL;
    uint16_t analogR;
};

const ControllerReport defaultControllerReport = {
    1, // 1 is report ID
    0, 0, 0, 0, 0,          0,
    0, 0, 0, 0, 0, 0, 0,    0,
    128,
    128,
    128,
    128,
    0,
    0
};

extern ControllerReport hidReport;

void actuateReportFromGCState(const GCReport &);

void enterMode(void actuateReportFunc(void));

}
}

#endif
