#ifndef __NUNCHUK_HPP
#define __NUNCHUK_HPP

#include "global.hpp"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>

#define NUNCHUK_SDA_PIN 8
#define NUNCHUK_SCL_PIN 9

// Split into 2 chunks of 2 bytes each, this init sequence
// puts the nunchuk into unencrypted mode. The first two bytes
// of the generated report are the X-axis and Y-axis values,
// the rest is accelerometer data that we're going to ignore.
const uint8_t INIT[4] = {0xF0, 0x55, 0xFB, 0x00};
const uint8_t ADDR = 0x52; // This is the same for all models of nunchuk

// The nunchuk protocol involves sending the address to read/write
// as one byte and then either writing it with the next byte
// or requesting a read dataframe. State data is stored at 0x00
const uint8_t STATUS_REPORT = 0x00;

extern i2c_inst_t *i2c;
extern uint8_t nunchuk_report_data[2];
extern bool i2c_initialized;

void init_i2c();
void init_nunchuk();
void read_nunchuk();
void fetch_nunchuk_reports();

#endif
