#include "usb_configurations/xbox_360.hpp"

namespace USBConfigurations
{
namespace Xbox360
{

const uint16_t descriptor_strings_len = 3;
const char *descriptor_strings[descriptor_strings_len] = {
        "Arte",
        "pico-rectangle - XInput mode",
        "1"
};

ControllerReport xInputReport = defaultControllerReport;

int16_t int16StickFromGccStick(uint8_t gccStick) {
    if (gccStick <= 128-70) {
        return 0x8000;
    }
    else if (gccStick >= 128+70) {
        return 0x7FFF;
    }
    else {
        return (((int)gccStick)-128)*((128 << 8)/70.); // Clamp
    }
}

void actuateReportFromGCState(const GCReport &gcReport) {
    xInputReport.reportId = 0;
    xInputReport.rightStickPress = 0;
    xInputReport.leftStickPress = 0;
    xInputReport.back = 0;
    xInputReport.start = gcReport.start;
    xInputReport.dRight = gcReport.dRight;
    xInputReport.dLeft = gcReport.dLeft;
    xInputReport.dDown = gcReport.dDown;
    xInputReport.dUp = gcReport.dUp;
    xInputReport.zl = 0;
    xInputReport.zr = gcReport.z;
    xInputReport.home = 0;
    xInputReport.pad1 = 0;
    xInputReport.a = gcReport.a;
    xInputReport.b = gcReport.b;
    xInputReport.x = gcReport.x;
    xInputReport.y = gcReport.y;
	xInputReport.leftTrigger = gcReport.l ? 255 : gcReport.analogL >= 140 ? 255 : (uint8_t)(((int)(gcReport.analogL >= 40 ? gcReport.analogL-40 : 0))*255/100);
	xInputReport.rightTrigger = gcReport.r ? 255 : gcReport.analogR >= 140 ? 255 : (uint8_t)(((int)(gcReport.analogR >= 40 ? gcReport.analogR-40 : 0))*255/100);
	xInputReport.leftStickX = int16StickFromGccStick(gcReport.xStick);
	xInputReport.leftStickY = int16StickFromGccStick(gcReport.yStick);
	xInputReport.rightStickX = int16StickFromGccStick(gcReport.cxStick);
	xInputReport.rightStickY = int16StickFromGccStick(gcReport.cyStick);
}

void enterMode(void actuateReportFunc(void)) {

    CommunicationProtocols::USB::Configuration usbConfiguration =
    {
        .configNoFunc = {
            .inEpMaxPacketSize = 32,
            .inEpActualPacketSize = 32, //TODO ?
            .outEpMaxPacketSize = 32,
            .epOutId = 1,
            .descriptorStrings = descriptor_strings,
            .descriptorStringsLen = descriptor_strings_len,
            .hid = false,
            .bcdHID = 0,
            .hidReportDescriptor = nullptr,
            .hidReportDescriptorLen = 0,
            .useWinUSB = false,
            .VID = 0x045E, //Microsoft Corp.
            .PID = 0x028E, //Xbox 360 Controller
            .bcdDevice = 0x100,
            .xinput = true,

            .hidReportPtr = (uint8_t*)&xInputReport
        },
        .reportActuationFunc = actuateReportFunc
    };

    CommunicationProtocols::USB::enterMode(usbConfiguration);
}

}
}