#include "dac_algorithms/NASB_Austin.hpp"
#include "communication_protocols/joybus.hpp"

namespace DACAlgorithms {
namespace NASB_Austin {

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


GCReport getGCReport(GpioToButtonSets::NASB_Austin::ButtonSet buttonSet) {
    
    GpioToButtonSets::NASB_Austin::ButtonSet bs = buttonSet; // Alterable copy

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
        xy = coords(0.7,0.7);
    }
    else if (horizontal) {
        xy = coords(1.0, 0.0);
    }
    else if (vertical) {
        xy = coords(0.0, 1.0);
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

    if (cVertical && cHorizontal) cxy = coords(0.7, 0.7);
    else if (cHorizontal) cxy = coords(1.0, 0.0);
    else if (cVertical) cxy = coords(0.0, 1.0);
    else cxy = coords(0.0, 0.0);

    if (cHorizontal && bs.cLeft) cxy.x = oppositeCoord(cxy.x);
    if (cVertical && bs.cDown) cxy.y = oppositeCoord(cxy.y);

    gcReport.cxStick = cxy.x;
    gcReport.cyStick = cxy.y;

    /* No triggers */

    /* Buttons */
    gcReport.a = bs.a;
    gcReport.b = bs.b;
    gcReport.x = bs.x;
    gcReport.y = bs.y1 || bs.y2;
    gcReport.z = bs.z;
    gcReport.l = bs.l;
    gcReport.r = bs.r1 || bs.r2;
    gcReport.start = bs.start;

    return gcReport;
}

}
}