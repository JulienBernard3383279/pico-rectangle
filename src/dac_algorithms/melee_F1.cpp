#include "dac_algorithms/melee_F1.hpp"
#include "dac_algorithms/analog_values/melee_values.hpp"
#include "communication_protocols/joybus.hpp"

namespace DACAlgorithms {
namespace MeleeF1 {

#define coord(x) ((uint8_t)(NEUTRAL_OFFSET + MAX_OFFSET*x + 0.5))
#define oppositeCoord(x) -((uint8_t)x)

bool banParasolDashing = false;
bool banSlightSideB = false;

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

    if (vertical && horizontal) {                                                           // if diagonal input
        if (bs.l || bs.r) {                                                                     // if L or R are held (for shield drop / wavedash)
            if (bs.mx == bs.my) xy = coords(DIAG_SHIELD_X, readUp ? DIAG_SHIELD_Y_UP : DIAG_SHIELD_Y_DN);                        // if MX and MY are BOTH released OR held (default)
            else if (bs.mx) xy = coords(MODX_D_SHIELD_X, MODX_D_SHIELD_Y);                                         // else if only MX is held
            else xy = (banParasolDashing && readUp) ? coords(MODY_D_SHIELD_NOPARADASH_X, MODY_D_SHIELD_NOPARADASH_Y) : coords(MODY_D_SHIELD_X, MODY_D_SHIELD_Y); // else if only MY is held
        }
        else if (bs.b && (bs.mx != bs.my)) {                                                // else if B AND exactly one of MX/MY is held
            if (bs.mx) {                                                                        // if MX is held
                if (bs.cDown) xy = coords(MODX_D_CD_B_X, MODX_D_CD_B_Y);                                            // account for c-stick inputs
                else if (bs.cLeft) xy = coords(MODX_D_CL_B_X, MODX_D_CL_B_Y);                                        
                else if (bs.cUp) xy = coords(MODX_D_CU_B_X, MODX_D_CU_B_Y);                               
                else if (bs.cRight) xy = coords(MODX_D_CR_B_X, MODX_D_CR_B_Y);
                else xy = coords(MODX_DIAG_B_X, MODX_DIAG_B_Y);
            }
            else {                                                                              // if MY is held
                if (bs.cDown) xy = coords(MODY_D_CD_B_X, MODY_D_CD_B_Y);                                             // account for c-stick inputs
                else if (bs.cLeft) xy = coords(MODY_D_CL_B_X, MODY_D_CL_B_Y);
                else if (bs.cUp) xy = coords(MODY_D_CU_B_X, MODY_D_CU_B_Y);
                else if (bs.cRight) xy = coords(MODY_D_CR_B_X, MODY_D_CR_B_Y);
                else xy = coords(MODY_DIAG_B_X, MODY_DIAG_B_Y);
            }
        }
        else if (bs.mx != bs.my) {                                                          //else if exactly one of MX/MY is held (B is not held)
            if (bs.mx) {                                                                        // if MX is held
                if (bs.cDown) xy = coords(MODX_D_CD_X, MODX_D_CD_Y);                                             // account for c-stick inputs
                else if (bs.cLeft) xy = coords(MODX_D_CL_X, MODX_D_CL_Y);
                else if (bs.cUp) xy = coords(MODX_D_CU_X, MODX_D_CU_Y);
                else if (bs.cRight) xy = coords(MODX_D_CR_X, MODX_D_CR_Y);
                else xy = coords(MODX_DIAG_X, MODX_DIAG_Y);
            }
            else {                                                                              // if MY is held
                if (bs.cDown) xy = coords(MODY_D_CD_X, MODY_D_CD_Y);                                             // account for c-stick inputs
                else if (bs.cLeft) xy = coords(MODY_D_CL_X, MODY_D_CL_Y);
                else if (bs.cUp) xy = coords(MODY_D_CU_X, MODY_D_CU_Y);
                else if (bs.cRight) xy = coords(MODY_D_CR_X, MODY_D_CR_Y);
                else xy = coords(MODY_DIAG_X, MODY_DIAG_Y);
            }
        }
        else xy = coords(DIAGONAL_X, DIAGONAL_Y);                                                              // diagonal inputs with no modifiers
    }
    else if (horizontal) {
        if (bs.mx == bs.my) xy = coords(HORIZONTAL_X, HORIZONTAL_Y);
        else if (bs.mx) xy =  (buttonSet.left && buttonSet.right) ? coords(HORIZONTAL_X, HORIZONTAL_Y) : coords(MODX_H_X, MODX_H_Y);
        else xy = ((banSlightSideB && bs.b) || buttonSet.left && buttonSet.right) ? coords(HORIZONTAL_X, HORIZONTAL_Y) : coords(MODY_H_X, MODY_H_Y);
        // Read the original rectangleInput to bypass SOCD
    }
    else if (vertical) {
        if (bs.mx == bs.my) xy = coords(VERTICAL_X, VERTICAL_Y);
        else if (bs.mx) xy=coords(MODX_V_X, MODX_V_Y);
        else xy = coords(MODY_V_X, MODY_V_Y);
    }
    else {
        xy = coords(NEUTRAL_X, NEUTRAL_Y);
    }

    if (horizontal && !readRight) xy.x = oppositeCoord(xy.x);
    if (vertical && !readUp) xy.y = oppositeCoord(xy.y);

    gcReport.xStick = xy.x;
    gcReport.yStick = xy.y;

    /* C-Stick */
    
    bool cVertical = bs.cUp != bs.cDown;
    bool cHorizontal = bs.cLeft != bs.cRight;

    Coords cxy;

    if (bs.mx && bs.my) cxy = coords(C_NEUTRAL_X, C_NEUTRAL_Y);
    else if (cVertical && cHorizontal) cxy = coords(C_DIAGONAL_X, C_DIAGONAL_Y);
    else if (cHorizontal) cxy = bs.mx ? coords(C_MODX_FSMASH_X, readUp ? C_MODX_FSMASH_Y : -C_MODX_FSMASH_Y) : coords(C_HORIZONTAL_X, C_HORIZONTAL_Y);
    else if (cVertical) cxy = coords(C_VERTICAL_X, C_VERTICAL_Y);
    else cxy = coords(C_NEUTRAL_X, C_NEUTRAL_Y);

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
    gcReport.analogL = bs.l ? SHIELD_FULL : bs.ms ? SHIELD_MID : bs.ls ? SHIELD_LIGHT : SHIELD_NONE;
    gcReport.analogR = bs.r ? SHIELD_FULL : SHIELD_NONE;

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