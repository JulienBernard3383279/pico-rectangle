#include "communication_protocols/joybus.hpp"

#include "global.hpp"
#include <array>
#include <memory>
#include "string.h"

#include "hardware/gpio.h"

#include "hardware/pio.h"
#include "my_pio.pio.h"

#define CRCPP_USE_CPP11
#include "lib/CRC.h"

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

enum class MetacommCategory : uint8_t { // Scratch
    PROTOCOL_UPGRADES = 0,
    STATIC_DATA = 1, // Controller type, firmware version, slippi tag, unique ID within controller type
    PROGRAMMING = 2,
    POLL_RESPONSE_EXTENSIONS = 3,
    VENDOR = 4
};

enum class ProtocolUpgradeCommand : uint8_t {
    OLD_PROTOCOL_UPGRADE_REQUEST = 0,
    NEW_PROTOCOL_UPGRADE_REQUEST = 1,
    UPGRADE_ACCEPTED_BY_HOST = 2,

    NONE = 0xff
};

namespace CommunicationProtocols
{
namespace Joybus
{

std::array<uint8_t, 257> buffer{}; // 2 + 255 ;_;

bool stallRevertOnce = false;
bool revertModeNextPoll = false;
ProtocolUpgradeCommand previousProtocolUpgradeCommand = ProtocolUpgradeCommand::NONE;
uint8_t currentProtocolId = 0;
//bool mult5crc1011bmode = false;

PIO pio;
uint32_t offset;
pio_sm_config config;

CRC::Parameters<uint16_t, 16> crcParameters() {
    CRC::Parameters<uint16_t, 16> parameters;

    parameters.initialValue = 0;
    parameters.polynomial = 0x011b; // https://users.ece.cmu.edu/~koopman/crc/crc16.html 0x808d => best HV (5) for up to 99 bits, poll is 64 + gives some margin for metadata later
    // This library considers XMODEM to be polynomial 0x1021; when its polynomial is x^16 + x^12 + x^5 + 1; so polynomial x^16 + x^8 + x^4 + x^3 + x + 1 is 0x011b
    parameters.reflectInput = false;
    parameters.reflectOutput = false;
    parameters.finalXOR = 0;

    return parameters;
}

void respond(const uint8_t* data, const int len, int waitUs=6) {

    std::unique_ptr<uint8_t[]> data2;
    int len2;

    std::unique_ptr<uint32_t[]> result;
    int resultLen;
    
    if (currentProtocolId == 0) {
        len2 = len;
        data2 = std::make_unique<uint8_t[]>(len2);
        for (int i = 0; i<len; i++) {
            data2[i] = data[i];
        }
        resultLen = len/2 + 1;
        result = std::make_unique<uint32_t[]>(resultLen);
    }
    else { // == 1
        len2 = len + 2;
        data2 = std::make_unique<uint8_t[]>(len2);
        for (int i = 0; i<len; i++) {
            data2[i] = data[i];
        }
        auto crc = crcParameters();
        uint16_t result = CRC::Calculate((const void*)data, len, crc);
        memcpy((void*)(data2.get() + len), (const void*)(&result), 2);
    }

    convertToPio(data2.get(), len2, result.get(), resultLen);
    if (waitUs != 0) sleep_us(waitUs); // 3.75us into the bit before end bit => 6.25 to wait if the end-bit is 5us long

    pio_sm_set_enabled(pio, 0, false);
    pio_sm_init(pio, 0, offset+save_offset_outmode, &config);
    pio_sm_set_enabled(pio, 0, true);

    for (int i = 0; i<resultLen; i++) pio_sm_put_blocking(pio, 0, result[i]);
}

void respondNoWait(uint8_t* data, int len) { // Not = 0 because branching code for waits = ??
    respond(data, len, 0);
}

void respondMetacommKO() {
    uint8_t KO[1] = { 0x03 };
    respond(KO, 1);
}

void respondMetacommOK() {
    uint8_t OK[1] = { 0x3f };
    respond(OK, 1);
}

// returns true = error detected
bool checkForErrors(size_t commandLen) {
    if (currentProtocolId == 0) return false;

    auto crc = crcParameters();
    uint16_t result = CRC::Calculate((const void*)buffer.data(), commandLen, crc);

    buffer[commandLen] = pio_sm_get_blocking(pio, 0);
    buffer[commandLen+1] = pio_sm_get_blocking(pio, 0);

    return memcpy(&result, buffer.data()+commandLen, 2);// != *(uint16_t*)(buffer.data()+commandLen);
}

void enterMode(int dataPin, std::function<GCReport()> func) {

    gpio_init(dataPin);
    gpio_set_dir(dataPin, GPIO_IN);
    gpio_pull_up(dataPin);

    gpio_init(rumblePin);
    gpio_set_dir(rumblePin, GPIO_OUT);

    sleep_us(100); // Stabilize voltages

    pio = pio0;
    pio_gpio_init(pio, dataPin);
    offset = pio_add_program(pio, &save_program);

    config = save_program_get_default_config(offset);
    sm_config_set_in_pins(&config, dataPin);
    sm_config_set_out_pins(&config, dataPin, 1);
    sm_config_set_set_pins(&config, dataPin, 1);
    sm_config_set_clkdiv(&config, 5);
    sm_config_set_out_shift(&config, true, false, 32);
    sm_config_set_in_shift(&config, false, true, 8);
    
    pio_sm_init(pio, 0, offset, &config);
    pio_sm_set_enabled(pio, 0, true);
    
    //TODO The whole no wait thing is kinda hacky...

    while (true) {
        buffer[0] = pio_sm_get_blocking(pio, 0);

        if (buffer[0] == 0) { // Probe
            uint8_t probeResponse[3] = { 0x09, 0x00, 0x03 };
            if (checkForErrors(1)) goto command_rejected;
            respond(probeResponse, 3);
        }
        else if (buffer[0] == 0x41) { // Origin (NOT 0x81)
            gpio_put(25, 1);
            if (checkForErrors(1)) goto command_rejected;
            uint8_t originResponse[10] = { 0x00, 0x80, 128, 128, 128, 128, 0, 0, 0, 0 };
            // Here we don't wait because convertToPio takes time
            respondNoWait(originResponse, 10);
        }
        else if (buffer[0] == 0x40) { // Maybe poll //TODO Check later inputs...
            buffer[1] = pio_sm_get_blocking(pio, 0);
            buffer[2] = pio_sm_get_blocking(pio, 0);

            if (checkForErrors(3)) goto command_rejected;

            gpio_put(rumblePin, buffer[2] & 1);

            GCReport gcReport = func();

            respondNoWait((uint8_t*)(&gcReport), 8);
        }
        else if (buffer[0] == 0x3c) { // Metacomms
            //TODO Ideally shouldn't be right in the communication protocol file but it's gonna be tough responding through a call
            const uint8_t metaCommLen = buffer[1] = pio_sm_get_blocking(pio, 0);
            for (uint16_t i = 0; i<metaCommLen; i++) {
                buffer[i+2] = pio_sm_get_blocking(pio, 0); //TODO Needs some timeout...
            }
            if (checkForErrors(1+metaCommLen)) goto command_rejected;

            const uint8_t* metaBuf = buffer.data() + 2;
            if (metaCommLen == 0) { // Capabilities request
                uint8_t categoriesRequest[1] = { 1 << (uint8_t)MetacommCategory::PROTOCOL_UPGRADES }; // Only support protocol upgrades so far
                respond(categoriesRequest, 1);
            }
            else {
                const uint8_t postCategoryLen = metaCommLen - 1;
                switch ((MetacommCategory)metaBuf[0]) {
                    case MetacommCategory::PROTOCOL_UPGRADES:
                        if (postCategoryLen == 0) { // List supported protocols
                            uint8_t canDoProtocol1[2] = { 1, 2 };  // len 1, protocol 1 => bit 1; protocol 0 is default joybus
                            respond(canDoProtocol1, 2);
                        }
                        else if (postCategoryLen>=2 && metaBuf[2] < 2) { // [command] [protocol#]
                            ProtocolUpgradeCommand command = (ProtocolUpgradeCommand)metaBuf[1];
                            uint8_t protocolId = metaBuf[2];
                            switch (command) {
                                case ProtocolUpgradeCommand::OLD_PROTOCOL_UPGRADE_REQUEST:
                                    respondMetacommOK();
                                    currentProtocolId = protocolId;
                                    stallRevertOnce = true;
                                    revertModeNextPoll = true;
                                    previousProtocolUpgradeCommand = command;
                                    break;
                                case ProtocolUpgradeCommand::NEW_PROTOCOL_UPGRADE_REQUEST:
                                    if (previousProtocolUpgradeCommand == ProtocolUpgradeCommand::OLD_PROTOCOL_UPGRADE_REQUEST) {
                                        respondMetacommOK();
                                        stallRevertOnce = true;
                                    }
                                    else {
                                        respondMetacommKO();
                                    }
                                    break;
                                case ProtocolUpgradeCommand::UPGRADE_ACCEPTED_BY_HOST:
                                    if (previousProtocolUpgradeCommand == ProtocolUpgradeCommand::NEW_PROTOCOL_UPGRADE_REQUEST) {
                                        respondMetacommOK();
                                        revertModeNextPoll = false;
                                    }
                                    else {
                                        respondMetacommKO(); // This won't abort the protocol change (uh oh)
                                    }
                                    previousProtocolUpgradeCommand = ProtocolUpgradeCommand::NONE;
                                    break;
                                default:
                                    respondMetacommKO();
                            }
                        }
                        else {
                            respondMetacommKO();
                        }
                        break;
                    default:
                        respondMetacommKO();
                        break;
                }
            }
        }
        else {
            command_rejected:
            pio_sm_set_enabled(pio, 0, false);
            sleep_us(400);
            pio_sm_init(pio, 0, offset+save_offset_inmode, &config);
            pio_sm_set_enabled(pio, 0, true);
        }

        if (!stallRevertOnce && revertModeNextPoll) {
            currentProtocolId = 1-currentProtocolId; // Will have to do better if it ever supports more than 2
            previousProtocolUpgradeCommand = ProtocolUpgradeCommand::NONE;
        }
        stallRevertOnce = false;
    }
}

}
}
