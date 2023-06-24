#include "logging.hpp"

#define USE_UART_GLOBAL (USE_UART0 | USE_UART1)
#define USED_UART (USE_UART0 ? uart0 : uart1)

#define UART_SPEED (115200*5)

void initialize_uart() {
    #if USE_UART_GLOBAL

    uart_init(USED_UART, UART_SPEED); 
    // Set the GPIO pin mux to the UART
    #if PROD_BOARD
    gpio_set_function(16, GPIO_FUNC_UART);
    gpio_set_function(17, GPIO_FUNC_UART);
    #else
    gpio_set_function(USE_UART0 ? 0 : 8, GPIO_FUNC_UART);
    gpio_set_function(USE_UART0 ? 1 : 9, GPIO_FUNC_UART);
    #endif

    #endif
}

char cyclic_logging_buffer[4096];
int cyclic_logging_buffer_start=0;
int cyclic_logging_buffer_end=0;

void increment_boundary(int& i) {
    i++;
    if (i == 4096) i = 0;
}

void log_uart_flush() {
    #if USE_UART_GLOBAL
    while (cyclic_logging_buffer_start != cyclic_logging_buffer_end) {
        uart_putc(USED_UART, cyclic_logging_buffer[cyclic_logging_buffer_start]);
        increment_boundary(cyclic_logging_buffer_start);
    }
    #endif
}

void log_uart_full_flush() {
    #if USE_UART_GLOBAL
    char cyclic_logging_buffer_copy[4097];
    memcpy(cyclic_logging_buffer_copy, cyclic_logging_buffer + cyclic_logging_buffer_start, 4096 - cyclic_logging_buffer_start);
    memcpy(cyclic_logging_buffer_copy + (4096 - cyclic_logging_buffer_start), cyclic_logging_buffer, cyclic_logging_buffer_start);
    cyclic_logging_buffer_copy[4096] = 0;
    uart_puts(USED_UART, cyclic_logging_buffer_copy);
    uart_puts(USED_UART, "UART_CYCLIC_BUFFER_OVERFLOWED");
    #endif
}

void log_uart_put(const char* str) {
    for (const char* pc = str; *pc != 0; pc++) {
        cyclic_logging_buffer[cyclic_logging_buffer_end] = *pc;
        increment_boundary(cyclic_logging_buffer_end);
        if (cyclic_logging_buffer_end == cyclic_logging_buffer_start) log_uart_full_flush();
    }
}

void log_uart(const char* str) {
    #if USE_UART_GLOBAL
    log_uart_put(str);
    #endif
}
void log_uart_int(int i) {
    #if USE_UART_GLOBAL
    char str[16];
    sprintf(str, "%d", i);
    log_uart_put(str);
    #endif
}
void log_uart_uint(uint32_t u) {
    #if USE_UART_GLOBAL
    char str[16];
    sprintf(str, "%u", u);
    log_uart_put(str);
    #endif
}

void log_uart_float(float f) {
    #if USE_UART_GLOBAL
    char str[64];
    int ret = snprintf(str, sizeof(str), "%f", f);
    log_uart_put(str);
    #endif
}

void log_uart_array(const uint8_t *ptr, uint16_t len) {
    #if USE_UART_GLOBAL
    log_uart("array len=");log_uart_int(len);log_uart("\n");
    for (int i = 0; i<len; i++) {
        log_uart_int(ptr[i]); log_uart(" ");
    }
    log_uart("\n");
    #endif
}

void log_uart_array(const uint32_t *ptr, uint16_t len) {
    #if USE_UART_GLOBAL
    log_uart("array len=");log_uart_int(len);log_uart("\n");
    for (int i = 0; i<len; i++) {
        log_uart_uint(ptr[i]); log_uart(" ");
    }
    log_uart("\n");
    #endif
}

void log_uart_array(const int *ptr, uint16_t len) {
    #if USE_UART_GLOBAL
    log_uart("array len=");log_uart_int(len);log_uart("\n");
    for (int i = 0; i<len; i++) {
        log_uart_int(ptr[i]); log_uart(" ");
    }
    log_uart("\n");
    #endif
}