#ifndef INPUTS_HPP
#define INPUTS_HPP

#include "pico/stdlib.h"

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
    bool RectangleInput::* ptrToMember;
};

/* Initializes the B0XX/F1 pin mapping */
void initInputs(const PinMapping *pinMappings, size_t pinMappingsLength);

/* Takes a snapshot of the pins and provide the corresponding RectangleInput */
RectangleInput getRectangleInput();

#endif