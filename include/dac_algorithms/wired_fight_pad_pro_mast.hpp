#ifndef DAC_ALGORITHMS__WIRED_FIGHT_PAD_PRO_MAST_HPP
#define DAC_ALGORITHMS__WIRED_FIGHT_PAD_PRO_MAST_HPP

#include "usb_configurations/wired_fight_pad_pro.hpp"

#include "gpio_to_button_sets/Mast.hpp"

namespace DACAlgorithms {
namespace WiredFightPadProMast {

void actuateWFPPReport(GpioToButtonSets::Mast::ButtonSet buttonSet);

}
}
#endif