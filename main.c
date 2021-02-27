#include "pico/stdlib.h"

#include "hardware/gpio.h"

#include "hardware/structs/systick.h"

#include "hardware/regs/pio.h"
#include "hardware/structs/pio.h"

// List of usable Pico input pins except 28 i.e 0-27 \ 23-25
#define inputPinsLength 25
const uint8_t inputPins[inputPinsLength] =
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 26, 27};

// because 28 is the GC data line
const uint8_t gcDataPin = 28;
const uint8_t ledPin = 25;

#define changeBit(r,b,v) r = (r & (~(1<<b))) | ((!!v)<<b)
#define accessBit(buffer, offset) ((buffer[offset/8] & (0x0080 >> (offset%8))) != 0)

int main() {

    /* Init */

    // Clock at 125MHz
    set_sys_clock_khz(125000, true);
    // returns true

    // Lib init
    stdio_init_all();

    // Led init
    gpio_init(ledPin);
    gpio_set_dir(ledPin, GPIO_OUT);
    changeBit(sio_hw->gpio_out, 25, 0);
    
    systick_hw->csr = (1 << 2) | (1 << 0);
    // rvr apparently default 0xFFFFFF in practice

    /*if (systick_hw->calib & ((1 << 31)|(1<<30))) {
        changeBit(sio_hw->gpio_out, 25, 1);
        while (1);
    }*/

    // 2^23 = 8388608
    // 2^23/125 = 67108.864
    // -> we can wait max 67ms with &0x8 checking, ~134ms all around

    // Inputs init
    for (int pinNo = 0; pinNo < inputPinsLength; ++pinNo) {
        gpio_init(inputPins[pinNo]);
        gpio_set_dir(inputPins[pinNo], GPIO_IN);
        gpio_pull_up(inputPins[pinNo]);
    }

    // GC data init
    gpio_init(gcDataPin);


    /* State machine */

    uint32_t lowTimings[100]; // should not >25
    uint32_t highTimings[100]; // should not >25
    bool readings[100]; // should not >25
    uint32_t readIndex=0;
    uint32_t responseBitLength=0;
    uint8_t response[10] = { 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00 };
    uint32_t origin;
    uint32_t target;

stateLabel_InputInit:

    gpio_set_dir(gcDataPin, GPIO_IN);
    //gpio_disable_pulls(gcDataPin);
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

    target = systick_hw->cvr - 1250; // if no low for 10us, reset query read
    while (sio_hw->gpio_in & (1 << gcDataPin)) {
        if ((systick_hw->cvr - target) & 0x00800000) goto stateLabel_InputInit;
    }
    lowTimings[readIndex] = systick_hw->cvr;
    goto stateLabel_InputAwaitHigh;
        

stateLabel_HandleConsoleProbe:

    changeBit(sio_hw->gpio_out, 25, 1);
    responseBitLength=24;
    response[0]=0x09;
    response[1]=0x00;
    response[2]=0x03;
    goto stateLabel_Respond;


stateLabel_HandleConsoleOriginQuery:

    responseBitLength=80;
    response[0]=0x00;
    response[1]=0x80;
    response[2]=128;
    response[3]=128;
    response[4]=128;
    response[5]=128;
    response[6]=0;
    response[7]=0;
    response[8]=0;
    response[9]=0;
    goto stateLabel_Respond;

stateLabel_HandleConsolePoll:

    responseBitLength=64;
    response[0]=0x00;
    response[1]=0x80;
    response[2]=128;
    response[3]=128;
    response[4]=128;
    response[5]=128;
    response[6]=0;
    response[7]=0;
    goto stateLabel_Respond;

stateLabel_Respond:

    // Response is 4us indexed

    // When we switch from input pull-up to output-high, we don't want a sudden low
    // Setting high before setting the direction adresses that
    changeBit(sio_hw->gpio_out, gcDataPin, 1);
    gpio_set_dir(gcDataPin, GPIO_OUT);

    origin = systick_hw->cvr - 200; // Padding for iteration 0 & mode swap

    for (uint32_t i=0; i<responseBitLength; i++) {

        target = origin - i*500; // origin + i*4us
        target &= 0x00FFFFFF;
        while ((target - systick_hw->cvr) & 0x00800000);
        changeBit(sio_hw->gpio_out, gcDataPin, 0);

        target = origin - i*500 - (accessBit(response, i) ? 125 : 375); // origin + i*4us + bit ? 1us : 3us;
        target &= 0x00FFFFFF;
        while ((target - systick_hw->cvr) & 0x00800000);
        changeBit(sio_hw->gpio_out, gcDataPin, 1);
    }

    // Send end bit
    target = origin - responseBitLength*500; // origin + 4us*length
    target &= 0x00FFFFFF;
    while ((target - systick_hw->cvr) & 0x00800000);
    changeBit(sio_hw->gpio_out, gcDataPin, 0);

    target = origin - responseBitLength*500 - 250; // origin + 4us*length + 2us
    target &= 0x00FFFFFF;
    while ((target - systick_hw->cvr) & 0x00800000);
    changeBit(sio_hw->gpio_out, gcDataPin, 1);

    goto stateLabel_InputInit;

    return 1;
}

    /*char number[10];
    sprintf(number, "%d\n", counter++);
    printf(number);
    sleep_ms(1000);
    gpio_put(LED_PIN, 0);
    sprintf(number, "%d\n", counter++);
    printf(number);
    sleep_ms(1000);
    gpio_put(LED_PIN, 1);*/
        
    /*gpio_init(3);
    gpio_set_dir(3, GPIO_IN);
    gpio_pull_up(3);

    while (1) {
        int32_t a = sio_hw->gpio_in;
        int32_t b = a & (1 << 3); // Read GPIO 3
        changeBit(sio_hw->gpio_out, 25, b);
    }*/

    /*uint32_t target = systick_hw->cvr;
    int i;
    while (1) {
        for (i=0; i<5000000; ++i) {
            target -= 125;
            target &= 0x00FFFFFF;
            while ((target - systick_hw->cvr) & 0x00800000);
        }
        changeBit(sio_hw->gpio_out, 25, 0);
        
        for (i=0; i<5000000; ++i) {
            target -= 125;
            target &= 0x00FFFFFF;
            while ((target - systick_hw->cvr) & 0x00800000);
        }
        changeBit(sio_hw->gpio_out, 25, 1);
    }*/

    /*while (1) {
        char number[30];
        sprintf(number, "%d\n", systick_hw->cvr);
        printf(number);
        sleep_ms(1000);
    }*/

/*bool inline accessBit(uint8_t* buffer, uint32_t offset) {
    return (buffer[offset/8] & (0x0080 >> (offset%8))) != 0; // Little endian
    //return (buffer[offset/8] & (0x0001 << (offset%8))) != 0; // Big endian
}*/