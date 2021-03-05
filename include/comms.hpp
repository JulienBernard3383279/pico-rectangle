#include "pico/stdlib.h"

#ifndef COMMS_HPP
#define COMMS_HPP

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

const extern GCReport defaultReport;

/* Must be called before calling any of the other functions */
void initComms(uint8_t dataPin, uint32_t microsecondCycles);

/* Blocking wait on a poll request - handles responding to probe/origin requests */
void awaitPoll();

/* Sends arbitrary number of bytes to console */
void respond(uint8_t* responsePointer, uint32_t responseBitLength);

/* Sends a Gamecube controller state to console (helper) */
void respondToPoll(GCReport *gcReport);

#endif