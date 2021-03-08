#ifndef COMMS_HPP
#define COMMS_HPP

#include "pico/stdlib.h"

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

const GCReport defaultGcReport = {
    .a=0, .b=0, .x=0, .y=0, .start=0, .pad0=0,
    .dLeft=0, .dRight=0, .dUp=0, .dDown=0, .z=0, .r=0, .l=0, .pad1=1,
    .xStick=128,
    .yStick=128,
    .cxStick=128,
    .cyStick=128,
    .analogL=0,
    .analogR=0
};

/* Must be called before calling any of the other functions */
void initComms(uint8_t dataPin, uint32_t microsecondCycles);

/* Blocking wait on a poll request - handles responding to probe/origin requests */
void awaitPoll();

/* Sends arbitrary number of bytes to console */
void respond(uint8_t* responsePointer, uint32_t responseBitLength);

/* Sends a Gamecube controller state to console (helper) */
void respondToPoll(GCReport *gcReport);

#endif