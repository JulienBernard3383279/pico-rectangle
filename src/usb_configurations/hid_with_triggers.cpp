#include "usb_configurations/hid_with_triggers.hpp"

namespace USBConfigurations
{
namespace HidWithTriggers
{

uint8_t hidReportDescriptor[] = {
0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
0x09, 0x04,        // Usage (Joystick)
0xA1, 0x01,        // Collection (Application)
0x85, 0x01,        //   Report ID (1)
0x09, 0x01,        //   Usage (Pointer)
0xA1, 0x00,        //   Collection (Physical)
0x05, 0x09,        //     Usage Page (Button)
0x19, 0x01,        //     Usage Minimum (0x01)
0x29, 0x10,        //     Usage Maximum (0x10)
0x15, 0x00,        //     Logical Minimum (0)
0x25, 0x01,        //     Logical Maximum (1)
0x75, 0x01,        //     Report Size (1)
0x95, 0x10,        //     Report Count (16)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
0x09, 0x30,        //     Usage (X) - X
0x09, 0x31,        //     Usage (Y) - Y
0x09, 0x32,        //     Usage (Z) - CX
0x09, 0x33,        //     Usage (Rx) - CY
0x09, 0x36,        //     Usage (Slider)
0x09, 0x36,        //     Usage (Slider)
0x16, 0x00, 0x80,       //     Logical Minimum (0)
0x26, 0xFF, 0x7F,  //     Logical Maximum (2^16-1)
0x75, 0x10,        //     Report Size (16)
0x95, 0x06,        //     Report Count (6)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              //   End Collection
0xC0,              // End Collection
};
const uint16_t hidReportDescriptorLen = sizeof(hidReportDescriptor);

const uint16_t descriptor_strings_len = 3;
const char *descriptor_strings[descriptor_strings_len] = {
        "Arte",
        "pico-rectangle - HID with triggers mode",
        "1"
};

ControllerReport hidReport = defaultControllerReport;
void actuateReportFromGCState(const GCReport &gcReport) {
    hidReport.reportId = 1;
    hidReport.a = gcReport.a;
    hidReport.b = gcReport.b;
    hidReport.x = gcReport.x;
    hidReport.y = gcReport.y;
    hidReport.dLeft = gcReport.dLeft;
    hidReport.dRight = gcReport.dRight;
    hidReport.dUp = gcReport.dUp;
    hidReport.dDown = gcReport.dDown;
    hidReport.l = gcReport.l;
    hidReport.r = gcReport.r;
    hidReport.z = gcReport.z;
    hidReport.start = gcReport.start;
    hidReport.xStick = ((gcReport.xStick - 128) << 8) + 127;
    hidReport.yStick = ((gcReport.yStick - 128) << 8) + 127;
    hidReport.cxStick = ((gcReport.cxStick - 128) << 8) + 127;
    hidReport.cyStick = ((gcReport.cyStick - 128) << 8) + 127;
    hidReport.analogL = ((gcReport.analogL - 128) << 8) + 128; // Why ?
    hidReport.analogR = ((gcReport.analogR - 128) << 8) + 128; // That's a good question
}

void enterMode(void actuateReportFunc(void)) {

    CommunicationProtocols::USB::Configuration usbConfiguration =
    {
        .inEpMaxPacketSize = 64,
        .inEpActualPacketSize = 15,
        .outEpMaxPacketSize = 64,
        .epOutId = 2,
        .descriptorStrings = descriptor_strings,
        .descriptorStringsLen = descriptor_strings_len,
        .hid = true,
        .bcdHID = 0x0110,
        .hidReportDescriptor = hidReportDescriptor,
        .hidReportDescriptorLen = hidReportDescriptorLen,
        .useWinUSB = false,
        .VID = 0x121D,
        .PID = 0x1112,
        .bcdDevice = 0x100,

        .hidReportPtr = (uint8_t*)&hidReport,
        .reportActuationFunc = actuateReportFunc
    };

    CommunicationProtocols::USB::enterMode(usbConfiguration);
}

}
}