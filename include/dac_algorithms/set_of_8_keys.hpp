#ifndef DAC_ALGORITHMS__SET_OF_8_KEYS_HPP
#define DAC_ALGORITHMS__SET_OF_8_KEYS_HPP

#include "usb_configurations/keyboard_8kro.hpp"

#include "gpio_to_button_sets/F1.hpp"

namespace DACAlgorithms {
namespace SetOf8Keys {

void actuate8KeysReport(GpioToButtonSets::F1::ButtonSet buttonSet);

}
}

#endif