#include "dac_algorithms/ultimate_F1.hpp"
#include "communication_protocols/joybus/gcReport.hpp"

namespace DACAlgorithms {
namespace UltimateF1 {

#define coord(x) ((uint8_t)(128. + 80.*x + 0.5))
#define oppositeCoord(x) -((uint8_t)x)

// 2 IP declarations
bool left_wasPressed = false;
bool right_wasPressed = false;
bool up_wasPressed = false;
bool down_wasPressed = false;

bool left_outlawUntilRelease = false;
bool right_outlawUntilRelease = false;
bool up_outlawUntilRelease = false;
bool down_outlawUntilRelease = false;

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


GCReport getGCReport(GpioToButtonSets::F1::ButtonSet buttonSet) {
    
    GpioToButtonSets::F1::ButtonSet bs = buttonSet; // Alterable copy

    GCReport gcReport = defaultGcReport;

    /* 2IP No reactivation */
    
    if (left_wasPressed && bs.left && bs.right && !right_wasPressed) left_outlawUntilRelease=true;
    if (right_wasPressed && bs.left && bs.right && !left_wasPressed) right_outlawUntilRelease=true;
    if (up_wasPressed && bs.up && bs.down && !down_wasPressed) up_outlawUntilRelease=true;
    if (down_wasPressed && bs.up && bs.down && !up_wasPressed) down_outlawUntilRelease=true;

    if (!bs.left) left_outlawUntilRelease=false;
    if (!bs.right) right_outlawUntilRelease=false;
    if (!bs.up) up_outlawUntilRelease=false;
    if (!bs.down) down_outlawUntilRelease=false;

    left_wasPressed = bs.left;
    right_wasPressed = bs.right;
    up_wasPressed = bs.up;
    down_wasPressed = bs.down;

    if (left_outlawUntilRelease) bs.left=false;
    if (right_outlawUntilRelease) bs.right=false;
    if (up_outlawUntilRelease) bs.up=false;
    if (down_outlawUntilRelease) bs.down=false;
    
    /* Stick */

    bool vertical = bs.up || bs.down;
    bool readUp = bs.up;

    bool horizontal = bs.left || bs.right;
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

    gcReport.xStick = xy.x;
    gcReport.yStick = xy.y;

    /* C-Stick */
    
    bool cVertical = bs.cUp != bs.cDown;
    bool cHorizontal = bs.cLeft != bs.cRight;

    Coords cxy;
    if (bs.mx && bs.my) cxy = coords(0.0, 0.0);
    else if (cVertical && cHorizontal) cxy = coords(0.7, 0.7);
    else if (cHorizontal) cxy = bs.mx ? coords(0.7, readUp ? 0.7 : -0.7) : coords(1.0, 0.0);
    else if (cVertical) cxy = coords(0.0, 1.0);
    else cxy = coords(0.0, 0.0);

    if (cHorizontal && bs.cLeft) cxy.x = oppositeCoord(cxy.x);
    if (cVertical && bs.cDown) cxy.y = oppositeCoord(cxy.y);

    gcReport.cxStick = cxy.x;
    gcReport.cyStick = cxy.y;

    /* Dpad */
    if (bs.mx && bs.my) {
        gcReport.dDown = bs.cDown;
        gcReport.dLeft = bs.cLeft;
        gcReport.dUp = bs.cUp;
        gcReport.dRight = bs.cRight;
    }

    /* Triggers */
    gcReport.analogL = bs.l ? 140 : bs.ms ? 94 : bs.ls ? 49 : 0;
    gcReport.analogR = bs.r ? 140 : 0;

    /* Buttons */
    gcReport.a = bs.a;
    gcReport.b = bs.b;
    gcReport.x = bs.x;
    gcReport.y = bs.y;
    gcReport.z = bs.z;
    gcReport.l = bs.l;
    gcReport.r = bs.r;
    gcReport.start = bs.start;

    return gcReport;
}

}
}