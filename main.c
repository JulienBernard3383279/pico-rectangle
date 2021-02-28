#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/structs/systick.h"
#include "hardware/regs/pio.h"
#include "hardware/structs/pio.h"

const bool banParasolDashing = true;
const bool banSlightSideB = true;

struct GCReport {
    uint8_t a : 1; uint8_t b : 1; uint8_t x:1; uint8_t y : 1; uint8_t start : 1; uint8_t pad0 : 3;
    uint8_t dLeft : 1; uint8_t dRight : 1; uint8_t dUp : 1; uint8_t dDown : 1; uint8_t z : 1; uint8_t r : 1; uint8_t l : 1; uint8_t pad1 : 1;
    uint8_t xStick;
    uint8_t yStick;
    uint8_t cxStick;
    uint8_t cyStick;
    uint8_t analogL;
    uint8_t analogR;
};

const struct GCReport defaultReport = {
    .a=0, .b=0, .x=0, .y=0, .start=0, .pad0=0,
    .dLeft=0, .dRight=0, .dDown=0, .dUp=0, .z=0, .r=0, .l=0, .pad1=1,
    .xStick=128,
    .yStick=128,
    .cxStick=128,
    .cyStick=128,
    .analogL=0,
    .analogR=0
};

struct RectangleInput {
    bool a; bool b; bool x; bool y; bool z;
    bool l; bool r; bool ls; bool ms;
    bool mx; bool my;
    bool start;
    bool left; bool right; bool up; bool down;
    bool cLeft; bool cRight; bool cUp; bool cDown;
};

struct pinMapping {
    uint8_t pin;
    uint8_t offset;
};

#define NUMBER_OF_INPUTS 20
const struct pinMapping pinMappings[NUMBER_OF_INPUTS] = {
    { 0, offsetof(struct RectangleInput, start) },
    { 2, offsetof(struct RectangleInput, right) },
    { 3, offsetof(struct RectangleInput, down) },
    { 4, offsetof(struct RectangleInput, left) },
    { 5, offsetof(struct RectangleInput, l) },
    { 6, offsetof(struct RectangleInput, mx) },
    { 7, offsetof(struct RectangleInput, my) },
    { 12, offsetof(struct RectangleInput, cUp) },
    { 13, offsetof(struct RectangleInput, cLeft) },
    { 14, offsetof(struct RectangleInput, a) },
    { 15, offsetof(struct RectangleInput, cDown) },
    { 16, offsetof(struct RectangleInput, cRight) },
    { 17, offsetof(struct RectangleInput, up) },
    { 18, offsetof(struct RectangleInput, ms) },
    { 19, offsetof(struct RectangleInput, z) },
    { 20, offsetof(struct RectangleInput, ls) },
    { 21, offsetof(struct RectangleInput, x) },
    { 22, offsetof(struct RectangleInput, y) },
    { 26, offsetof(struct RectangleInput, b) },
    { 27, offsetof(struct RectangleInput, r) }
};

const uint8_t gcDataPin = 28;
const uint8_t ledPin = 25;

#define changeBit(r,b,v) r = (r & (~(1<<b))) | ((!!v)<<b)
#define accessBit(buffer, offset) ((buffer[offset/8] & (0x0080 >> (offset%8))) != 0)
#define coord(x) ((uint8_t)(128. + 80.*x + 0.5))
#define oppositeCoord(x) (128 - (x - 128))

struct Coords {
    uint8_t x;
    uint8_t y;
};

inline struct Coords coords(float xFloat, float yFloat) {
    struct Coords r;
    r.x = coord(xFloat);
    r.y = coord(yFloat);
    return r;
}

const uint32_t us = 125;

int main() {

    // Clock at 125MHz
    set_sys_clock_khz(us*1000, true);

    // Led init
    gpio_init(ledPin);
    gpio_set_dir(ledPin, GPIO_OUT);
    changeBit(sio_hw->gpio_out, 25, 0);
    
    systick_hw->csr = (1 << 2) | (1 << 0);
    
    // 2^23 = 8388608
    // 2^23/125 = 67108.864
    // -> we can wait max 67ms with &0x8 checking, ~134ms all around

    // Inputs init
    for (int pinNo = 0; pinNo < NUMBER_OF_INPUTS; ++pinNo) {
        gpio_init(pinMappings[pinNo].pin);
        gpio_set_dir(pinMappings[pinNo].pin, GPIO_IN);
        gpio_pull_up(pinMappings[pinNo].pin);
    }

    // GC data init
    gpio_init(gcDataPin);

    // State machine declarations
    uint32_t lowTimings[100];
    uint32_t highTimings[100];
    bool readings[100];
    uint32_t readIndex=0;
    uint32_t responseBitLength=0;
    uint8_t responseBuffer[10] = { 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00 };
    uint8_t* responsePointer = responseBuffer;
    uint32_t origin;
    uint32_t target;
    
    // 2 IP declarations
    bool left_wasPressed = false;
    bool right_wasPressed = false;
    bool up_wasPressed = false;
    bool down_wasPressed = false;
    
    bool left_outlawUntilRelease = false;
    bool right_outlawUntilRelease = false;
    bool up_outlawUntilRelease = false;
    bool down_outlawUntilRelease = false;

    // Poll response declarations
    uint32_t inputSnapshot;
    struct GCReport gcReport;
    struct RectangleInput ri;
    
stateLabel_InputInit:

    gpio_set_dir(gcDataPin, GPIO_IN);
    gpio_pull_up(gcDataPin);

    while (sio_hw->gpio_in & (1 << gcDataPin));

    // New message from console, init
    readIndex = 0;
    lowTimings[readIndex] = systick_hw->cvr;

stateLabel_InputAwaitHigh:

    while (!(sio_hw->gpio_in & (1 << gcDataPin)));

    // End-bit (50% low 50% high) -> treat as high bit
    // 5 us indexed queries, 4 us indexed responses
    // End-bit should be 2.5 us low, high bit 1.25us low, low 3.75us low
    // Check < 2.8us low
    // 2.8us = 350 cycles

    highTimings[readIndex] = systick_hw->cvr;
    readings[readIndex] = (lowTimings[readIndex] - highTimings[readIndex]) < 350;

    readIndex++;
    if (readIndex==100) goto stateLabel_InputInit; // shields against garbage data on gc line

    // Look for bit patterns

    if (readIndex==9 &&
        (readings[8] == true) &&
        (readings[0] == false) && (readings[2] == false) && 
        (readings[3] == false) && (readings[4] == false) &&
        (readings[5] == false) && (readings[6] == false))
    {
        if (readings[1] == false && readings[7] == false)
            goto stateLabel_HandleConsoleProbe;
        if (readings[1] == true && readings[7] == true)
            goto stateLabel_HandleConsoleOriginQuery;
    }

    if (readIndex==25 &&
        (readings[24] == true) && (readings[0] == false) &&
        (readings[1] == true) && (readings[2] == false) &&
        (readings[3] == false) && (readings[4] == false) &&
        (readings[5] == false) && (readings[6] == false) &&
        (readings[7] == false) && (readings[8] == false) &&
        (readings[9] == false) && (readings[10] == false) &&
        (readings[11] == false) && (readings[12] == false) &&
        (readings[13] == false) && (readings[14] == true) &&
        (readings[15] == true) && (readings[16] == false) &&
        (readings[17] == false) && (readings[18] == false) &&
        (readings[19] == false) && (readings[20] == false) &&
        (readings[21] == false) && (readings[22] == false))
            goto stateLabel_HandleConsolePoll;

stateLabel_InputAwaitLow:

    target = systick_hw->cvr - 10*us; // if no low for 10us, reset query read
    while (sio_hw->gpio_in & (1 << gcDataPin)) {
        if ((systick_hw->cvr - target) & 0x00800000) goto stateLabel_InputInit;
    }
    lowTimings[readIndex] = systick_hw->cvr;
    goto stateLabel_InputAwaitHigh;
        

stateLabel_HandleConsoleProbe:

    changeBit(sio_hw->gpio_out, ledPin, 1);
    
    responseBuffer[0]=0x09;
    responseBuffer[1]=0x00;
    responseBuffer[2]=0x03;

    responseBitLength=24;
    responsePointer = responseBuffer;

    goto stateLabel_Respond;

stateLabel_HandleConsoleOriginQuery:

    responseBuffer[0]=0x00;
    responseBuffer[1]=0x80;
    responseBuffer[2]=128;
    responseBuffer[3]=128;
    responseBuffer[4]=128;
    responseBuffer[5]=128;
    responseBuffer[6]=0;
    responseBuffer[7]=0;
    responseBuffer[8]=0;
    responseBuffer[9]=0;

    responseBitLength=80;
    responsePointer = responseBuffer;

    goto stateLabel_Respond;

stateLabel_Respond:

    // Response is 4us indexed

    // When we switch from input pull-up to output-high, we don't want a sudden low
    // Setting high before setting the direction adresses that
    changeBit(sio_hw->gpio_out, gcDataPin, 1);
    gpio_set_dir(gcDataPin, GPIO_OUT);

    origin = systick_hw->cvr;

    for (uint32_t i=0; i<responseBitLength; i++) {

        target = origin - i*4*us;
        while ((target - systick_hw->cvr) & 0x00800000);
        changeBit(sio_hw->gpio_out, gcDataPin, 0);

        target = origin - i*4*us - (accessBit(responsePointer, i) ? us : 3*us);
        while ((target - systick_hw->cvr) & 0x00800000);
        changeBit(sio_hw->gpio_out, gcDataPin, 1);
    }

    // Send end bit
    target = origin - responseBitLength*4*us;
    while ((target - systick_hw->cvr) & 0x00800000);
    changeBit(sio_hw->gpio_out, gcDataPin, 0);

    target = origin - responseBitLength*4*us - 2*us;
    while ((target - systick_hw->cvr) & 0x00800000);
    changeBit(sio_hw->gpio_out, gcDataPin, 1);

    goto stateLabel_InputInit;

stateLabel_HandleConsolePoll:

    // Button to controller state translation
    inputSnapshot = sio_hw->gpio_in;

    gcReport = defaultReport;

    for (int pinNo = 0; pinNo < NUMBER_OF_INPUTS; ++pinNo) {
        *((bool*)( (char*)&ri + pinMappings[pinNo].offset)) = !(inputSnapshot & (1 << (pinMappings[pinNo].pin)));
    }

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

    struct Coords xy;

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

    struct Coords cxy;

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

    responsePointer = (uint8_t*) &gcReport;
    responseBitLength=64;

    //TODO Nerfs

    goto stateLabel_Respond;

    return 1;
}