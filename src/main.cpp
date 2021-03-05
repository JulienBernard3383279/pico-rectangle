#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "comms.hpp"
#include "logic.hpp"
#include "inputs.hpp"

const uint32_t us = 125;

const uint8_t ledPin = 25;

const uint8_t gcDataPin = 28;

#define NUMBER_OF_INPUTS 20
const PinMapping pinMappings[NUMBER_OF_INPUTS] = {
    { 0, &RectangleInput::start },
    { 2, &RectangleInput::right },
    { 3, &RectangleInput::down },
    { 4, &RectangleInput::left },
    { 5, &RectangleInput::l },
    { 6, &RectangleInput::mx },
    { 7, &RectangleInput::my },
    { 12, &RectangleInput::cUp },
    { 13, &RectangleInput::cLeft },
    { 14, &RectangleInput::a },
    { 15, &RectangleInput::cDown },
    { 16, &RectangleInput::cRight },
    { 17, &RectangleInput::up },
    { 18, &RectangleInput::ms },
    { 19, &RectangleInput::z },
    { 20, &RectangleInput::ls },
    { 21, &RectangleInput::x },
    { 22, &RectangleInput::y },
    { 26, &RectangleInput::b },
    { 27, &RectangleInput::r }
};

int main() {

    // Clock at 125MHz
    set_sys_clock_khz(us*1000, true);

    gpio_init(ledPin);
    gpio_set_dir(ledPin, GPIO_OUT);
    gpio_put(ledPin, 1);

    initInputs(pinMappings, 20);
    initComms(gcDataPin, us);

    GCReport gcReport;
    RectangleInput ri;

    while (1) {
        awaitPoll();
        ri = getRectangleInput();
        gcReport = makeReport(ri);
        respondToPoll(&gcReport);
    }

    return 1;
}