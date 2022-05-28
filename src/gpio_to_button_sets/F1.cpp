#include "gpio_to_button_sets/F1.hpp"

#include "pico/stdlib.h"

#include "persistence/pages/runtime_remapping.hpp"
#include "persistence/functions.hpp"

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
    { 19, &ButtonSet::z },
    { 20, &ButtonSet::ls },
    { 21, &ButtonSet::x },
    { 22, &ButtonSet::y },
    { 26, &ButtonSet::b },
    { 27, &ButtonSet::r }
};

bool remapped;

const PinMapping remappedPinMappings[] = {
    { Persistence::read<Persistence::Pages::RuntimeRemapping>()->f1GpioToButtonSetRemapping.startPin, &ButtonSet::start },
    { Persistence::read<Persistence::Pages::RuntimeRemapping>()->f1GpioToButtonSetRemapping.rightPin, &ButtonSet::right },
    { Persistence::read<Persistence::Pages::RuntimeRemapping>()->f1GpioToButtonSetRemapping.downPin, &ButtonSet::down },
    { Persistence::read<Persistence::Pages::RuntimeRemapping>()->f1GpioToButtonSetRemapping.leftPin, &ButtonSet::left },
    { Persistence::read<Persistence::Pages::RuntimeRemapping>()->f1GpioToButtonSetRemapping.lPin, &ButtonSet::l },
    { Persistence::read<Persistence::Pages::RuntimeRemapping>()->f1GpioToButtonSetRemapping.mxPin, &ButtonSet::mx },
    { Persistence::read<Persistence::Pages::RuntimeRemapping>()->f1GpioToButtonSetRemapping.myPin, &ButtonSet::my },
    { Persistence::read<Persistence::Pages::RuntimeRemapping>()->f1GpioToButtonSetRemapping.cUpPin, &ButtonSet::cUp },
    { Persistence::read<Persistence::Pages::RuntimeRemapping>()->f1GpioToButtonSetRemapping.cLeftPin, &ButtonSet::cLeft },
    { Persistence::read<Persistence::Pages::RuntimeRemapping>()->f1GpioToButtonSetRemapping.aPin, &ButtonSet::a },
    { Persistence::read<Persistence::Pages::RuntimeRemapping>()->f1GpioToButtonSetRemapping.cDownPin, &ButtonSet::cDown },
    { Persistence::read<Persistence::Pages::RuntimeRemapping>()->f1GpioToButtonSetRemapping.cRightPin, &ButtonSet::cRight },
    { Persistence::read<Persistence::Pages::RuntimeRemapping>()->f1GpioToButtonSetRemapping.upPin, &ButtonSet::up },
    { Persistence::read<Persistence::Pages::RuntimeRemapping>()->f1GpioToButtonSetRemapping.zPin, &ButtonSet::z },
    { Persistence::read<Persistence::Pages::RuntimeRemapping>()->f1GpioToButtonSetRemapping.lsPin, &ButtonSet::ls },
    { Persistence::read<Persistence::Pages::RuntimeRemapping>()->f1GpioToButtonSetRemapping.xPin, &ButtonSet::x },
    { Persistence::read<Persistence::Pages::RuntimeRemapping>()->f1GpioToButtonSetRemapping.yPin, &ButtonSet::y },
    { Persistence::read<Persistence::Pages::RuntimeRemapping>()->f1GpioToButtonSetRemapping.bPin, &ButtonSet::b },
    { Persistence::read<Persistence::Pages::RuntimeRemapping>()->f1GpioToButtonSetRemapping.rPin, &ButtonSet::r }
};

bool init = false;

void initDefaultConversion() {
    remapped = Persistence::isnt0xFF(Persistence::read<Persistence::Pages::RuntimeRemapping>()->f1GpioToButtonSetRemapping.configured);
    for (PinMapping pinMapping : remapped ? remappedPinMappings : pinMappings) {
        gpio_init(pinMapping.pin);
        gpio_set_dir(pinMapping.pin, GPIO_IN);
        gpio_pull_up(pinMapping.pin);
    }
    init = true;
}

ButtonSet defaultConversion() {

    if (!init) initDefaultConversion();

    if (remapped) gpio_put(LED_PIN, 1);
    
    ButtonSet f1ButtonSet;

    uint32_t inputSnapshot = sio_hw->gpio_in;

    for (PinMapping pinMapping : remapped ? remappedPinMappings : pinMappings) {
        f1ButtonSet.*(pinMapping.ptrToMember) = !(inputSnapshot & (1 << (pinMapping.pin)));
    }

    return f1ButtonSet;
}

}
}