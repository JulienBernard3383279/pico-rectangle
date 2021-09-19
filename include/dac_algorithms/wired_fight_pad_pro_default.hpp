#ifndef DAC_ALGORITHMS__WIRED_FIGHT_PAD_PRO_DEFAULT_HPP
#define DAC_ALGORITHMS__WIRED_FIGHT_PAD_PRO_DEFAULT_HPP

#include "usb_configurations/wired_fight_pad_pro.hpp"

#include "gpio_to_button_sets/F1.hpp"

namespace DACAlgorithms {
namespace WiredFightPadProDefault {

void actuateWFPPReport(GpioToButtonSets::F1::ButtonSet buttonSet);

}
}
#endif