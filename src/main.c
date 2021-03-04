#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "comms.h"
#include "logic.h"

static const uint32_t us = 125;

static const uint8_t ledPin = 25;

static const uint8_t gcDataPin = 28;

#define NUMBER_OF_INPUTS 20
const struct PinMapping pinMappings[NUMBER_OF_INPUTS] = {
    { 0, offsetof(struct RectangleInput, start) },
    { 2, offsetof(struct RectangleInput, right) },
    { 3, offsetof(struct RectangleInput, down) },
    { 4, offsetof(struct RectangleInput, left) },
    { 5, offsetof(struct RectangleInput, l) },
    { 6, offsetof(struct RectangleInput, mx) },
    { 7, offsetof(struct RectangleInput, my) },
    { 12, offsetof(struct RectangleInput, cUp) },
    { 13, offsetof(struct RectangleInput, cLeft) },
    { 14, offsetof(struct RectangleInput, a) },
    { 15, offsetof(struct RectangleInput, cDown) },
    { 16, offsetof(struct RectangleInput, cRight) },
    { 17, offsetof(struct RectangleInput, up) },
    { 18, offsetof(struct RectangleInput, ms) },
    { 19, offsetof(struct RectangleInput, z) },
    { 20, offsetof(struct RectangleInput, ls) },
    { 21, offsetof(struct RectangleInput, x) },
    { 22, offsetof(struct RectangleInput, y) },
    { 26, offsetof(struct RectangleInput, b) },
    { 27, offsetof(struct RectangleInput, r) }
};

int main() {

    // Clock at 125MHz
    set_sys_clock_khz(us*1000, true);

    gpio_init(ledPin);
    gpio_set_dir(ledPin, GPIO_OUT);
    gpio_put(ledPin, 1);

    initLogic(pinMappings, 20);
    initComms(gcDataPin, us);

    struct GCReport gcReport;
    
    while (1) {
        awaitPoll();
        gcReport = makeReport();
        respondToPoll(&gcReport);
    }

    return 1;
}