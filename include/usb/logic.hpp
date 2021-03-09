#ifndef USB__LOGIC_HPP
#define USB__LOGIC_HPP

#include "pico/types.h"

// BIT FIELDS ARE LSB
struct __attribute__((packed)) AdapterReport {
    uint8_t fixed;
    uint8_t controllerType;
    uint8_t a:1; uint8_t b:1; uint8_t x:1; uint8_t y:1; uint8_t dLeft:1; uint8_t dRight:1; uint8_t dDown:1; uint8_t dUp:1;
    uint8_t start : 1; uint8_t z:1; uint8_t r:1; uint8_t l : 1; uint8_t padding:4;
    uint8_t xStick;
    uint8_t yStick;
    uint8_t cxStick;
    uint8_t cyStick;
    uint8_t analogL;
    uint8_t analogR;
    uint8_t remain[27];
};

const AdapterReport defaultAdapterReport = {
    33,
    0x10,
    0, 0, 0, 0, 0, 0, 0, 0,
    0,          0, 0, 0, 0,
    128,
    128,
    128,
    128,
    128,
    128,
    {0}
}; // partial initialization -> rest is 0

extern "C" uint8_t* build_usb_report(void);

extern "C" void inform_in_transfer_completed(void);

extern "C" void wait_until_n_us_before_in_transfer(uint32_t n);

extern "C" void initUsbLogic(uint32_t us);

#endif