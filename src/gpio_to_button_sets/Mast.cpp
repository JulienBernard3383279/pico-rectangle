#include "gpio_to_button_sets/Mast.hpp"

#include "pico/stdlib.h"

#include "persistence/pages/runtime_remapping.hpp"
#include "persistence/functions.hpp"

#include "global.hpp"

namespace GpioToButtonSets {
namespace Mast {

struct PinMapping {
    uint8_t pin;
    bool ButtonSet::* ptrToMember;
};

const PinMapping pinMappings[] = {
    { 0, &ButtonSet::plus },
    { 2, &ButtonSet::right },
    { 3, &ButtonSet::down },
    { 4, &ButtonSet::left },
    { 5, &ButtonSet::l },
    { 6, &ButtonSet::mx },
    { 7, &ButtonSet::my },
    { 8, &ButtonSet::minus },
    { 9, &ButtonSet::home },
    { 12, &ButtonSet::cUp },
    { 13, &ButtonSet::cLeft },
    { 14, &ButtonSet::a },
    { 15, &ButtonSet::cDown },
    { 16, &ButtonSet::cRight },
    { 17, &ButtonSet::up },
    { 19, &ButtonSet::zr },
    { 20, &ButtonSet::zl },
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
    
    ButtonSet mastButtonSet;

    uint32_t inputSnapshot = sio_hw->gpio_in;

    for (PinMapping pinMapping : pinMappings) {
        mastButtonSet.*(pinMapping.ptrToMember) = !(inputSnapshot & (1 << (pinMapping.pin)));
    }

    return mastButtonSet;
}

}
}