#include "inputs.hpp"

const PinMapping *pinMappings_;
size_t pinMappingsLength_;

void initInputs(const PinMapping *pinMappings, size_t pinMappingsLength) {
    pinMappings_ = pinMappings;
    pinMappingsLength_ = pinMappingsLength;

    // Inputs init
    for (int pinNo = 0; pinNo < pinMappingsLength; ++pinNo) {
        gpio_init(pinMappings[pinNo].pin);
        gpio_set_dir(pinMappings[pinNo].pin, GPIO_IN);
        gpio_pull_up(pinMappings[pinNo].pin);
    }
}

uint32_t inputSnapshot;

RectangleInput getRectangleInput() {
    RectangleInput ri;

    // Button to controller state translation
    inputSnapshot = sio_hw->gpio_in;

    for (int pinNo = 0; pinNo < pinMappingsLength_; ++pinNo) {
        ri.*pinMappings_[pinNo].ptrToMember = !(inputSnapshot & (1 << (pinMappings_[pinNo].pin)));
    }

    return ri;
}