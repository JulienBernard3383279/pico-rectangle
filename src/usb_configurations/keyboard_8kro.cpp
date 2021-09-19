#include "usb_configurations/keyboard_8kro.hpp"

namespace USBConfigurations
{
namespace Keyboard8KRO
{

KeyId keyIdFromLowerCaseLetter(char letter) {
    return letter+4-'a';
}

KeyId hidReport[8] = {0, 0, 0, 0, 0, 0, 0, 0};

const uint16_t keyboard_hid_report_descriptor_len = 24;
uint8_t keyboard_hid_report_descriptor[keyboard_hid_report_descriptor_len] =
{
       0x05, 0x01, // Usage page: generic desktop
       0x09, 0x06, // Usage: keyboard
       0xA1, 0x01, // Collection: application
       0x95, 0x08, // Report count: 8 (8KRO)
       0x75, 0x08, // Report size: 8 (byte)
       0x15, 0x00, // Logical minimum: 0
       0x26, 0x97, 0x00, // Logical maximum: 0x97
       0x05, 0x07, // Usage page: keyboard
       0x19, 0x00, // Usage minimum: undefined
       0x29, 0x97, // Usage maximum: keyboard LANG8 -> 0x97 apparently
       0x81, 0x00, // Input (data, Ary, Abs)
       0xc0 // End collection
};

const uint16_t descriptor_strings_len = 3;
const char *descriptor_strings[descriptor_strings_len] = {
        "Arte",
        "8KRO HID Keyboard",
        "1"
};

void enterMode(void (*actuateReportFunc)(void)) {
    CommunicationProtocols::USB::Configuration Keyboard8KROUSBConfiguration =
    {
        .inEpMaxPacketSize = 64,
        .inEpActualPacketSize = 8,
        .outEpMaxPacketSize = 64,
        .epOutId = 1,
        .descriptorStrings = descriptor_strings,
        .descriptorStringsLen = descriptor_strings_len,
        .hid = true,
        .bcdHID = 0x0111,
        .hidReportDescriptor = keyboard_hid_report_descriptor,
        .hidReportDescriptorLen = keyboard_hid_report_descriptor_len,
        .useWinUSB = false,
        .VID = 0x121D, // ("H"1D) TODO Pick a sensible VID - any HID VID/PID will do
        .PID = 0x1111,
        .bcdDevice = 0x100,

        .hidReportPtr = hidReport,
        .reportActuationFunc = actuateReportFunc
    };

    CommunicationProtocols::USB::enterMode(Keyboard8KROUSBConfiguration);
}

}
}