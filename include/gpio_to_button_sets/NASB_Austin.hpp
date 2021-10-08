#ifndef GPIO_TO_BUTTON_SETS__NASB_AUSTIN_HPP
#define GPIO_TO_BUTTON_SETS__NASB_AUSTIN_HPP

namespace GpioToButtonSets {
namespace NASB_Austin {

struct ButtonSet {
    bool a; bool b; bool x; bool y1; bool y2; bool z;
    bool l; bool r1; bool r2;
    bool start;
    bool left; bool right; bool up; bool down;
    bool cLeft; bool cRight; bool cUp; bool cDown;
};

ButtonSet defaultConversion();

}
}

#endif