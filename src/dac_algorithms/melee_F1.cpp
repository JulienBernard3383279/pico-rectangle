#include "dac_algorithms/melee_F1.hpp"
#include "communication_protocols/joybus.hpp"

#include "hardware/timer.h"
#include "pico/multicore.h"
#include "string.h"

#include <math.h>

namespace DACAlgorithms {
namespace MeleeF1 {

#define coord(x) ((uint8_t)(128. + 80.*x + 0.5))
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

    if (vertical && horizontal) {
        if (bs.l || bs.r) {
            if (bs.mx == bs.my) xy = coords(0.7, readUp ? 0.7 : 0.6875);
            else if (bs.mx) xy = coords(0.6375, 0.375);
            else xy = (banParasolDashing && readUp) ? coords(0.475, 0.875) : coords(0.5, 0.85);
        }
        else if (bs.b && (bs.mx != bs.my)) {
            if (bs.mx) {
                if (bs.cDown) xy = coords(0.9125, 0.45);
                else if (bs.cLeft) xy = coords(0.85, 0.525);
                else if (bs.cUp) xy = coords(0.7375, 0.5375);
                else if (bs.cRight) xy = coords(0.6375, 0.5375);
                else xy = coords(0.9125, 0.3875);
            }
            else {
                if (bs.cDown) xy = coords(0.45, 0.875);
                else if (bs.cLeft) xy = coords(0.525, 0.85);
                else if (bs.cUp) xy = coords(0.5875, 0.8);
                else if (bs.cRight) xy = coords(0.5875, 0.7125);
                else xy = coords(0.3875, 0.9125);
            }
        }
        else if (bs.mx != bs.my) {
            if (bs.mx) {
                if (bs.cDown) xy = coords(0.7, 0.3625);
                else if (bs.cLeft) xy = coords(0.7875, 0.4875);
                else if (bs.cUp) xy = coords(0.7, 0.5125);
                else if (bs.cRight) xy = coords(0.6125, 0.525);
                else xy = coords(0.7375, 0.3125);
            }
            else {
                if (bs.cDown) xy = coords(0.3625, 0.7);
                else if (bs.cLeft) xy = coords(0.4875, 0.7875);
                else if (bs.cUp) xy = coords(0.5125, 0.7);
                else if (bs.cRight) xy = coords(0.6375, 0.7625);
                else xy = coords(0.3125, 0.7375);
            }
        }
        else xy = coords(0.7,0.7);
    }
    else if (horizontal) {
        if (bs.mx == bs.my) xy = coords(1.0, 0.0);
        else if (bs.mx) xy =  (buttonSet.left && buttonSet.right) ? coords(1.0, 0.0) : coords(0.6625, 0.0);
        else xy = ((banSlightSideB && bs.b) || buttonSet.left && buttonSet.right) ? coords(1.0, 0.0) : coords(0.3375, 0.0);
        // Read the original rectangleInput to bypass SOCD
    }
    else if (vertical) {
        if (bs.mx == bs.my) xy = coords(0.0, 1.0);
        else if (bs.mx) xy=coords(0.0, 0.5375);
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
    
    bool cVertical = bs.cUp != bs.cDown;
    bool cHorizontal = bs.cLeft != bs.cRight;

    Coords cxy;

    if (bs.mx && bs.my) cxy = coords(0.0, 0.0);
    else if (cVertical && cHorizontal) cxy = coords(0.525, 0.85);
    else if (cHorizontal) cxy = bs.mx ? coords(0.8375, readUp ? 0.3125 : -0.3125) : coords(1.0, 0.0);
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

namespace EmulatedTravelTime {

    // The time unit is us

    //const uint32_t travelTimeFor1 = 8'333;
    const uint32_t travelTimeFor1 = 8'333; // Testing // 0.5f for 1 => 0.4f for 0 -> 0.8 (dash) and 0.9f to do -1 => 1 (dash dance)

    /* Algorithm explanation

    A separate thread repeatedly reads the GPIO and generate the gamecube report. The thread handling the communications just reads the latest report.
        
    TravelTimeState describes the current movement for one stick: origin, destination, time of departure and travel time (time to reach the destination)
    
    When a button press/release is detected, a new GC report will be generated and updateTravelTimeEmulation will be called.
    Provided the destination changed, it computes where the stick is currently, that is, [ (destination-origin)*(current time - time of departure)/travel time ]
    and updates the origin to that spot
    
    It then computes the distance to be travelled (norm of destination - origin) and multiplies it by travelTimeFor1 to obtain the travel time of the movement
    travelTimeFor1 is the time it takes to move a GC stick by 1 unit ie center -> full right (128 -> 208).

    applyTravelTimeEmulation is called after every GPIO set check regardless of whether a press/release happened.
    It updates the x/y value of the GCReport to be sent to where the stick is currently (the same formula as in updateTravelTimeEmulation) */

    /* Note: the travel time should probably not be linear with distance, we take less than 2x the time to do0=>1 when doing -1=>1
       and we take more than 0.5x the time to do 0=>0.5 than to do 0 => 1. sqrt may fit the bill */

    struct TravelTimeState {
        uint64_t timeOfDeparture = 0;
        uint32_t travelTime = 1;
        uint8_t xOrigin = 128; // Could be made more precise by using >uint8_t to keep track of sub-tick distances travelled
        uint8_t yOrigin = 128;
        uint8_t xDestination = 128;
        uint8_t yDestination = 128;
    };

    void applyTravelTimeEmulation(uint8_t &x, uint8_t &y, const TravelTimeState &tts, uint64_t timestamp) {
        // If the travel is complete, don't touch anything: the state build method returns the destination already
        bool travelComplete = (int64_t)(timestamp - tts.timeOfDeparture) > (int64_t)(tts.travelTime);
        if (!travelComplete) {
            float travelCompletionRatio = (timestamp - tts.timeOfDeparture) / (float)tts.travelTime;
            x = (uint8_t) ( (int)tts.xOrigin + ((int)tts.xDestination - (int)tts.xOrigin) * travelCompletionRatio + 0.5 );
            y = (uint8_t) ( (int)tts.yOrigin + ((int)tts.yDestination - (int)tts.yOrigin) * travelCompletionRatio + 0.5 );
        }
    }
    
    void updateTravelTimeEmulation(const uint8_t x, const uint8_t y, TravelTimeState &tts, uint64_t timestamp) {
        if (x != tts.xDestination || y != tts.yDestination) {
            // Update origin, destination and time of departure
            bool travelComplete = (int64_t)(timestamp - tts.timeOfDeparture) > (int64_t)(tts.travelTime);
            if (travelComplete) {
                tts.xOrigin = tts.xDestination;
                tts.yOrigin = tts.yDestination;
            }
            else {
                float travelCompletionRatio = (timestamp - tts.timeOfDeparture) / (float)tts.travelTime;
                tts.xOrigin = (uint8_t) ( (int)tts.xOrigin + ((int)tts.xDestination - (int)tts.xOrigin) * travelCompletionRatio + 0.5 );
                tts.yOrigin = (uint8_t) ( (int)tts.yOrigin + ((int)tts.yDestination - (int)tts.yOrigin) * travelCompletionRatio + 0.5 );
            }
            tts.xDestination = x;
            tts.yDestination = y;
            tts.timeOfDeparture = timestamp;

            // Compute travel time to reach destination
            double norm = sqrt( ((int)tts.yDestination-(int)tts.yOrigin)*((int)tts.yDestination-(int)tts.yOrigin) + ((int)tts.xDestination-(int)tts.xOrigin)*((int)tts.xDestination-(int)tts.xOrigin) );
            if (norm != 0) {
                tts.travelTime = (uint32_t) (norm*travelTimeFor1/80);
            }
        }
    }

    volatile GCReport core0GcReport;
    void core1_entry() {
        GpioToButtonSets::F1::ButtonSet(*getButtonSet)(void) = (GpioToButtonSets::F1::ButtonSet(*)(void)) multicore_fifo_pop_blocking();
        volatile GCReport *gcReportPtr = (volatile GCReport*) multicore_fifo_pop_blocking();
        
        GpioToButtonSets::F1::ButtonSet previousButtonSet;
        TravelTimeState travelTimeStateStick{};
        TravelTimeState travelTimeStateCStick{};
        GCReport localGcReport;

        while (true) {
            GpioToButtonSets::F1::ButtonSet buttonSet = getButtonSet();

            uint64_t timestamp = time_us_64();
            if ( memcmp((const void*)&buttonSet, (const void*)&previousButtonSet, sizeof(GpioToButtonSets::F1::ButtonSet)) ) {
                localGcReport = DACAlgorithms::MeleeF1::getGCReport(buttonSet);
                updateTravelTimeEmulation(localGcReport.xStick, localGcReport.yStick, travelTimeStateStick, timestamp);
                updateTravelTimeEmulation(localGcReport.cxStick, localGcReport.cyStick, travelTimeStateCStick, timestamp);
                previousButtonSet = buttonSet;
            }

            applyTravelTimeEmulation(localGcReport.xStick, localGcReport.yStick, travelTimeStateStick, timestamp);
            applyTravelTimeEmulation(localGcReport.cxStick, localGcReport.cyStick, travelTimeStateCStick, timestamp);

            memcpy ( (void*)gcReportPtr, (const void*) &localGcReport, sizeof(GCReport) );
        }
    }

    void initialize(GpioToButtonSets::F1::ButtonSet(*func)(void)) {
        multicore_launch_core1(core1_entry);
        multicore_fifo_push_blocking((uint32_t)func);
        multicore_fifo_push_blocking((uint32_t)&core0GcReport);
    }

    GCReport getGCReport() {
        GCReport gcReport;
        memcpy((void*)&gcReport, (const void*)&core0GcReport, sizeof(GCReport));
        return gcReport;
    }
}

}
}