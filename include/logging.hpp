#ifndef __LOGGING_HPP
#define __LOGGING_HPP

#include "pico/stdlib.h"
#include <stdio.h>
#include "string.h"

void initialize_uart();

void log_uart_flush();

void log_uart(const char* str);
void log_uart_int(int i);
void log_uart_uint(uint32_t i);
void log_uart_float(float f);
void log_uart_array(uint8_t *ptr, uint16_t len);
void log_uart_array(int *ptr, uint16_t len);

inline void logf_uart(const char* str) {
    log_uart(str); log_uart_flush();
}
inline void logf_uart_int(int i) {
    log_uart_int(i); log_uart_flush();
}
inline void logf_uart_uint(int i) {
    log_uart_uint(i); log_uart_flush();
}
inline void logf_uart_array(uint8_t *ptr, uint16_t len) {
    log_uart_array(ptr, len); log_uart_flush();
}
inline void logf_uart_array(int *ptr, uint16_t len) {
    log_uart_array(ptr, len); log_uart_flush();
}

#endif