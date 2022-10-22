#ifndef USB__XINPUT_HPP
#define USB__XINPUT_HPP

#include "pico/stdlib.h"
#include <array>

const std::array<uint8_t, 0x11> xInputUnknownDescriptor = {
    0x11, // Length of descriptor
    0x21, // Descriptor id
    0x10, 0x01, 0x01, 0x25, // Unknown
    0x81, // EP IN
    0x14, // Length of IN report
    0x03, 0x03, 0x03, 0x04, 0x13, // Unknown
    0x01, // EP OUT
    0x08, // Length of rumble command on this EP
    0x03, 0x03 // Unknown
};

// bmRequestType = 0xc1, bmRequest = 1, wValue = 0x0100, wIndex = 0x0000, wLength = 20
const size_t xInputSpecificControlRequestResponse1Len = 20;
const uint8_t xInputSpecificControlRequestResponse1[xInputSpecificControlRequestResponse1Len] =
    { 0x00, 0x14, 0xff, 0xf7, 0xff, 0xff, 0xc0, 0xff, 0xc0, 0xff, 0xc0, 0xff, 0xc0, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
// ^ description du report ? Report Id (?), len 20, 8 boutons utilisés, 7 boutons utilisÃ©s, 2 triggers jusqu'à  0xff, 4 axes sur 16 bits signés, 6 octets vides

// bmRequestType = 0xc1, bmRequest = 1, wValue = 0x0000, wIndex = 0x0000, wLength = 8
const size_t xInputSpecificControlRequestResponse2Len = 8;
const uint8_t xInputSpecificControlRequestResponse2[xInputSpecificControlRequestResponse2Len] = { 0x00, 0x08, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00 };

// bmRequestType = 0xc0, bmRequest = 1, wValue = 0x0000, wIndex = 0x0000, wLength = 4
const size_t xInputSpecificControlRequestResponse3Len = 4;
const uint8_t xInputSpecificControlRequestResponse3[xInputSpecificControlRequestResponse3Len] = { 0x0f, 0xd5, 0xe6, 0xff };

#endif