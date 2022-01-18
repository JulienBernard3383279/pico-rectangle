#ifndef DAC_ALGORITHMS__MELEE_F1_HPP
#define DAC_ALGORITHMS__MELEE_F1_HPP

#include "communication_protocols/joybus/gcReport.hpp"

#include "gpio_to_button_sets/F1.hpp"

namespace DACAlgorithms {
namespace MeleeF1 {

extern bool banParasolDashing;
extern bool banSlightSideB;

GCReport getGCReport(GpioToButtonSets::F1::ButtonSet buttonSet);

// Emulated travel time: in order to have it happen independently of GC polls, we'll
// await GPIO changes and construct the state every change in a dedicated core

// On single-core MCUs this could also be done by deferring GC comms to UART or
// using interrupts

namespace EmulatedTravelTime {
    void initialize(GpioToButtonSets::F1::ButtonSet(*func)(void));
    GCReport getGCReport();
}

}
}

#endif