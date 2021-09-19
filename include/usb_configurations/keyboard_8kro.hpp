#ifndef USB_CONFIGURATIONS__8KRO_KEYBOARD_HPP
#define USB_CONFIGURATIONS__8KRO_KEYBOARD_HPP

#include "communication_protocols/usb.hpp"

namespace USBConfigurations
{
namespace Keyboard8KRO
{

using KeyId = uint8_t;

KeyId keyIdFromLowerCaseLetter(char letter);

extern KeyId hidReport[8];

void enterMode(void actuateReportFunc(void));

}
}

#endif
