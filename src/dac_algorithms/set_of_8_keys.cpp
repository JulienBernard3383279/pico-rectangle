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


// If you want to configure a key that isn't a lowercase letter of the alphabet, refer to
// https://gist.github.com/MightyPork/6da26e382a7ad91b5496ee55fdc73db2 and replace the whole
// "USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter('q')" by the hex code that maps
// to your key. For exemple, for enter on L: { &F1::ButtonSet::l,      0x28  }

const KeyMapping keyMappings[] = {
    { &F1::ButtonSet::l,      USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter('q') },
    { &F1::ButtonSet::left,   USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter('w') },
    { &F1::ButtonSet::down,   USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter('e') },
    { &F1::ButtonSet::right,  USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter('r') },
    { &F1::ButtonSet::mx,     USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter('a') },
    { &F1::ButtonSet::my,     USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter('s') },
    { &F1::ButtonSet::start,  USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter('t') },
    { &F1::ButtonSet::cDown,  USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter('c') },
    { &F1::ButtonSet::cLeft,  USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter('f') },
    { &F1::ButtonSet::cUp,    USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter('g') },
    { &F1::ButtonSet::a,      USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter('v') },
    { &F1::ButtonSet::cRight, USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter('b') },
    { &F1::ButtonSet::r,      USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter('y') },
    { &F1::ButtonSet::b,      USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter('h') },
    { &F1::ButtonSet::y,      USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter('u') },
    { &F1::ButtonSet::x,      USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter('j') },
    { &F1::ButtonSet::ls,     USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter('i') },
    { &F1::ButtonSet::z,      USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter('k') },
    { &F1::ButtonSet::ms,     USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter('o') },
    { &F1::ButtonSet::up,     USBConfigurations::Keyboard8KRO::keyIdFromLowerCaseLetter('l') }
};

void appendKey(char key, int& index) {
    if (index==8) return;
    USBConfigurations::Keyboard8KRO::hidReport[index] = key;
    index++;
}

void actuate8KeysReport(F1::ButtonSet buttonSet) {
    log_uart0("actuateKeys\n");
    for (int i = 0; i<8; i++) USBConfigurations::Keyboard8KRO::hidReport[i] = 0;
    int index=0;
    for (KeyMapping keyMapping : keyMappings) {
        if (buttonSet.*(keyMapping.ptrToMember)) {
            log_uart0_int(keyMapping.key); log_uart0(" ");
            appendKey(keyMapping.key, index);
        }
    }
    if (index>0) log_uart0("\n");
}

}
}