#ifndef __GLOBAL_HPP
#define __GLOBAL_HPP

#include <stdio.h>

const int us = 125;

#define LED_PIN 25
#define USB_POWER_PIN 24

const uint8_t gcDataPin = 28;

#define USE_UART0 0

class Messages {
    public:
        const char * modeMeleeAdapter = "Melee Adapter";
        const char * three = "Three";
        const char * two = "Two";
        const char * one = "One";
        const char * restart = "Restart";
};

Messages const messages;

#endif