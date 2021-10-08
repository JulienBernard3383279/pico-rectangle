#ifndef DAC_ALGORITHMS__NASB_AUSTIN_HPP
#define DAC_ALGORITHMS__NASB_AUSTIN_HPP

#include "communication_protocols/joybus/gcReport.hpp"

#include "gpio_to_button_sets/NASB_Austin.hpp"

namespace DACAlgorithms {
namespace NASB_Austin {

GCReport getGCReport(GpioToButtonSets::NASB_Austin::ButtonSet buttonSet);

}
}

#endif