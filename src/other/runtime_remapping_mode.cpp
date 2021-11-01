#include <stdio.h>
#include <stdlib.h>
#include "pico/time.h"
#include <vector>
#include <algorithm>
#include "global.hpp"
#include "persistence/pages/runtime_remapping.hpp"
#include "persistence/functions.hpp"

uint32_t findPressed(std::vector<uint32_t> eligiblePins) {
    /*uint32_t mask = 0;
    for (uint32_t pin : eligiblePins) {
        mask += 1 << pin;
    }

    uint32_t snapshot = sio_hw->gpio_in & mask;

    for (uint32_t pin : eligiblePins) {
        if (snapshot & (1 << pin) == 0) return pin;
    }*/

    for (uint32_t pin : eligiblePins) {
        if (!gpio_get(pin)) return pin;
    }

    return -1;
}

namespace Other {
    void enterRuntimeRemappingMode() {

        std::vector<uint32_t> eligiblePins { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 26, 27};
        std::vector<uint32_t> pinsPressedInOrder {};

        sleep_ms(3000);

        gpio_put(LED_PIN, 1);
        int led = 1;

        for (uint32_t pin : eligiblePins) {
            gpio_init(pin);
            gpio_set_dir(pin, GPIO_IN);
            gpio_pull_up(pin);
        }

        while (pinsPressedInOrder.size() != 20) {
            uint32_t pressedPin = findPressed(eligiblePins);
            if ( pressedPin != -1) {
                eligiblePins.erase(std::remove_if(eligiblePins.begin(), eligiblePins.end(), [pressedPin](int i){return pressedPin==i;}));
                pinsPressedInOrder.push_back(pressedPin);
                led = !led;
                gpio_put(LED_PIN, led);
            }
        }

        gpio_put(LED_PIN, 0);

        Persistence::Pages::RuntimeRemapping runtimeRemappingCheckout = Persistence::clone<Persistence::Pages::RuntimeRemapping>();

        runtimeRemappingCheckout.f1GpioToButtonSetRemapping.configured = 0;

        runtimeRemappingCheckout.f1GpioToButtonSetRemapping.lPin      = pinsPressedInOrder[0];  // L
        runtimeRemappingCheckout.f1GpioToButtonSetRemapping.leftPin   = pinsPressedInOrder[1];  // Left
        runtimeRemappingCheckout.f1GpioToButtonSetRemapping.downPin   = pinsPressedInOrder[2];  // Down
        runtimeRemappingCheckout.f1GpioToButtonSetRemapping.rightPin  = pinsPressedInOrder[3];  // Right
        runtimeRemappingCheckout.f1GpioToButtonSetRemapping.mxPin     = pinsPressedInOrder[4];  // MX
        runtimeRemappingCheckout.f1GpioToButtonSetRemapping.myPin     = pinsPressedInOrder[5];  // MY
        runtimeRemappingCheckout.f1GpioToButtonSetRemapping.startPin  = pinsPressedInOrder[6];  // Start
        runtimeRemappingCheckout.f1GpioToButtonSetRemapping.cLeftPin  = pinsPressedInOrder[7];  // CLeft
        runtimeRemappingCheckout.f1GpioToButtonSetRemapping.cDownPin  = pinsPressedInOrder[8];  // CDown
        runtimeRemappingCheckout.f1GpioToButtonSetRemapping.cUpPin    = pinsPressedInOrder[9];  // CUp
        runtimeRemappingCheckout.f1GpioToButtonSetRemapping.aPin      = pinsPressedInOrder[10]; // A
        runtimeRemappingCheckout.f1GpioToButtonSetRemapping.cRightPin = pinsPressedInOrder[11]; // CRight
        runtimeRemappingCheckout.f1GpioToButtonSetRemapping.rPin      = pinsPressedInOrder[12]; // R
        runtimeRemappingCheckout.f1GpioToButtonSetRemapping.bPin      = pinsPressedInOrder[13]; // B
        runtimeRemappingCheckout.f1GpioToButtonSetRemapping.yPin      = pinsPressedInOrder[14]; // Y
        runtimeRemappingCheckout.f1GpioToButtonSetRemapping.xPin      = pinsPressedInOrder[15]; // X
        runtimeRemappingCheckout.f1GpioToButtonSetRemapping.lsPin     = pinsPressedInOrder[16]; // LS
        runtimeRemappingCheckout.f1GpioToButtonSetRemapping.zPin      = pinsPressedInOrder[17]; // Z
        runtimeRemappingCheckout.f1GpioToButtonSetRemapping.msPin     = pinsPressedInOrder[18]; // MS
        runtimeRemappingCheckout.f1GpioToButtonSetRemapping.upPin     = pinsPressedInOrder[19]; // Up

        Persistence::commit(runtimeRemappingCheckout);

        while (1);
    }
}