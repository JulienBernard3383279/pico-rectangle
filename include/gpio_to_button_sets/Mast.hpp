#ifndef GPIO_TO_BUTTON_SETS__MAST_HPP
#define GPIO_TO_BUTTON_SETS__MAST_HPP

namespace GpioToButtonSets {
namespace Mast {

struct ButtonSet {
    bool a; bool b; bool x; bool y;
    bool l; bool r; bool zl; bool zr;
    bool mx; bool my;
    bool minus; bool plus; bool home;
    bool left; bool right; bool up; bool down;
    bool cLeft; bool cRight; bool cUp; bool cDown;
};

ButtonSet defaultConversion();

}
}

#endif