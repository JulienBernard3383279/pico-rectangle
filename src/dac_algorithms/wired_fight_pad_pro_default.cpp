#include "dac_algorithms/wired_fight_pad_pro_default.hpp"

namespace DACAlgorithms {
namespace WiredFightPadProDefault {

//TODO Default 2ip probably better

// Note: since the 1.0 coordinates on HID are defined by the highest value you reach, setting to +-80 should be fine

uint8_t hatFromDpadValues(bool dLeft, bool dRight, bool dUp, bool dDown) {
    if (dUp && !dLeft && !dRight) return 0;
    else if (dUp && dRight) return 1;
    else if (dRight && !dDown) return 2;
    else if (dRight && dDown) return 3;
    else if (dDown && !dLeft) return 4;
    else if (dDown && dLeft) return 5;
    else if (dLeft && !dUp) return 6;
    else if (dLeft && dUp) return 7;
    else return 15;
}

// Map MX to -, MY to +, Start to Home, L to ZL, R to ZR, Z to R, LS to L
// MS is a modifier: MS+Start = Photo, MS+Left/Right/Up/Down = Dpad left/right/up/down
void actuateWFPPReport(GpioToButtonSets::F1::ButtonSet buttonSet) {

    bool left = buttonSet.left && !(buttonSet.ms);
    bool right = buttonSet.right && !(buttonSet.ms);
    bool up = buttonSet.up && !(buttonSet.ms);
    bool down = buttonSet.down && !(buttonSet.ms);

    bool dLeft = buttonSet.left && buttonSet.ms;
    bool dRight = buttonSet.right && buttonSet.ms;
    bool dUp = buttonSet.up && buttonSet.ms;
    bool dDown = buttonSet.down && buttonSet.ms;
    
    USBConfigurations::WiredFightPadPro::hidReport = {
        .y=buttonSet.y,
        .b=buttonSet.b,
        .a=buttonSet.a,
        .x=buttonSet.x,
        .l=buttonSet.ls, // LS -> L
        .r=buttonSet.z, // Z -> R
        .zl=buttonSet.l, // L -> ZL
        .zr=buttonSet.r, // R -> ZR
        .minus=buttonSet.mx, // MX -> -
        .plus=buttonSet.my, // MY -> +
        .pad=0,
        .home=buttonSet.start && !(buttonSet.ms), // Start -> Home
        .photo=buttonSet.ms && buttonSet.start, // MS -> Photo
        .pad2=0,
        .hat=hatFromDpadValues(dLeft, dRight, dUp, dDown),
        .xStick=(uint8_t)(!(left ^ right) ? 128 : left ? 48 : 208), // Neutral SOCD
        .yStick=(uint8_t)(!(up ^ down) ? 128 : up ? 48 : 208), // Neutral SOCD + reversed axis
        .cxStick=(uint8_t)(!(buttonSet.cLeft ^ buttonSet.cRight) ? 128 : buttonSet.cLeft ? 48 : 208), // Neutral SOCD
        .cyStick=(uint8_t)(!(buttonSet.cUp ^ buttonSet.cDown) ? 128 : buttonSet.cUp ? 48 : 208), // Neutral SOCD + reversed axis
        .pad3=0
    };
}

}
}