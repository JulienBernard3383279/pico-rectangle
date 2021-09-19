#include "usb_configurations/gcc_to_usb_adapter.hpp"

namespace USBConfigurations
{
namespace GccToUsbAdapter
{

AdapterReport hidReport = defaultAdapterReport;

const uint16_t WUP_028_hid_report_descriptor_len = 214;
uint8_t WUP_028_hid_report_descriptor[WUP_028_hid_report_descriptor_len] =
{
    5,   5,   9,   0,
    161,   1, 133,  17,  25,  0,  42, 255,   0,  21,   0,  38, 255,   0, 117,   8, 149,   5, 145,   0, 192, 
    161,   1, 133,  33,  25,  0,  42, 255,   0,  21,   0,  38, 255,   0, 117,   8, 149,  37, 129,   0, 192, 
    161,   1, 133,  18,  25,  0,  42, 255,   0,  21,   0,  38, 255,   0, 117,   8, 149,   1, 145,   0, 192, 
    161,   1, 133,  34,  25,  0,  42, 255,   0,  21,   0,  38, 255,   0, 117,   8, 149,  25, 129,   0, 192, 
    161,   1, 133,  19,  25,  0,  42, 255,   0,  21,   0,  38, 255,   0, 117,   8, 149,   1, 145,   0, 192, 
    161,   1, 133,  35,  25,  0,  42, 255,   0,  21,   0,  38, 255,   0, 117,   8, 149,   2, 129,   0, 192, 
    161,   1, 133,  20,  25,  0,  42, 255,   0,  21,   0,  38, 255,   0, 117,   8, 149,   1, 145,   0, 192, 
    161,   1, 133,  36,  25,  0,  42, 255,   0,  21,   0,  38, 255,   0, 117,   8, 149,   2, 129,   0, 192, 
    161,   1, 133,  21,  25,  0,  42, 255,   0,  21,   0,  38, 255,   0, 117,   8, 149,   1, 145,   0, 192, 
    161,   1, 133,  37,  25,  0,  42, 255,   0,  21,   0,  38, 255,   0, 117,   8, 149,   2, 129,   0, 192
};

const uint16_t descriptor_strings_len = 3;
const char *descriptor_strings[descriptor_strings_len] = {
        "Nintendo",
        "WUP-028",
        "15/07/2014" // The "release number"
};

void actuateReportFromGCState(GCReport gcReport) {
    hidReport.portReports[0].controllerType = 0x10;
    hidReport.portReports[0].a = gcReport.a;
    hidReport.portReports[0].b = gcReport.b;
    hidReport.portReports[0].x = gcReport.x;
    hidReport.portReports[0].y = gcReport.y;
    hidReport.portReports[0].dLeft = gcReport.dLeft;
    hidReport.portReports[0].dRight = gcReport.dRight;
    hidReport.portReports[0].dUp = gcReport.dUp;
    hidReport.portReports[0].dDown = gcReport.dDown;
    hidReport.portReports[0].l = gcReport.l;
    hidReport.portReports[0].r = gcReport.r;
    hidReport.portReports[0].z = gcReport.z;
    hidReport.portReports[0].start = gcReport.start;
    hidReport.portReports[0].xStick = gcReport.xStick;
    hidReport.portReports[0].yStick = gcReport.yStick;
    hidReport.portReports[0].cxStick = gcReport.cxStick;
    hidReport.portReports[0].cyStick = gcReport.cyStick;
    hidReport.portReports[0].analogL = gcReport.analogL; // It's not a 1:1 translation but it aligns correctly
    hidReport.portReports[0].analogR = gcReport.analogR; // between console & USB
}

void enterMode(void (*actuateReportFunc)(void)) {
    CommunicationProtocols::USB::Configuration AdapterUSBConfiguration =
    {
        .inEpMaxPacketSize = 37,
        .inEpActualPacketSize = 37,
        .outEpMaxPacketSize = 5,
        .epOutId = 2,
        .descriptorStrings = descriptor_strings,
        .descriptorStringsLen = descriptor_strings_len,
        .hid = true,
        .bcdHID = 0x0110,
        .hidReportDescriptor = WUP_028_hid_report_descriptor,
        .hidReportDescriptorLen = WUP_028_hid_report_descriptor_len,
        .useWinUSB = true,
        .VID = 0x057E, // Nintendo VID
        .PID = 0x0337, // WUP-028 PID
        .bcdDevice = 0x102, // Use different bcdDevice as the normal WUP-028, which is 0x0100

        .hidReportPtr = (uint8_t*)&hidReport,
        .reportActuationFunc = actuateReportFunc
    };

    CommunicationProtocols::USB::enterMode(AdapterUSBConfiguration);
}

}
}

/* WUP-028 HID Descriptor - defines opaque input/output reports */
/*
0x05, 0x05,        // Usage Page (Game Ctrls)
0x09, 0x00,        // Usage (Undefined)
0xA1, 0x01,        // Collection (Application)
0x85, 0x11,        //   Report ID (17)
0x19, 0x00,        //   Usage Minimum (Undefined)
0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x75, 0x08,        //   Report Size (8)
0x95, 0x05,        //   Report Count (5)
0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection
0xA1, 0x01,        // Collection (Application)
0x85, 0x21,        //   Report ID (33)
0x19, 0x00,        //   Usage Minimum (Undefined)
0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x75, 0x08,        //   Report Size (8)
0x95, 0x25,        //   Report Count (37)
0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection
0xA1, 0x01,        // Collection (Application)
0x85, 0x12,        //   Report ID (18)
0x19, 0x00,        //   Usage Minimum (Undefined)
0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection
0xA1, 0x01,        // Collection (Application)
0x85, 0x22,        //   Report ID (34)
0x19, 0x00,        //   Usage Minimum (Undefined)
0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x75, 0x08,        //   Report Size (8)
0x95, 0x19,        //   Report Count (25)
0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection
0xA1, 0x01,        // Collection (Application)
0x85, 0x13,        //   Report ID (19)
0x19, 0x00,        //   Usage Minimum (Undefined)
0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection
0xA1, 0x01,        // Collection (Application)
0x85, 0x23,        //   Report ID (35)
0x19, 0x00,        //   Usage Minimum (Undefined)
0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x75, 0x08,        //   Report Size (8)
0x95, 0x02,        //   Report Count (2)
0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection
0xA1, 0x01,        // Collection (Application)
0x85, 0x14,        //   Report ID (20)
0x19, 0x00,        //   Usage Minimum (Undefined)
0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection
0xA1, 0x01,        // Collection (Application)
0x85, 0x24,        //   Report ID (36)
0x19, 0x00,        //   Usage Minimum (Undefined)
0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x75, 0x08,        //   Report Size (8)
0x95, 0x02,        //   Report Count (2)
0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection
0xA1, 0x01,        // Collection (Application)
0x85, 0x15,        //   Report ID (21)
0x19, 0x00,        //   Usage Minimum (Undefined)
0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection
0xA1, 0x01,        // Collection (Application)
0x85, 0x25,        //   Report ID (37)
0x19, 0x00,        //   Usage Minimum (Undefined)
0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x75, 0x08,        //   Report Size (8)
0x95, 0x02,        //   Report Count (2)
0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection
*/