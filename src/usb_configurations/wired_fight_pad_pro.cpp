#include "usb_configurations/wired_fight_pad_pro.hpp"

namespace USBConfigurations
{
namespace WiredFightPadPro
{

WFPPReport hidReport = defaultWFPPReport;

const uint16_t WFPP_hid_report_descriptor_len = 80;
uint8_t WFPP_hid_report_descriptor[WFPP_hid_report_descriptor_len] =
{
    5, 1, 9, 5, 161, 1, 21, 0, 37, 1, 53, 0, 69, 1, 117, 1, 149, 14, 5, 9, 25, 1, 41, 14, 129, 2, 149, 2, 129, 1, 5, 1, 37, 7, 70, 59, 1, 117, 4, 149, 1, 101, 20, 9, 57, 129, 66, 101, 0, 149, 1, 129, 1,
    38, 255, 0,    
    70, 255, 0,
    9, 48, 9, 49, 9, 50, 9, 53, 117, 8, 149, 4, 129, 2, 117, 8, 149, 1, 129, 1, 192
};

const uint16_t descriptor_strings_len = 3;
const char *descriptor_strings[descriptor_strings_len] = {
        "Performance Designed Products",
        "Wired Fight Pad Pro for Nintendo Switch",
        "00002014603B7353" // Release number
};

uint8_t hatFromDpad(const GCReport &gcReport) {
    if (gcReport.dUp && !gcReport.dLeft && !gcReport.dRight) return 0;
    else if (gcReport.dUp && gcReport.dRight) return 1;
    else if (gcReport.dRight && !gcReport.dDown) return 2;
    else if (gcReport.dRight && gcReport.dDown) return 3;
    else if (gcReport.dDown && !gcReport.dLeft) return 4;
    else if (gcReport.dDown && gcReport.dLeft) return 5;
    else if (gcReport.dLeft && !gcReport.dUp) return 6;
    else if (gcReport.dLeft && gcReport.dUp) return 7;
    else return 15;
}

void actuateReportFromGCState(GCReport gcReport) {
    hidReport = defaultWFPPReport;
    hidReport.y = gcReport.y;
    hidReport.b = gcReport.b;
    hidReport.a = gcReport.a;
    hidReport.x = gcReport.x;
    hidReport.r = gcReport.z;
    hidReport.zl = gcReport.l;
    hidReport.zr = gcReport.r;
    hidReport.home = gcReport.start;
    hidReport.hat = hatFromDpad(gcReport);
    hidReport.xStick = gcReport.xStick;
    hidReport.yStick = -gcReport.yStick;
    hidReport.cxStick = gcReport.cxStick;
    hidReport.cyStick = -gcReport.cyStick;
}

void enterMode(void (*actuateReportFunc)(void)) {
    CommunicationProtocols::USB::Configuration WFPPUSBConfiguration =
    {
        .inEpMaxPacketSize = 64,
        .inEpActualPacketSize = 8,
        .outEpMaxPacketSize = 64,
        .epOutId = 1,
        .descriptorStrings = descriptor_strings,
        .descriptorStringsLen = descriptor_strings_len,
        .hid = true,
        .bcdHID = 0x0100,
        .hidReportDescriptor = WFPP_hid_report_descriptor,
        .hidReportDescriptorLen = WFPP_hid_report_descriptor_len,
        .useWinUSB = false,
        .VID = 0x0E6F,
        .PID = 0x0185,
        .bcdDevice = 0x1077, // Use different bcdDevice as the normal one (0x0077)

        .hidReportPtr = (uint8_t*)&hidReport,
        .reportActuationFunc = actuateReportFunc
    };

    CommunicationProtocols::USB::enterMode(WFPPUSBConfiguration);
}

}
}

/*
	0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
	0x09, 0x05,        // Usage (Game Pad)
	0xA1, 0x01,        // Collection (Application)
	0x15, 0x00,        //   Logical Minimum (0)
	0x25, 0x01,        //   Logical Maximum (1)
	0x35, 0x00,        //   Physical Minimum (0)
	0x45, 0x01,        //   Physical Maximum (1)
	0x75, 0x01,        //   Report Size (1)
	0x95, 0x0E,        //   Report Count (14)
	0x05, 0x09,        //   Usage Page (Button)
	0x19, 0x01,        //   Usage Minimum (0x01)
	0x29, 0x0E,        //   Usage Maximum (0x0E)
	0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x95, 0x02,        //   Report Count (2)
	0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
	0x25, 0x07,        //   Logical Maximum (7)
	0x46, 0x3B, 0x01,  //   Physical Maximum (315)
	0x75, 0x04,        //   Report Size (4)
	0x95, 0x01,        //   Report Count (1)
	0x65, 0x14,        //   Unit (System: English Rotation, Length: Centimeter)
	0x09, 0x39,        //   Usage (Hat switch)
	0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
	0x65, 0x00,        //   Unit (None)
	0x95, 0x01,        //   Report Count (1)
	0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x26, 0xFF, 0x00,  //   Logical Maximum (255)
	0x46, 0xFF, 0x00,  //   Physical Maximum (255)
	0x09, 0x30,        //   Usage (X)
	0x09, 0x31,        //   Usage (Y)
	0x09, 0x32,        //   Usage (Z)
	0x09, 0x35,        //   Usage (Rz)
	0x75, 0x08,        //   Report Size (8)
	0x95, 0x04,        //   Report Count (4)
	0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x75, 0x08,        //   Report Size (8)
	0x95, 0x01,        //   Report Count (1)
	0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0xC0,              // End Collection

	// 80 bytes
	*/