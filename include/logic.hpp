#include "pico/stdlib.h"
#include "comms.hpp"

#ifndef LOGIC_H
#define LOGIC_H

struct RectangleInput {
    bool a; bool b; bool x; bool y; bool z;
    bool l; bool r; bool ls; bool ms;
    bool mx; bool my;
    bool start;
    bool left; bool right; bool up; bool down;
    bool cLeft; bool cRight; bool cUp; bool cDown;
};

struct PinMapping {
    uint8_t pin;
    uint8_t offset;
};

void initLogic(const PinMapping *pinMappings, size_t pinMappingsLength);

/* Reads input pins and makes a B0XX/F1 report according to the pin mapping provided during init */
GCReport makeReport();

#endif