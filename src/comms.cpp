#include "hardware/structs/systick.h"
#include "hardware/gpio.h"

#include "comms.hpp"

#define changeBit(r,b,v) r = (r & (~(1<<b))) | ((!!v)<<b)
#define accessBit(buffer, offset) ((buffer[offset/8] & (0x0080 >> (offset%8))) != 0)

uint8_t gcDataPin;
uint32_t us;

void initComms(uint8_t dataPin, uint32_t microsecondCycles) {

    gcDataPin = dataPin;
    us = microsecondCycles;

    gpio_init(gcDataPin);
    gpio_set_dir(gcDataPin, GPIO_IN);
    gpio_pull_up(gcDataPin);

    // Configure timer
    systick_hw->csr = (1 << 2) | (1 << 0);

    // 2^23 = 8388608
    // 2^23/125 = 67108.864
    // -> we can wait max 67ms with &0x8 checking, ~134ms all around
}

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

void awaitPoll() {

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
            return;

stateLabel_InputAwaitLow:

    target = systick_hw->cvr - 10*us; // if no low for 10us, reset query read
    while (sio_hw->gpio_in & (1 << gcDataPin)) {
        if ((systick_hw->cvr - target) & 0x00800000) goto stateLabel_InputInit;
    }
    lowTimings[readIndex] = systick_hw->cvr;
    goto stateLabel_InputAwaitHigh;

stateLabel_HandleConsoleProbe:

    responseBuffer[0]=0x09;
    responseBuffer[1]=0x00;
    responseBuffer[2]=0x03;

    responseBitLength=24;
    responsePointer = responseBuffer;

    respond(responsePointer, responseBitLength);
    goto stateLabel_InputInit;

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

    respond(responsePointer, responseBitLength);
    goto stateLabel_InputInit;
}

void respond(uint8_t* responsePointer, uint32_t responseBitLength) {
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
}

void respondToPoll(GCReport *gcReport) {
    respond((uint8_t*)gcReport, 64);
}