#ifndef DAC_ALGORITHMS__PROJECT_PLUS_F1_HPP
#define DAC_ALGORITHMS__PROJECT_PLUS_F1_HPP

#include "communication_protocols/joybus/gcReport.hpp"

#include "gpio_to_button_sets/F1.hpp"

namespace DACAlgorithms {
namespace ProjectPlusF1 {

GCReport getGCReport(GpioToButtonSets::F1::ButtonSet buttonSet);

}
}

#endif