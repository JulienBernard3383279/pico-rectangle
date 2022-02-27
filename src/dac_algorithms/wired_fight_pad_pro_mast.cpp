#include "dac_algorithms/wired_fight_pad_pro_mast.hpp"


#define coord(x) ((uint8_t)(128. + 80.*x + 0.5))
#define oppositeCoord(x) -((uint8_t)x)

namespace DACAlgorithms {
namespace WiredFightPadProMast {

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

struct Coords {
    uint8_t x;
    uint8_t y;
};

Coords coords(float xFloat, float yFloat) {
    Coords r;
    r.x = coord(xFloat);
    r.y = coord(yFloat);
    return r;
}

// Map MX to -, MY to +, Start to Home, L to ZL, R to ZR, Z to R, LS to L
// MS is a modifier: MS+Start = Photo, MS+Left/Right/Up/Down = Dpad left/right/up/down
void actuateWFPPReport(GpioToButtonSets::Mast::ButtonSet buttonSet) {

    GpioToButtonSets::Mast::ButtonSet bs = buttonSet; // Alterable copy

    /* Stick */

    bool vertical = bs.up || bs.down; // Vertical up priority
    bool readUp = bs.up;

    bool horizontal = bs.left ^ bs.right; // Horizontal neutral
    bool readRight = bs.right;

    Coords xy;

    if (vertical && horizontal) {
        if (bs.l || bs.r) {
            if (bs.mx == bs.my) xy = coords(0.7, 0.7); // Changed to 0.7, 0.7
            else if (bs.mx) xy = coords(0.6375, 0.375); // Untouched
            else xy = coords(0.85, 0.5); // Changed to 0.85, 0.5 (no parasol dashing)
        }
        // Removed extended up b coords
        else if (bs.mx != bs.my) { // All of these changed to match Crane's values
            if (bs.mx) {
                if (bs.cDown) xy = coords(0.7625, 0.6125);
                else if (bs.cLeft) xy = coords(0.825, 0.525);
                else if (bs.cUp) xy = coords(0.8875, 0.4375);
                else if (bs.cRight) xy = coords(0.9, 0.3375);
                else xy = coords(0.5, 0.325);
            }
            else {
                if (bs.cDown) xy = coords(0.6125, 0.7625);
                else if (bs.cLeft) xy = coords(0.525, 0.825);
                else if (bs.cUp) xy = coords(0.4375, 0.8875);
                else if (bs.cRight) xy = coords(0.3375, 0.9375);
                else xy = coords(0.475, 0.6125);
            }
        }
        else xy = coords(0.7,0.7);
    }
    else if (horizontal) {
        if (bs.mx == bs.my) xy = coords(1.0, 0.0);
        else if (bs.mx) xy =  coords(0.5, 0.0); // 0.6625 -> 0.5 + removed ledgedash facilitation
        else xy = coords(0.3375, 0.0); // No slight side B, unchanged besides that
    }
    else if (vertical) {
        if (bs.mx == bs.my) xy = coords(0.0, 1.0);
        else if (bs.mx) xy=coords(0.0, 0.2875); // 0.5375 -> 0.2875 (unlike Crane's code)
        else xy = coords(0.0, 0.6375); // 0.7375 -> 0.6375 <- very weird. To test.
    }
    else {
        xy = coords(0.0, 0.0);
    }

    if (horizontal && !readRight) xy.x = oppositeCoord(xy.x);
    if (vertical && !readUp) xy.y = oppositeCoord(xy.y);

    /* C-Stick */
    
    bool cVertical = bs.cUp != bs.cDown;
    bool cHorizontal = bs.cLeft != bs.cRight;

    Coords cxy;
    if (bs.mx && bs.my) cxy = coords(0.0, 0.0);
    else if (cVertical && cHorizontal) cxy = coords(0.7, 0.7);
    else if (cHorizontal) cxy = bs.mx ? coords(0.8375, readUp ? 0.3125 : -0.3125) : coords(1.0, 0.0);
    else if (cVertical) cxy = coords(0.0, 1.0);
    else cxy = coords(0.0, 0.0);

    if (cHorizontal && bs.cLeft) cxy.x = oppositeCoord(cxy.x);
    if (cVertical && bs.cDown) cxy.y = oppositeCoord(cxy.y);

    bool dLeft = buttonSet.cLeft && buttonSet.mx && buttonSet.my;
    bool dRight = buttonSet.cRight && buttonSet.mx && buttonSet.my;
    bool dUp = buttonSet.cUp && buttonSet.mx && buttonSet.my;
    bool dDown = buttonSet.cDown && buttonSet.mx && buttonSet.my;
    
    uint8_t oppositeYStick = oppositeCoord(xy.y);
    uint8_t oppositeCYStick = oppositeCoord(cxy.y);
    USBConfigurations::WiredFightPadPro::hidReport = {
        .y=buttonSet.y,
        .b=buttonSet.b,
        .a=buttonSet.a,
        .x=buttonSet.x,
        .l=buttonSet.l,
        .r=buttonSet.r,
        .zl=buttonSet.zl,
        .zr=buttonSet.zr,
        .minus=buttonSet.minus,
        .plus=buttonSet.plus,
        .pad=0,
        .home=buttonSet.home,
        .photo=0,
        .pad2=0,
        .hat=hatFromDpadValues(dLeft, dRight, dUp, dDown),
        .xStick=xy.x,
        .yStick=oppositeYStick, // Neutral SOCD + reversed axis
        .cxStick=cxy.x, // Neutral SOCD
        .cyStick=oppositeCYStick, // Neutral SOCD + reversed axis
        .pad3=0
    };
}

}
}