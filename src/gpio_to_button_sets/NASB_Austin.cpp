#include "gpio_to_button_sets/NASB_Austin.hpp"

#include "pico/stdlib.h"

#include "global.hpp"

namespace GpioToButtonSets {
namespace NASB_Austin {

struct PinMapping {
    uint8_t pin;
    bool ButtonSet::* ptrToMember;
};

/* Replace the numbers here */
const PinMapping pinMappings[] = {
    { 0, &ButtonSet::start },
    { 1, &ButtonSet::left },
    { 2, &ButtonSet::right },
    { 3, &ButtonSet::down },
    { 4, &ButtonSet::up },
    { 5, &ButtonSet::l },
    { 6, &ButtonSet::r1 },
    { 7, &ButtonSet::r2 },
    { 8, &ButtonSet::a },
    { 9, &ButtonSet::b },
    { 10, &ButtonSet::x },
    { 11, &ButtonSet::y1 },
    { 12, &ButtonSet::y2 },
    { 13, &ButtonSet::cLeft },
    { 14, &ButtonSet::cRight },
    { 15, &ButtonSet::cDown },
    { 16, &ButtonSet::cUp },
    { 17, &ButtonSet::start }
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

    ButtonSet nasbButtonSet;

    uint32_t inputSnapshot = sio_hw->gpio_in;

    for (PinMapping pinMapping : pinMappings) {
        nasbButtonSet.*(pinMapping.ptrToMember) = !(inputSnapshot & (1 << (pinMapping.pin)));
    }

    return nasbButtonSet;
}

}
}