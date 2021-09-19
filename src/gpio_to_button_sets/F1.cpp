#include "gpio_to_button_sets/F1.hpp"

#include "pico/stdlib.h"

#include "global.hpp"

namespace GpioToButtonSets {
namespace F1 {

struct PinMapping {
    uint8_t pin;
    bool ButtonSet::* ptrToMember;
};

const PinMapping pinMappings[] = {
    #if !USE_UART0
    { 0, &ButtonSet::start },
    #endif
    { 2, &ButtonSet::right },
    { 3, &ButtonSet::down },
    { 4, &ButtonSet::left },
    { 5, &ButtonSet::l },
    { 6, &ButtonSet::mx },
    { 7, &ButtonSet::my },
    { 12, &ButtonSet::cUp },
    { 13, &ButtonSet::cLeft },
    { 14, &ButtonSet::a },
    { 15, &ButtonSet::cDown },
    { 16, &ButtonSet::cRight },
    { 17, &ButtonSet::up },
    { 18, &ButtonSet::ms },
    { 19, &ButtonSet::z },
    { 20, &ButtonSet::ls },
    { 21, &ButtonSet::x },
    { 22, &ButtonSet::y },
    { 26, &ButtonSet::b },
    { 27, &ButtonSet::r }
};

bool init = false;

void initDefaultConversion() {
    for (PinMapping pinMapping : pinMappings) {
        gpio_init(pinMapping.pin);
        gpio_set_dir(pinMapping.pin, GPIO_IN);
        gpio_pull_up(pinMapping.pin);
    }
    init = true;
}

ButtonSet defaultConversion() {
    if (!init) initDefaultConversion();

    ButtonSet f1ButtonSet;

    uint32_t inputSnapshot = sio_hw->gpio_in;

    for (PinMapping pinMapping : pinMappings) {
        f1ButtonSet.*(pinMapping.ptrToMember) = !(inputSnapshot & (1 << (pinMapping.pin)));
    }

    return f1ButtonSet;
}

}
}