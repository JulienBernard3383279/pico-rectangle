#include "dac_algorithms/set_of_8_keys.hpp"

#include "hardware/gpio.h"
#include "global.hpp"
#include "pico/stdlib.h"

namespace DACAlgorithms {
namespace SetOf8Keys {

void log_uart0(const char* str) {
    #if USE_UART0
    uart_puts(uart0, str);
    #endif
}
void log_uart0_int(int i) {
    #if USE_UART0
    char str[16];
    sprintf(str, "%d", i);
    uart_puts(uart0, str);
    #endif
}

using namespace GpioToButtonSets;

struct KeyMapping {
    bool F1::ButtonSet::* ptrToMember;
    char key;
};

const KeyMapping keyMappings[] = {
    { &F1::ButtonSet::l, 'q' },
    { &F1::ButtonSet::left, 'w' },
    { &F1::ButtonSet::down, 'e' },
    { &F1::ButtonSet::right, 'r' },
    { &F1::ButtonSet::mx, 'a' },
    { &F1::ButtonSet::my, 's' },
    { &F1::ButtonSet::start, 't' },
    { &F1::ButtonSet::cDown, 'c' },
    { &F1::ButtonSet::cLeft, 'f' },
    { &F1::ButtonSet::cUp, 'g' },
    { &F1::ButtonSet::a, 'v' },
    { &F1::ButtonSet::cRight, 'b' },
    { &F1::ButtonSet::r, 'y' },
    { &F1::ButtonSet::b, 'h' },
    { &F1::ButtonSet::y, 'u' },
    { &F1::ButtonSet::x, 'j' },
    { &F1::ButtonSet::ls, 'i' },
    { &F1::ButtonSet::z, 'k' },
    { &F1::ButtonSet::ms, 'o' },
    { &F1::ButtonSet::up, 'l' }
};

void appendKey(char key, int& index) {
    if (index==8) return;
    USBConfigurations::Keyboard8KRO::hidReport[index] = USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter(key);
    index++;
}

void actuate8KeysReport(F1::ButtonSet buttonSet) {
    log_uart0("actuateKeys\n");
    for (int i = 0; i<8; i++) USBConfigurations::Keyboard8KRO::hidReport[i] = 0;
    int index=0;
    for (KeyMapping keyMapping : keyMappings) {
        if (buttonSet.*(keyMapping.ptrToMember)) {
            gpio_put(25, 1);
            log_uart0_int(USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter(keyMapping.key)); log_uart0(" ");
            appendKey(keyMapping.key, index);
        }
    }
    if (index>0) log_uart0("\n");
}

}
}