#ifndef GPIO_TO_BUTTON_SETS__F1_HPP
#define GPIO_TO_BUTTON_SETS__F1_HPP

namespace GpioToButtonSets {
namespace F1 {

struct ButtonSet {
    bool a; bool b; bool x; bool y; bool z;
    bool l; bool r; bool ls; bool ms;
    bool mx; bool my;
    bool start;
    bool left; bool right; bool up; bool down;
    bool cLeft; bool cRight; bool cUp; bool cDown;
};

ButtonSet defaultConversion();

}
}

#endif