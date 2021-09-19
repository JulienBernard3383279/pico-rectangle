#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include <array>

#include "global.hpp"

#include "dac_algorithms/melee_F1.hpp"
#include "dac_algorithms/set_of_8_keys.hpp"
#include "dac_algorithms/wired_fight_pad_pro_default.hpp"

#include "gpio_to_button_sets/F1.hpp"

#include "usb_configurations/gcc_to_usb_adapter.hpp"
#include "usb_configurations/keyboard_8kro.hpp"
#include "usb_configurations/wired_fight_pad_pro.hpp"

#include "communication_protocols/joybus.hpp"

#define LED_PIN 25
#define USB_POWER_PIN 24

const uint8_t gcDataPin = 28;

int main() {

    set_sys_clock_khz(1000*us, true);
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    gpio_init(USB_POWER_PIN);
    gpio_set_dir(USB_POWER_PIN, GPIO_IN);

    #ifdef USE_UART0
    // Initialise UART 0
    uart_init(uart0, 115200);
 
    // Set the GPIO pin mux to the UART - 0 is TX, 1 is RX
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);
    #endif

    const uint8_t keyboardPin = 
    #ifdef USE_UART0
    5
    #else
    0
    #endif
    ;

    std::array<uint8_t, 3> modePins = { 7, 6, keyboardPin }; // DO NOT USE PIN GP15

    for (uint8_t modePin : modePins) {
        gpio_init(modePin);
        gpio_set_dir(modePin, GPIO_IN);
        gpio_pull_up(modePin);
    }

    /* Mode selection logic */

    // Not plugged through USB =>  F1 / melee / joybus
    if (!gpio_get(USB_POWER_PIN)) CommunicationProtocols::Joybus::enterMode(gcDataPin, [](){
        return DACAlgorithms::MeleeF1::getGCReport(GpioToButtonSets::F1::defaultConversion());
    });

    // Else:
    // 7 (L): F1 / melee / wired_fight_pad_pro
    if (!gpio_get(7)) USBConfigurations::WiredFightPadPro::enterMode([](){
        USBConfigurations::WiredFightPadPro::actuateReportFromGCState(DACAlgorithms::MeleeF1::getGCReport(GpioToButtonSets::F1::defaultConversion()));
    });

    // 6 (Left): F1 / wired_fight_pad_pro_default / wired_fight_pad_pro
    if (!gpio_get(6)) USBConfigurations::WiredFightPadPro::enterMode([](){
        DACAlgorithms::WiredFightPadProDefault::actuateWFPPReport(GpioToButtonSets::F1::defaultConversion());
    });

    // 0 (Start): F1 / 8 keys set / 8KRO keyboard
    if (!gpio_get(keyboardPin)) USBConfigurations::Keyboard8KRO::enterMode([](){
        DACAlgorithms::SetOf8Keys::actuate8KeysReport(GpioToButtonSets::F1::defaultConversion());
    });

    // Default: F1 / melee / adapter
    USBConfigurations::GccToUsbAdapter::enterMode([](){
        USBConfigurations::GccToUsbAdapter::actuateReportFromGCState(DACAlgorithms::MeleeF1::getGCReport(GpioToButtonSets::F1::defaultConversion()));
    });
}