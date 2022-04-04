#include "other/nunchuk.hpp"
#include "global.hpp"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/time.h"
#include <stdio.h>

i2c_inst_t *i2c = i2c0;
uint8_t nunchuk_report_data[2] = {0x00, 0x00};
bool i2c_initialized = false;

void init_i2c() {
  if (i2c_initialized) {
    return;
  }

  gpio_set_function(NUNCHUK_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(NUNCHUK_SCL_PIN, GPIO_FUNC_I2C);

  gpio_pull_up(NUNCHUK_SDA_PIN);
  gpio_pull_up(NUNCHUK_SCL_PIN);

  i2c_init(i2c, 400 * 1000);
  i2c_initialized = true;
}

// This is a safely repeatable action, so no re-initialization guard
void init_nunchuk() {
  init_i2c();

  i2c_write_blocking(i2c, ADDR, INIT, 2, false);
  i2c_write_blocking(i2c, ADDR, &INIT[2], 2, false);
}

void read_nunchuk() {
  i2c_write_blocking(i2c, ADDR, &STATUS_REPORT, 1, false);
  i2c_read_blocking(i2c, ADDR, nunchuk_report_data, 2, false);
}

void fetch_nunchuk_reports() {
    while (true) {
        read_nunchuk();
    }
}
