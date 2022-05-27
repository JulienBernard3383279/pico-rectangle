#include "communication_protocols/joybus.hpp"

#include "global.hpp"

#include "hardware/gpio.h"

#include "hardware/pio.h"
#include "my_pio.pio.h"

// PIO Shifts to the right by default
// In: pushes batches of 8 shifted left, i.e we get [0x40, 0x03, rumble (the end bit is never pushed)]
// Out: We push commands for a right shift with an enable pin, ie 5 (101) would be 0b11'10'11
// So in doesn't need post processing but out does
void convertToPio(const uint8_t* command, const int len, uint32_t* result, int& resultLen) {
    if (len == 0) {
        resultLen = 0;
        return;
    }
    resultLen = len/2 + 1;
    int i;
    for (i = 0; i < resultLen; i++) {
        result[i] = 0;
    }
    for (i = 0; i < len; i++) {
        for (int j = 0; j < 8; j++) {
            result[i / 2] += 1 << (2 * (8 * (i % 2) + j) + 1);
            result[i / 2] += (!!(command[i] & (0x80u >> j))) << (2 * (8 * (i % 2) + j));
        }
    }
    // End bit
    result[len / 2] += 3 << (2 * (8 * (len % 2)));
}

namespace CommunicationProtocols
{
namespace Joybus
{

void enterMode(int dataPin, std::function<GCReport()> func) {
    gpio_init(dataPin);
    gpio_set_dir(dataPin, GPIO_IN);
    gpio_pull_up(dataPin);

    gpio_init(rumblePin);
    gpio_set_dir(rumblePin, GPIO_OUT);

    sleep_us(100); // Stabilize voltages

    PIO pio = pio0;
    pio_gpio_init(pio, dataPin);
    uint offset = pio_add_program(pio, &save_program);

    pio_sm_config config = save_program_get_default_config(offset);
    sm_config_set_in_pins(&config, dataPin);
    sm_config_set_out_pins(&config, dataPin, 1);
    sm_config_set_set_pins(&config, dataPin, 1);
    sm_config_set_clkdiv(&config, 5);
    sm_config_set_out_shift(&config, true, false, 32);
    sm_config_set_in_shift(&config, false, true, 8);
    
    pio_sm_init(pio, 0, offset, &config);
    pio_sm_set_enabled(pio, 0, true);
    
    while (true) {
        uint8_t buffer[3];
        buffer[0] = pio_sm_get_blocking(pio, 0);

        if (buffer[0] == 0) { // Probe
            uint8_t probeResponse[3] = { 0x09, 0x00, 0x03 };
            uint32_t result[2];
            int resultLen;
            convertToPio(probeResponse, 3, result, resultLen);
            sleep_us(6); // 3.75us into the bit before end bit => 6.25 to wait if the end-bit is 5us long

            pio_sm_set_enabled(pio, 0, false);
            pio_sm_init(pio, 0, offset+save_offset_outmode, &config);
            pio_sm_set_enabled(pio, 0, true);

            for (int i = 0; i<resultLen; i++) pio_sm_put_blocking(pio, 0, result[i]);
        }
        else if (buffer[0] == 0x41) { // Origin (NOT 0x81)
            gpio_put(25, 1);
            uint8_t originResponse[10] = { 0x00, 0x80, 128, 128, 128, 128, 0, 0, 0, 0 };
            uint32_t result[6];
            int resultLen;
            convertToPio(originResponse, 10, result, resultLen);
            // Here we don't wait because convertToPio takes time

            pio_sm_set_enabled(pio, 0, false);
            pio_sm_init(pio, 0, offset+save_offset_outmode, &config);
            pio_sm_set_enabled(pio, 0, true);

            for (int i = 0; i<resultLen; i++) pio_sm_put_blocking(pio, 0, result[i]);
        }
        else if (buffer[0] == 0x40) { // Maybe poll //TODO Check later inputs...
            buffer[0] = pio_sm_get_blocking(pio, 0);
            buffer[0] = pio_sm_get_blocking(pio, 0);
            gpio_put(rumblePin, buffer[0] & 1);

            GCReport gcReport = func();

            uint32_t result[5];
            int resultLen;
            convertToPio((uint8_t*)(&gcReport), 8, result, resultLen);

            pio_sm_set_enabled(pio, 0, false);
            pio_sm_init(pio, 0, offset+save_offset_outmode, &config);
            pio_sm_set_enabled(pio, 0, true);

            for (int i = 0; i<resultLen; i++) pio_sm_put_blocking(pio, 0, result[i]);
        }
        else {
            pio_sm_set_enabled(pio, 0, false);
            sleep_us(400);
            pio_sm_init(pio, 0, offset+save_offset_inmode, &config);
            pio_sm_set_enabled(pio, 0, true);
        }
    }
}

}
}
