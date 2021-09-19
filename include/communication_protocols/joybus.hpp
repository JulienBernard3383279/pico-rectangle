#ifndef COMMUNICATION_PROTOCOLS__JOYBUS_HPP
#define COMMUNICATION_PROTOCOLS__JOYBUS_HPP

#include "pico/stdlib.h"
#include "communication_protocols/joybus/gcReport.hpp"

namespace CommunicationProtocols
{
namespace Joybus
{

/**
 * @short Enters the Joybus communication mode
 * 
 * @param dataPin GPIO number of the console data line pin
 * @param func Function to be called to obtain the GCReport to be sent to the console
 */
void enterMode(int dataPin, GCReport func(void));

}
}

#endif