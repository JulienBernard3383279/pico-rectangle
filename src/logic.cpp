#include "logic.hpp"

#define coord(x) ((uint8_t)(128. + 80.*x + 0.5))
#define oppositeCoord(x) -((uint8_t)x)

bool banParasolDashing;
bool banSlightSideB;

void initLogic(ParasolDashing parasolDashingConfig, SlightSideB slightSideBConfig) {
    banParasolDashing = parasolDashingConfig == ParasolDashing::BAN;
    banSlightSideB = slightSideBConfig == SlightSideB::BAN;
}

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

GCReport gcReport;
RectangleInput ri;
GCReport makeReport(const RectangleInput &rectangleInput) {

    ri = rectangleInput; // local alterable copy
    gcReport = defaultReport;

    /* 2IP No reactivation */
    
    if (left_wasPressed && ri.left && ri.right && !right_wasPressed) left_outlawUntilRelease=true;
    if (right_wasPressed && ri.left && ri.right && !left_wasPressed) right_outlawUntilRelease=true;
    if (up_wasPressed && ri.up && ri.down && !down_wasPressed) up_outlawUntilRelease=true;
    if (down_wasPressed && ri.up && ri.down && !up_wasPressed) down_outlawUntilRelease=true;

    if (!ri.left) left_outlawUntilRelease=false;
    if (!ri.right) right_outlawUntilRelease=false;
    if (!ri.up) up_outlawUntilRelease=false;
    if (!ri.down) down_outlawUntilRelease=false;

    left_wasPressed = ri.left;
    right_wasPressed = ri.right;
    up_wasPressed = ri.up;
    down_wasPressed = ri.down;

    if (left_outlawUntilRelease) ri.left=false;
    if (right_outlawUntilRelease) ri.right=false;
    if (up_outlawUntilRelease) ri.up=false;
    if (down_outlawUntilRelease) ri.down=false;
    
    /* Stick */

    bool vertical = ri.up || ri.down;
    bool readUp = ri.up;

    bool horizontal = ri.left || ri.right;
    bool readRight = ri.right;

    Coords xy;

    if (vertical && horizontal) {
        if (ri.l || ri.r) {
            if (ri.mx == ri.my) xy = coords(0.7, readUp ? 0.7 : 0.6875);
            else if (ri.mx) xy = coords(0.6375, 0.375);
            else xy = (banParasolDashing && readUp) ? coords(0.875, 0.475) : coords(0.85, 0.5);
        }
        else if (ri.b && (ri.mx != ri.my)) {
            if (ri.mx) {
                if (ri.cDown) xy = coords(0.9125, 0.45);
                else if (ri.cLeft) xy = coords(0.85, 0.525);
                else if (ri.cUp) xy = coords(0.7375, 0.5375);
                else if (ri.cRight) xy = coords(0.6375, 0.5375);
                else xy = coords(0.9125, 0.3875);
            }
            else {
                if (ri.cDown) xy = coords(0.45, 0.875);
                else if (ri.cLeft) xy = coords(0.525, 0.85);
                else if (ri.cUp) xy = coords(0.5875, 0.8);
                else if (ri.cRight) xy = coords(0.5875, 0.7125);
                else xy = coords(0.3875, 0.9125);
            }
        }
        else if (ri.mx != ri.my) {
            if (ri.mx) {
                if (ri.cDown) xy = coords(0.7, 0.3625);
                else if (ri.cLeft) xy = coords(0.7875, 0.4875);
                else if (ri.cUp) xy = coords(0.7, 0.5125);
                else if (ri.cRight) xy = coords(0.6125, 0.525);
                else xy = coords(0.7375, 0.3125);
            }
            else {
                if (ri.cDown) xy = coords(0.3625, 0.7);
                else if (ri.cLeft) xy = coords(0.4875, 0.7875);
                else if (ri.cUp) xy = coords(0.5125, 0.7);
                else if (ri.cRight) xy = coords(0.6375, 0.7625);
                else xy = coords(0.3125, 0.7375);
            }
        }
        else xy = coords(0.7,0.7);
    }
    else if (horizontal) {
        if (ri.mx == ri.my) xy = coords(1.0, 0.0);
        else if (ri.mx) xy =  (ri.left && ri.right) ? coords(1.0, 0.0) : coords(0.6625, 0.0);
        else xy = ((banSlightSideB && ri.b) || ri.left && ri.right) ? coords(1.0, 0.0) : coords(0.3375, 0.0);
    }
    else if (vertical) {
        if (ri.mx == ri.my) xy = coords(0.0, 1.0);
        else if (ri.mx) xy=coords(0.0, 0.5375);
        else xy = coords(0.0, 0.7375);
    }
    else {
        xy = coords(0.0, 0.0);
    }

    if (horizontal && !readRight) xy.x = oppositeCoord(xy.x);
    if (vertical && !readUp) xy.y = oppositeCoord(xy.y);

    gcReport.xStick = xy.x;
    gcReport.yStick = xy.y;

    /* C-Stick */
    
    bool cVertical = ri.cUp != ri.cDown;
    bool cHorizontal = ri.cLeft != ri.cRight;

    Coords cxy;

    if (ri.mx && ri.my) cxy = coords(0.0, 0.0);
    else if (cVertical && cHorizontal) cxy = coords(0.525, 0.85);
    else if (cHorizontal) cxy = ri.mx ? coords(0.8375, readUp ? 0.3125 : -0.3125) : coords(1.0, 0.0);
    else if (cVertical) cxy = coords(0.0, 1.0);
    else cxy = coords(0.0, 0.0);

    if (cHorizontal && ri.cLeft) cxy.x = oppositeCoord(cxy.x);
    if (cVertical && ri.cDown) cxy.y = oppositeCoord(cxy.y);

    gcReport.cxStick = cxy.x;
    gcReport.cyStick = cxy.y;

    /* Dpad */
    if (ri.mx && ri.my) {
        gcReport.dDown = ri.cDown;
        gcReport.dLeft = ri.cLeft;
        gcReport.dUp = ri.cUp;
        gcReport.dRight = ri.cRight;
    }

    /* Triggers */
    gcReport.analogL = ri.l ? 140 : ri.ms ? 94 : ri.ls ? 49 : 0;
    gcReport.analogR = ri.r ? 140 : 0;

    /* Buttons */
    gcReport.a = ri.a;
    gcReport.b = ri.b;
    gcReport.x = ri.x;
    gcReport.y = ri.y;
    gcReport.z = ri.z;
    gcReport.l = ri.l;
    gcReport.r = ri.r;
    gcReport.start = ri.start;

    return gcReport;
}