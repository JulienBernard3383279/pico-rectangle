#ifndef USB_CONFIGURATIONS__XBOX_360_HPP
#define USB_CONFIGURATIONS__XBOX_360_HPP

#include "communication_protocols/usb.hpp"
#include "communication_protocols/joybus.hpp"
#include <array>

namespace USBConfigurations
{
namespace Xbox360
{

struct __attribute__((packed)) ControllerReport {
    uint8_t reportId;
	uint8_t reportSize;
    uint8_t dUp : 1; uint8_t dDown:1; uint8_t dLeft:1; uint8_t dRight : 1; uint8_t start : 1; uint8_t back:1; uint8_t leftStickPress:1; uint8_t rightStickPress:1;
    uint8_t zl:1; uint8_t zr:1; uint8_t home:1; uint8_t pad1:1; uint8_t a:1; uint8_t b:1; uint8_t x:1; uint8_t y:1;
	uint8_t leftTrigger;
	uint8_t rightTrigger;
	int16_t leftStickX;
	int16_t leftStickY;
	int16_t rightStickX;
	int16_t rightStickY;
	std::array<uint8_t, 6> reserved;
};

static_assert(sizeof(ControllerReport)==20);

const ControllerReport defaultControllerReport = {
    0,
    20,

    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    0,
    0,

    0,
    0,
    0,
    0,

    {0, 0, 0, 0, 0, 0}
};

extern ControllerReport xInputReport;

void actuateReportFromGCState(const GCReport &);

void enterMode(void actuateReportFunc(void));

}
}

#endif
