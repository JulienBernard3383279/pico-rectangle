#include "communication_protocols/usb.hpp"

#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

// Include the clock frequency
#include "global.hpp"
// Include the structs
#include "communication_protocols/usb/common.hpp"
#include "communication_protocols/usb/xinput.hpp"
// USB register definitions from pico-sdk
#include "hardware/regs/usb.h"
// USB hardware struct definitions from pico-sdk
#include "hardware/structs/usb.h"
// For interrupt enable and numbers
#include "hardware/irq.h"
// For resetting the USB controller
#include "hardware/resets.h"
// For the wait in the main loop
#include "hardware/structs/systick.h"

void log_uart0(const char* str) {
    #if USE_UART0
    uart_puts(uart0, str);
    #endif
}
void log_uart0_int(int i) {
    #if USE_UART0
    char str[16];
    sprintf(str, "%d", i);
    uart_puts(uart0, str);
    #endif
}

void log_uart0_array(uint8_t *ptr, uint16_t len) {
    log_uart0("array len=");log_uart0_int(len);log_uart0("\n");
    for (int i = 0; i<len; i++) {
        log_uart0_int(ptr[i]); log_uart0(" ");
    }
    log_uart0("\n");
}




/* dev_lowlevel example, migrated to C++, with fewer bugs, extended with WCID & multi-packet control transfer capabilities, and parameterized */

#define min(a,b) (a>b?b:a)

#define usb_hw_set hw_set_alias(usb_hw)
#define usb_hw_clear hw_clear_alias(usb_hw)

// Function prototypes for our device specific endpoint handlers defined
// later on
void ep0_in_handler(uint8_t *buf, uint16_t len);
void ep0_out_handler(uint8_t *buf, uint16_t len);
void ep_in_handler(uint8_t *buf, uint16_t len);
void ep_out_handler(uint8_t *buf, uint16_t len);

uint8_t epOutId = 1;

uint8_t ep0_in_addr() { return 0x80; }
uint8_t ep0_out_addr() { return 0x00; }
uint8_t ep_in_addr() { return 0x81; }
uint8_t ep_out_addr() { return epOutId; }





/* LOCAL STRUCTURES */
// Defined on the spot when const, else initialized in the entry point

// Global device address
bool should_set_address = false;
uint8_t dev_addr = 0;
volatile bool configured = false;

// Global data buffer for EP0
uint8_t ep0_buf[1024]; // Used to be 64. Sending a string of length > 32 in the string descriptor would cause undefined behaviour. Fun !
// We have 2MB so it's not like I give a fuck about reserving 1024 bytes

// Struct in which we keep the endpoint configuration
typedef void (*usb_ep_handler)(uint8_t *buf, uint16_t len);
struct usb_endpoint_configuration {
    const usb_endpoint_descriptor *descriptor;
    usb_ep_handler handler;

    // Pointers to endpoint + buffer control registers
    // in the USB controller DPSRAM
    volatile uint32_t *endpoint_control;
    volatile uint32_t *buffer_control;
    volatile uint8_t *data_buffer;

    // Toggle after each packet (unless replying to a SETUP)
    uint8_t next_pid;
};

// Struct in which we keep the device configuration
struct usb_device_configuration {
    const usb_device_descriptor *device_descriptor;
    const usb_interface_descriptor *interface_descriptor;
    const usb_configuration_descriptor *config_descriptor;
    const usb_hid_descriptor *hid_descriptor;
    const char *lang_descriptor;
    const char **descriptor_strings;
    usb_endpoint_configuration endpoints[USB_NUM_ENDPOINTS];
};

// Construct these in entry point
usb_device_configuration dev_config;
usb_device_descriptor device_descriptor;
usb_interface_descriptor interface_descriptor;
usb_configuration_descriptor config_descriptor;

usb_hid_descriptor hid_descriptor;

usb_endpoint_descriptor ep_in;
usb_endpoint_descriptor ep_out;

const char **descriptor_strings;
uint16_t descriptor_strings_len;

/* Always US english lang ID */
const char lang_descriptor[] = {
        4,         // bLength
        0x03,      // bDescriptorType == String Descriptor
        0x09, 0x04 // language id = us english
};

/* Control endpoints don't depend on the mode */
const usb_endpoint_descriptor ep0_out = {
        .bLength          = sizeof(struct usb_endpoint_descriptor),
        .bDescriptorType  = USB_DT_ENDPOINT,
        .bEndpointAddress = ep0_out_addr(), // EP0, OUT from host (device to host)
        .bmAttributes     = USB_TRANSFER_TYPE_CONTROL,
        .wMaxPacketSize   = 64,
        .bInterval        = 0
};
const usb_endpoint_descriptor ep0_in = {
        .bLength          = sizeof(struct usb_endpoint_descriptor),
        .bDescriptorType  = USB_DT_ENDPOINT,
        .bEndpointAddress = ep0_in_addr(), // EP0, IN from host (host to device)
        .bmAttributes     = USB_TRANSFER_TYPE_CONTROL,
        .wMaxPacketSize   = 64,
        .bInterval        = 0
};

bool useHID = false;
bool useXInput = false;

uint8_t *hid_report_descriptor;
uint16_t hid_report_descriptor_len;

/* Automatic WinUSB assignment descriptors */

// Built with the help of https://www.silabs.com/community/mcu/32-bit/forum.topic.html/using_microsoft_osd-lMCD
// and https://github.com/pbatard/libwdi/wiki/WCID-Devices

bool useWinUSB = false;
#define EXTENDED_COMPATIBILITY_ID 4
#define WINUSB_VENDOR_CODE 0xAF

struct extended_compatibility_interface_descriptor {
	uint8_t firstInterfaceNumber; /**< Interface number for which an extended compatibility feature descriptor is defined */
	uint8_t reserved1;
	char compatibleID[8]; /**< String describing the compatible id */
	char subCompatibleID[8]; /**< String describing the sub compatible id */
	uint8_t reserved2[6];
} __attribute__((packed));

struct t_ext_comp_desc {
	uint32_t length; /**< Size of this struct = 16 + bCount*24 */
	uint16_t bcdVersion; /**< 1.00 -> 0x0100 */
	uint16_t index;	/**< Command index - 0x04 for extended compatibility id */
	uint8_t count;		/**< Number of interfaces for which an extended compatibility feature descriptor is defined */
	uint8_t reserved1[7];
	extended_compatibility_interface_descriptor ecid;
} __attribute__((packed));

const t_ext_comp_desc extendedCompatibilityIdFeatureDescriptorWinUSB =
{
	40, // length
	0x0100, // bcdVersion
	EXTENDED_COMPATIBILITY_ID,
	1, // one interface with extended compatibility
	{0, 0, 0, 0, 0, 0, 0},
	{
        0, // Interface number 0
        0x01, // Reserved
        {0x57, 0x49, 0x4E, 0x55, 0x53, 0x42, 0x00, 0x00 }, // "WINUSB\0\0"
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0}
	}
};

const t_ext_comp_desc extendedCompatibilityIdFeatureDescriptorXInput =
{
	40,
	0x0100,
	EXTENDED_COMPATIBILITY_ID,
	1,
	{0, 0, 0, 0, 0, 0, 0},
	{
        0,
        0x01,
        {0x58, 0x55, 0x53, 0x42, 0x31, 0x30, 0x00, 0x00 }, // "XUSB10\0\0"
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0}
	}
};








/* LOGIC */

/**
 * @brief Given an endpoint address, return the usb_endpoint_configuration of that endpoint. Returns NULL
 * if an endpoint of that address is not found.
 *
 * @param addr
 * @return usb_endpoint_configuration*
 */
usb_endpoint_configuration *usb_get_endpoint_configuration(uint8_t addr) {
    usb_endpoint_configuration *endpoints = dev_config.endpoints;
    for (int i = 0; i < USB_NUM_ENDPOINTS; i++) {
        if (endpoints[i].descriptor && (endpoints[i].descriptor->bEndpointAddress == addr)) {
            return &endpoints[i];
        }
    }
    return NULL;
}

/**
 * @brief Given a C string, fill the EP0 data buf with a USB string descriptor for that string.
 *
 * @param C string you would like to send to the USB host
 * @return the length of the string descriptor in EP0 buf
 */
uint8_t usb_prepare_string_descriptor(const char *str) {
    // 2 for bLength + bDescriptorType + strlen * 2 because string is unicode. i.e. other byte will be 0
    uint8_t bLength = 2 + (strlen(str) * 2);
    static const uint8_t bDescriptorType = 0x03;

    volatile uint8_t *buf = &ep0_buf[0];
    *buf++ = bLength;
    *buf++ = bDescriptorType;

    uint8_t c;

    do {
        c = *str++;
        *buf++ = c;
        *buf++ = 0;
    } while (c != '\0');

    return bLength;
}

/**
 * @brief Same as above, but prepares answering an OS Descriptor request with a //WinUSB usage query
 *
 * @return the length of the string descriptor in EP0 buf
 */
uint8_t usb_prepare_winusb_string_descriptor() {
    static const uint8_t length = 0x12;
    static const uint8_t type = 0x03;
    static const uint8_t msVendorCode = WINUSB_VENDOR_CODE;
    static const uint8_t pad = 0x00;

    volatile uint8_t *buf = &ep0_buf[0];

    *buf++ = length;
    *buf++ = type;

    const char *mov = "MSFT100";
    for (int i = 0; i < 7; i++) {
        *buf++ = *mov++;
        *buf++ = 0;
    }

    *buf++ = msVendorCode;
    *buf++ = pad;

    return length;
}

/**
 * @brief Take a buffer pointer located in the USB RAM and return as an offset of the RAM.
 *
 * @param buf
 * @return uint32_t
 */
inline uint32_t usb_buffer_offset(volatile uint8_t *buf) {
    return (uint32_t) buf ^ (uint32_t) usb_dpram;
}

/**
 * @brief Set up the endpoint control register for an endpoint (if applicable. Not valid for EP0).
 *
 * @param ep
 */
void usb_setup_endpoint(const usb_endpoint_configuration *ep) {
    //printf("Set up endpoint 0x%x with buffer address 0x%p\n", ep->descriptor->bEndpointAddress, ep->data_buffer);

    // EP0 doesn't have one so return if that is the case
    if (!ep->endpoint_control) {
        return;
    }

    // Get the data buffer as an offset of the USB controller's DPRAM
    uint32_t dpram_offset = usb_buffer_offset(ep->data_buffer);
    uint32_t reg = EP_CTRL_ENABLE_BITS
                   | EP_CTRL_INTERRUPT_PER_BUFFER
                   | (ep->descriptor->bmAttributes << EP_CTRL_BUFFER_TYPE_LSB)
                   | dpram_offset;
    *ep->endpoint_control = reg;
}

/**
 * @brief Set up the endpoint control register for each endpoint.
 *
 */
void usb_setup_endpoints() {
    const usb_endpoint_configuration *endpoints = dev_config.endpoints;
    for (int i = 0; i < USB_NUM_ENDPOINTS; i++) {
        if (endpoints[i].descriptor && endpoints[i].handler) {
            usb_setup_endpoint(&endpoints[i]);
        }
    }
}

/**
 * @brief Given an endpoint configuration, returns true if the endpoint
 * is transmitting data to the host (i.e. is an IN endpoint)
 *
 * @param ep, the endpoint configuration
 * @return true
 * @return false
 */
inline bool ep_is_tx(usb_endpoint_configuration *ep) {
    return ep->descriptor->bEndpointAddress & USB_DIR_IN;
}

volatile int ep0InRemainingLength = 0;
uint8_t ep0InRemainsBuffer[255];
uint8_t ep0InRemainsBufferSwap[255];

// We do buf(which can be ep0InRemainsBuffer)[i+64] -> ep0InRemainsBuffer[i] at the end of the startTransfer method

/**
 * @brief Starts a transfer on a given endpoint.
 *
 * @param ep, the endpoint configuration.
 * @param buf, the data buffer to send. Only applicable if the endpoint is TX
 * @param len, the length of the data in buf (this example limits max len to one packet - 64 bytes)
 */
void usb_start_transfer(usb_endpoint_configuration *ep, const uint8_t *buf, uint16_t len) {

    bool isEp0In = usb_get_endpoint_configuration(ep0_in_addr()) == ep;

    if (isEp0In) {
        if (len > 64) {
            ep0InRemainingLength = len - 64;
            len = 64;
        }
        else {
            ep0InRemainingLength = 0;
        }
    }

    //printf("Start transfer of len %d on ep addr 0x%x\n", len, ep->descriptor->bEndpointAddress);

    //* Custom addition
    //* For in endpoints, if the transfer hasn't gone out yet, replace the data_buffer, and don't flip the pid
    //* i.e simply "replace" buffer
    bool transferNotFired = false;
    
    // Prepare buffer control register value
    uint32_t val = len | USB_BUF_CTRL_AVAIL;

    if (ep_is_tx(ep)) {
        transferNotFired = *(ep->buffer_control) & USB_BUF_CTRL_FULL; // 1 << 15

        // Need to copy the data from the user buffer to the usb memory
        memcpy((void *) ep->data_buffer, (void *) buf, len);
        // Mark as full
        val |= USB_BUF_CTRL_FULL;
    }

    if (isEp0In && ep0InRemainingLength > 0) {
        for (int i = 0; i<ep0InRemainingLength; ++i) {
            ep0InRemainsBufferSwap[i] = buf[i+64];
        }
        for (int i = 0; i<ep0InRemainingLength; ++i) {
            ep0InRemainsBuffer[i] = ep0InRemainsBufferSwap[i];
        }
    }

    // Set pid and flip for next transfer
    val |= ep->next_pid ? USB_BUF_CTRL_DATA1_PID : USB_BUF_CTRL_DATA0_PID;
    // Incomplete transfer or transfer not fired
    if (ep0InRemainingLength>0 || !transferNotFired) ep->next_pid ^= 1u;

    *ep->buffer_control = val;
}

/**
 * @brief Send device descriptor to host
 *
 */
void usb_handle_device_descriptor(volatile usb_setup_packet *pkt) {
    const usb_device_descriptor *d = dev_config.device_descriptor;
    // EP0 in
    usb_endpoint_configuration *ep = usb_get_endpoint_configuration(ep0_in_addr());
    // Always respond with pid 1
    ep->next_pid = 1;
    log_uart0_int((int)d); log_uart0("\n");
    for (int i = 0; i<sizeof(usb_device_descriptor); i++) {
        log_uart0_int(((uint8_t*)d)[i]); log_uart0(" ");
    }
    log_uart0("\n");
    usb_start_transfer(ep, (uint8_t *) d, min(pkt->wLength, sizeof(usb_device_descriptor)));
}

/**
 * @brief Send the configuration descriptor (and potentially the configuration and endpoint descriptors) to the host.
 *
 * @param pkt, the setup packet received from the host.
 */
void usb_handle_config_descriptor(volatile usb_setup_packet *pkt) {
    uint8_t *buf = &ep0_buf[0];

    // First request will want just the config descriptor
    const usb_configuration_descriptor *d = dev_config.config_descriptor;
    memcpy((void *) buf, d, sizeof(usb_configuration_descriptor));
    buf += sizeof(usb_configuration_descriptor);

    // If we more than just the config descriptor copy it all
    if (pkt->wLength >= d->wTotalLength) {
        memcpy((void *) buf, dev_config.interface_descriptor, sizeof(usb_interface_descriptor));
        buf += sizeof(usb_interface_descriptor);

        if (useHID) {
            memcpy((void *) buf, dev_config.hid_descriptor, sizeof(usb_hid_descriptor));
            buf += sizeof(usb_hid_descriptor);
        }

        if (useXInput) {
            memcpy((void*)buf, xInputUnknownDescriptor.data(), sizeof(xInputUnknownDescriptor));
            buf += sizeof(xInputUnknownDescriptor);
        }

        const usb_endpoint_configuration *ep = dev_config.endpoints;

        // Copy all the endpoint descriptors starting from EP1
        for (uint i = 2; i < USB_NUM_ENDPOINTS; i++) {
            if (ep[i].descriptor) {
                log_uart0("Added descriptor ep#");log_uart0_int(i);log_uart0("\n");
                memcpy((void *) buf, ep[i].descriptor, sizeof(usb_endpoint_descriptor));
                buf += sizeof(usb_endpoint_descriptor);
            }
        }

    }

    // Send data
    // Get len by working out end of buffer subtract start of buffer
    uint32_t len = (uint32_t) buf - (uint32_t) &ep0_buf[0];
    log_uart0("returned by usb_handle_config_descriptor:\n");
    log_uart0_array(ep0_buf, len);
    usb_start_transfer(usb_get_endpoint_configuration(ep0_in_addr()), &ep0_buf[0], len);
}

/**
 * @brief Handle a BUS RESET from the host by setting the device address back to 0.
 */
void usb_bus_reset(void) {
    // Set address back to 0
    dev_addr = 0;
    should_set_address = false;
    usb_hw->dev_addr_ctrl = 0;
    configured = false;
}

/**
 * Redirects to proper prepare functions based on index 0 / 255 / other
 */
void usb_handle_string_descriptor(volatile usb_setup_packet *pkt) {
    uint8_t i = pkt->wValue & 0xff;
    log_uart0("string index is ");log_uart0_int(i);log_uart0("\n");
    uint8_t len = 0;

    if (i == 0) {
        len = 4;
        memcpy(&ep0_buf[0], dev_config.lang_descriptor, len);
    }
    else if (useWinUSB && i == 0xEE) {
        len = usb_prepare_winusb_string_descriptor();
    }
    else {
        // Prepare fills in ep0_buf
        len = usb_prepare_string_descriptor(dev_config.descriptor_strings[i - 1]);
    }
    log_uart0("resulting string length is ");log_uart0_int(len);log_uart0("\n");
    usb_start_transfer(usb_get_endpoint_configuration(ep0_in_addr()), &ep0_buf[0], len);
}


/**
 * Custom, handle the extended compatibility descriptor request that's expected
 * to come in after the 0xEE string descriptor response
 * Two kind of requests will end up here: those that query the header only (16 long)
 * and those that query the full thing (40 i.e 16 + 24*1)
 * They are indistinguishable except by length
 * But it's just a question of truncation - the same buffer start is to be used
 */
void usb_handle_extended_compatibility_descriptor(volatile usb_setup_packet *pkt) {
    log_uart0("usb_handle_extended_compatibility_descriptor\n");
    if (useXInput) {
        usb_start_transfer(usb_get_endpoint_configuration(ep0_in_addr()), (uint8_t *) &extendedCompatibilityIdFeatureDescriptorXInput, min(sizeof(t_ext_comp_desc), pkt->wLength));
    }
    else {
        usb_start_transfer(usb_get_endpoint_configuration(ep0_in_addr()), (uint8_t *) &extendedCompatibilityIdFeatureDescriptorWinUSB, min(sizeof(t_ext_comp_desc), pkt->wLength));
    }
}

void usb_handle_hid_report_descriptor(volatile usb_setup_packet *pkt) {
    log_uart0("usb_handle_hid_report_descriptor\n");
    usb_start_transfer(usb_get_endpoint_configuration(ep0_in_addr()), hid_report_descriptor, min(hid_report_descriptor_len, pkt->wLength));
}

/**
 * @brief Handles a SET_ADDR request from the host. The actual setting of the device address in
 * hardware is done in ep0_in_handler. This is because we have to acknowledge the request first
 * as a device with address zero.
 *
 * @param pkt, the setup packet from the host.
 */
void usb_set_device_address(volatile usb_setup_packet *pkt) {
    // Set address is a bit of a strange case because we have to send a 0 length status packet first with
    // address 0
    dev_addr = (pkt->wValue & 0xff);
    //printf("Set address %d\r\n", dev_addr);
    // Will set address in the callback phase
    should_set_address = true;
    usb_start_transfer(usb_get_endpoint_configuration(ep0_in_addr()), NULL, 0);
}

/**
 * @brief Handles a SET_CONFIGRUATION request from the host. Assumes one configuration so simply
 * sends a zero length status packet back to the host.
 *
 * @param pkt, the setup packet from the host.
 */
void usb_set_device_configuration(volatile usb_setup_packet *pkt) {
    // Only one configuration so just acknowledge the request
    //printf("Device Enumerated\r\n");
    usb_start_transfer(usb_get_endpoint_configuration(ep0_in_addr()), NULL, 0);
    configured = true;
}


void log_usb_setup_packet(volatile usb_setup_packet *pkt);
void log_usb_setup_packet(volatile usb_setup_packet *pkt) {
    log_uart0("usb_setup_packet ");
    log_uart0_int(pkt->bmRequestType); log_uart0(" ");
    log_uart0_int(pkt->bRequest); log_uart0(" ");
    log_uart0_int(pkt->wValue); log_uart0(" ");
    log_uart0_int(pkt->wIndex); log_uart0(" ");
    log_uart0_int(pkt->wLength); log_uart0(" ");
    log_uart0("\n");
}

/**
 * @brief Respond to a setup packet from the host.
 *
 */
void usb_handle_setup_packet(void) {
    volatile usb_setup_packet *pkt = (volatile usb_setup_packet *) &usb_dpram->setup_packet;

    uint8_t req_direction = pkt->bmRequestType & USB_DIR_MASK;
    uint8_t req_type = pkt->bmRequestType & USB_REQ_TYPE_TYPE_MASK;
    uint8_t req_recipient = pkt->bmRequestType & USB_REQ_TYPE_RECIPIENT_MASK;

    uint8_t req = pkt->bRequest;
    
    log_uart0("\nusb_handle_setup_packet\n");
    log_usb_setup_packet(pkt);

    // Reset PID to 1 for EP0 IN
    usb_get_endpoint_configuration(ep0_in_addr())->next_pid = 1u;

    /* Setup packet responses overrides */
    if ((useWinUSB || useXInput) &&
        req_direction == USB_DIR_IN &&
        req_type == USB_REQ_TYPE_TYPE_VENDOR &&
        req_recipient == USB_REQ_TYPE_RECIPIENT_DEVICE &&
        req == WINUSB_VENDOR_CODE &&
        pkt->wIndex == EXTENDED_COMPATIBILITY_ID) {
        log_uart0("usb_handle_compatibility_descriptor\n");
        usb_handle_extended_compatibility_descriptor(pkt);
    }
    // <XInput>
    else if ( useXInput &&
        pkt->bmRequestType == 0xc1 &&
        pkt->bRequest == 1 &&
        pkt->wValue == 0x0100 &&
        pkt->wIndex == 0x0000 &&
        pkt->wLength == 20
    ) {
        usb_start_transfer(usb_get_endpoint_configuration(ep0_in_addr()), xInputSpecificControlRequestResponse1, pkt->wLength);
    }
    else if ( useXInput &&
        pkt->bmRequestType == 0xc1 &&
        pkt->bRequest == 1 &&
        pkt->wValue == 0x0000 &&
        pkt->wIndex == 0x0000 &&
        pkt->wLength == 8
    ) {
        usb_start_transfer(usb_get_endpoint_configuration(ep0_in_addr()), xInputSpecificControlRequestResponse2, pkt->wLength);
    }
    else if ( useXInput &&
        pkt->bmRequestType == 0xc0 &&
        pkt->bRequest == 1 &&
        pkt->wValue == 0x0000 &&
        pkt->wIndex == 0x0000 &&
        pkt->wLength == 4
    ) {
        usb_start_transfer(usb_get_endpoint_configuration(ep0_in_addr()), xInputSpecificControlRequestResponse3, pkt->wLength);
    }
    // </XInput>
    else if (
        req_direction == USB_DIR_IN &&
        req_type == USB_REQ_TYPE_STANDARD &&
        req_recipient == USB_REQ_TYPE_RECIPIENT_INTERFACE &&
        req == 6 && //TODO Remove magic numbers
        (pkt->wValue == 0x2200)) {
        log_uart0("usb_handle_hid_descriptor\n");
        usb_handle_hid_report_descriptor(pkt);
    }
    else if (req_direction == USB_DIR_OUT) {
        if (req == USB_REQUEST_SET_ADDRESS) {
            log_uart0("usb_set_device_address\n");
            usb_set_device_address(pkt);
        } else if (req == USB_REQUEST_SET_CONFIGURATION) {
            log_uart0("usb_set_device_configuration\n");
            usb_set_device_configuration(pkt);
        } else {
            log_uart0("usb: other out request\n");
            //* "Acknowledge the request" ?
            usb_start_transfer(usb_get_endpoint_configuration(ep0_in_addr()), NULL, 0);
            //printf("Other OUT request (0x%x)\r\n", pkt->bRequest);
        }
    } else if (req_direction == USB_DIR_IN) {
        if (req == USB_REQUEST_GET_DESCRIPTOR) {
            uint16_t descriptor_type = pkt->wValue >> 8;

            switch (descriptor_type) {
                case USB_DT_DEVICE:
                    log_uart0("usb_handle_device_descriptor\n");
                    usb_handle_device_descriptor(pkt);
                    //printf("GET DEVICE DESCRIPTOR\r\n");
                    break;

                case USB_DT_CONFIG:
                    log_uart0("usb_handle_config_descriptor\n");
                    usb_handle_config_descriptor(pkt);
                    //printf("GET CONFIG DESCRIPTOR\r\n");
                    break;

                case USB_DT_STRING:
                    log_uart0("usb_handle_string_descriptor\n");
                    usb_handle_string_descriptor(pkt);
                    //printf("GET STRING DESCRIPTOR\r\n");
                    break;

                default:
                    log_uart0("usb: other GET_DESCRIPTOR in request\n");
                    usb_start_transfer(usb_get_endpoint_configuration(ep0_in_addr()), NULL, 0);
                    break;
            }
        } else {
            log_uart0("usb: other non-GET_DESCRIPTOR in request\n");
            usb_start_transfer(usb_get_endpoint_configuration(ep0_in_addr()), NULL, 0);
            // Unreachable
        }
    }
}

/**
 * @brief Notify an endpoint that a transfer has completed.
 *
 * @param ep, the endpoint to notify.
 */
void usb_handle_ep_buff_done(usb_endpoint_configuration *ep) {
    uint32_t buffer_control = *ep->buffer_control;
    // Get the transfer length for this endpoint
    uint16_t len = buffer_control & USB_BUF_CTRL_LEN_MASK;

    // Call that endpoints buffer done handler
    ep->handler((uint8_t *) ep->data_buffer, len);
}

/**
 * @brief Find the endpoint configuration for a specified endpoint number and
 * direction and notify it that a transfer has completed.
 *
 * @param ep_num
 * @param in
 */
void usb_handle_buff_done(uint ep_num, bool in) {
    uint8_t ep_addr = ep_num | (in ? USB_DIR_IN : 0);
    //printf("EP %d (in = %d) done\n", ep_num, in);
    for (uint i = 0; i < USB_NUM_ENDPOINTS; i++) {
        usb_endpoint_configuration *ep = &dev_config.endpoints[i];
        if (ep->descriptor && ep->handler) {
            if (ep->descriptor->bEndpointAddress == ep_addr) {
                usb_handle_ep_buff_done(ep);
                return;
            }
        }
    }
}

/**
 * @brief Handle a "buffer status" irq. This means that one or more
 * buffers have been sent / received. Notify each endpoint where this
 * is the case.
 */
void usb_handle_buff_status() {
    uint32_t buffers = usb_hw->buf_status;
    uint32_t remaining_buffers = buffers;

    uint bit = 1u;
    for (uint i = 0; remaining_buffers && i < USB_NUM_ENDPOINTS * 2; i++) {
        if (remaining_buffers & bit) {
            // clear this in advance
            usb_hw_clear->buf_status = bit;
            // IN transfer for even i, OUT transfer for odd i
            usb_handle_buff_done(i >> 1u, !(i & 1u));
            remaining_buffers &= ~bit;
        }
        bit <<= 1u;
    }
}

/**
 * @brief EP0 in transfer complete. Either finish the SET_ADDRESS process, or receive a zero
 * length status packet from the host.
 *
 * @param buf the data that was sent
 * @param len the length that was sent
 */
void ep0_in_handler(uint8_t *buf, uint16_t len) {
    log_uart0("ep0_in_handler\n");

    //
    log_uart0("trace ep alteration\n");
    const usb_endpoint_configuration *ep = dev_config.endpoints;
    log_uart0_array((uint8_t*)(ep[2].descriptor), 7);
    //

    if (ep0InRemainingLength>0) {
        log_uart0("ep0_in_handler > epRemainingLength ");
        log_uart0_int(ep0InRemainingLength);
        log_uart0("\n");
        usb_start_transfer(usb_get_endpoint_configuration(ep0_in_addr()), ep0InRemainsBuffer, ep0InRemainingLength);
    }
    else if (should_set_address) {
        log_uart0("ep0_in_handler > should_set_address\n");
        // Set actual device address in hardware
        usb_hw->dev_addr_ctrl = dev_addr;
        should_set_address = false;
    } else {
        // Receive a zero length status packet from the host on EP0 OUT
        usb_endpoint_configuration *ep = usb_get_endpoint_configuration(ep0_out_addr());
        usb_start_transfer(ep, NULL, 0);
    }
}

void ep0_out_handler(uint8_t *buf, uint16_t len) {
    
}

volatile bool ep1_in_handler_happened = true;
void ep_in_handler(uint8_t *buf, uint16_t len) {
    ep1_in_handler_happened = true;
}

void ep_out_handler(uint8_t *buf, uint16_t len) {
    if (len==5) {
        gpio_put(rumblePin, !!buf[1]); //TODO XInput support
    }
    usb_start_transfer(usb_get_endpoint_configuration(ep_out_addr()), NULL, 5);
}

/**
 * @brief USB interrupt handler
 *
 */
void my_usb_isr(void) {

    /*#ifdef USE_UART0
    uart_puts(uart0, "isr_usbctrl\n");
    #endif*/

    // USB interrupt handler
    uint32_t status = usb_hw->ints;
    uint32_t handled = 0;

    // Setup packet received
    if (status & USB_INTS_SETUP_REQ_BITS) {
        handled |= USB_INTS_SETUP_REQ_BITS;
        usb_hw_clear->sie_status = USB_SIE_STATUS_SETUP_REC_BITS;
        usb_handle_setup_packet();
    }

    // Buffer status, one or more buffers have completed
    if (status & USB_INTS_BUFF_STATUS_BITS) {
        handled |= USB_INTS_BUFF_STATUS_BITS;
        usb_handle_buff_status();
    }

    // Bus is reset
    if (status & USB_INTS_BUS_RESET_BITS) {
        //printf("BUS RESET\n");
        handled |= USB_INTS_BUS_RESET_BITS;
        usb_hw_clear->sie_status = USB_SIE_STATUS_BUS_RESET_BITS;
        usb_bus_reset();
    }

    if (status ^ handled) {
        panic("Unhandled IRQ 0x%x\n", (uint) (status ^ handled));
    }
}

/**
 * @brief Set up the USB controller in device mode, clearing any previous state.
 *
 */
void usb_device_init() {
    //TODO Look into why there's so much delay for the device to be recognized when plugged in

    // Reset usb controller
    reset_block(RESETS_RESET_USBCTRL_BITS);
    unreset_block_wait(RESETS_RESET_USBCTRL_BITS);

    // Clear any previous state in dpram just in case
    memset(usb_dpram, 0, sizeof(*usb_dpram)); // <1>

    // Enable USB interrupt at processor
    irq_set_exclusive_handler(5, my_usb_isr);
    irq_set_enabled(USBCTRL_IRQ, true);

    // Mux the controller to the onboard usb phy
    usb_hw->muxing = USB_USB_MUXING_TO_PHY_BITS | USB_USB_MUXING_SOFTCON_BITS;

    // Force VBUS detect so the device thinks it is plugged into a host
    usb_hw->pwr = USB_USB_PWR_VBUS_DETECT_BITS | USB_USB_PWR_VBUS_DETECT_OVERRIDE_EN_BITS;

    // Enable the USB controller in device mode.
    usb_hw->main_ctrl = USB_MAIN_CTRL_CONTROLLER_EN_BITS;

    // Enable an interrupt per EP0 transaction
    usb_hw->sie_ctrl = USB_SIE_CTRL_EP0_INT_1BUF_BITS; // <2>

    // Enable interrupts for when a buffer is done, when the bus is reset,
    // and when a setup packet is received
    usb_hw->inte = USB_INTS_BUFF_STATUS_BITS |
                   USB_INTS_BUS_RESET_BITS |
                   USB_INTS_SETUP_REQ_BITS;

    // Set up endpoints (endpoint control registers)
    // described by device configuration
    usb_setup_endpoints();

    // Present full speed device by enabling pull up on DP
    usb_hw_set->sie_ctrl = USB_SIE_CTRL_PULLUP_EN_BITS;
}




void await_time32us(uint32_t target) {
    while ( (time_us_32() - target) & (1 << 31) );
}

/* INITIALIZATION */

namespace CommunicationProtocols
{
namespace USB
{

void inner_enterMode(ConfigurationNoFunc config, int headroomUs) {
    /* Initialize structures */

    // Always same size, always usb_dt, always interrupt, always bInterval1, the variables here are whether the 2nd endpoint is id 1 or 2, and the max packet sizes
    ep_in = {
        .bLength          = sizeof(usb_endpoint_descriptor),
        .bDescriptorType  = USB_DT_ENDPOINT,
        .bEndpointAddress = 0x81, // In endpoint (device to host), always ID 1
        .bmAttributes     = USB_TRANSFER_TYPE_INTERRUPT,
        .wMaxPacketSize   = config.inEpMaxPacketSize,
        .bInterval        = 1 // 1000 Hz reporting
    };
    ep_out = {
        .bLength          = sizeof(usb_endpoint_descriptor),
        .bDescriptorType  = USB_DT_ENDPOINT,
        .bEndpointAddress = config.epOutId, // Out endpoint (host to device), ID 1 or 2
        .bmAttributes     = USB_TRANSFER_TYPE_INTERRUPT,
        .wMaxPacketSize   = config.outEpMaxPacketSize,
        .bInterval        = 1 // 1000 Hz rumble
    };
    epOutId = config.epOutId;

    // The descriptor string is a usb_configurations parameter
    descriptor_strings = config.descriptorStrings;
    descriptor_strings_len = config.descriptorStringsLen;

    // Parameters here are the bcdHID, the HID descriptor, and the HID descriptor length, plus whether we're using HID (could be inferred, TODO later)
    useHID = config.hid;
    useXInput = config.xinput;
    hid_descriptor = {
        .bLength = 0x09,
        .bDescriptorType = 0x21,
        .bcdHID = config.bcdHID,
        .bCountryCode = 0x00,
        .bNumDescriptors = 0x01,
        .bDescriptorType2 = 0x22,
        .bDescriptorLength = config.hidReportDescriptorLen
    };

    useWinUSB = config.useWinUSB;

    // Variables here are the VID, the PID and the bcdDevice
    device_descriptor = {
        .bLength         = sizeof(usb_device_descriptor),
        .bDescriptorType = USB_DT_DEVICE,
        .bcdUSB          = 0x0200, //* USB 2.0 device (actually 1.1 but 2.0 identification necessary for WCID)
        .bDeviceClass    = (uint8_t)(config.xinput ? 0xFF : 0),      // Specified in interface descriptor
        .bDeviceSubClass = (uint8_t)(config.xinput ? 0xFF : 0),      // No subclass
        .bDeviceProtocol = (uint8_t)(config.xinput ? 0xFF : 0),      // No protocol
        .bMaxPacketSize0 = 64,     // Max packet size for ep0
        .idVendor        = config.VID,       // USB vendor ID
        .idProduct       = config.PID,       // USB product ID
        .bcdDevice       = config.bcdDevice, // Device revision number
        .iManufacturer   = 1,      // Manufacturer string index
        .iProduct        = 2,      // Product string index
        .iSerialNumber   = 3,      // Serial number index (used by some softwares as the device name... somehow)
        .bNumConfigurations = 1    // One configuration
    };

    // The only variable here is the bInterfaceClass
    interface_descriptor = {
        .bLength            = sizeof(usb_interface_descriptor),
        .bDescriptorType    = USB_DT_INTERFACE,
        .bInterfaceNumber   = 0,
        .bAlternateSetting  = 0,
        .bNumEndpoints      = 2,    // Always 2 endpoints
        .bInterfaceClass    = (uint8_t)(config.hid ? (uint8_t)0x03 : (uint8_t)0xff), // 3 = HID class, 255 = Vendor class
        .bInterfaceSubClass = (uint8_t)(config.xinput ? 0x5du : 0u),
        .bInterfaceProtocol = (uint8_t)(config.xinput ? 0x01u : 0u),
        .iInterface         = 0
    };

    // The variable here is the size, that depends on whether we have a HID descriptor or not
    config_descriptor = {
        .bLength         = sizeof(usb_configuration_descriptor),
        .bDescriptorType = USB_DT_CONFIG,
        .wTotalLength    = (uint16_t)(sizeof(usb_configuration_descriptor) +
                            sizeof(usb_interface_descriptor) +
                            (config.hid ? sizeof(usb_hid_descriptor) : 0) +
                            (config.xinput ? sizeof(xInputUnknownDescriptor) : 0) +
                            2 * sizeof(usb_endpoint_descriptor)),
        .bNumInterfaces  = 1,
        .bConfigurationValue = 1, // Configuration 1
        .iConfiguration = 0,      // No string
        .bmAttributes = (uint8_t)(config.xinput ? 0x80 : 0xe0), // xinput -> attributes: bus-powered device, else self powered-device, remote wakeup
        .bMaxPower = 0xFA         // 500ma
    };

    dev_config = {
        .device_descriptor = &device_descriptor,
        .interface_descriptor = &interface_descriptor,
        .config_descriptor = &config_descriptor,
        .hid_descriptor = &hid_descriptor,
        .lang_descriptor = lang_descriptor,
        .descriptor_strings = descriptor_strings,
        .endpoints = {
            {
                .descriptor = &ep0_out,
                .handler = &ep0_out_handler,
                .endpoint_control = NULL, // NA for EP0
                .buffer_control = &usb_dpram->ep_buf_ctrl[0].out,
                // EP0 in and out share a data buffer
                .data_buffer = &usb_dpram->ep0_buf_a[0]
            },
            {
                .descriptor = &ep0_in,
                .handler = &ep0_in_handler,
                .endpoint_control = NULL, // NA for EP0,
                .buffer_control = &usb_dpram->ep_buf_ctrl[0].in,
                // EP0 in and out share a data buffer
                .data_buffer = &usb_dpram->ep0_buf_a[0]
            },
            {
                .descriptor = &ep_in,
                .handler = &ep_in_handler,
                // EP1 starts at offset 0 for endpoint control
                .endpoint_control = &usb_dpram->ep_ctrl[0].in,
                .buffer_control = &usb_dpram->ep_buf_ctrl[1].in,
                // First free EPX buffer
                .data_buffer = &usb_dpram->epx_data[0 * 64]
            },
            {
                .descriptor = &ep_out,
                .handler = &ep_out_handler,
                .endpoint_control = &usb_dpram->ep_ctrl[config.epOutId-1].out,
                .buffer_control = &usb_dpram->ep_buf_ctrl[config.epOutId].out,
                // Second free EPX buffer
                .data_buffer = &usb_dpram->epx_data[1 * 64]
            }
        }
    };

    hid_report_descriptor = config.hidReportDescriptor;
    hid_report_descriptor_len = config.hidReportDescriptorLen;

    /* Initialize USB mode */

    usb_device_init();
    while (!configured);

    /* Start communications */

    usb_start_transfer(usb_get_endpoint_configuration(ep_out_addr()), nullptr, config.outEpMaxPacketSize); // Get ready to receive something and do absolutely nothing with it

    /* Setup rumble */

    gpio_init(rumblePin);
    gpio_set_dir(rumblePin, GPIO_OUT);
}

void enterMode(Configuration config, int headroomUs) {
    inner_enterMode(config.configNoFunc, headroomUs);

    uint32_t target;

    int counter = 0;
    uint32_t accu20 = 0;
    uint32_t accu70 = 0;
    uint32_t prevHandlerHappenedTimestamp;
    uint32_t currentHandlerHappenedTimestamp;

    int choice = 0; // 1 PC 2 Switch

    while (1) {
        while (!ep1_in_handler_happened) {
            if ((int32_t)(time_us_32() - target - headroomUs) > 150) {
                target += 1000;
                goto skip_wait_oneFunc;
            }
        }
        ep1_in_handler_happened = false;
        target = time_us_32() + 1000 - headroomUs;

        skip_wait_oneFunc:
 
         // Wait until n us before 1000-x us after completion of the previous ep1 transfer
        await_time32us(target);

        // Note: for wup-028 mode, actuate func call always takes 8.25 to 9.6us        
        config.reportActuationFunc();

        usb_start_transfer(usb_get_endpoint_configuration(ep_in_addr()), config.configNoFunc.hidReportPtr, config.configNoFunc.inEpActualPacketSize);
    }
}

void enterMode(ConfigurationNoFunc configNoFunc, FuncsDOP funcsDOP, int headroomUs) {
    inner_enterMode(configNoFunc, headroomUs);

    uint32_t target;

    int counter = 0;
    uint32_t accu20 = 0;
    uint32_t accu70 = 0;
    uint32_t prevHandlerHappenedTimestamp;
    uint32_t currentHandlerHappenedTimestamp;

    int choice = 0; // 1 PC 2 Switch

    while (1) {

        while (!ep1_in_handler_happened) {
            if (counter == 70 && (int32_t)(time_us_32() - target - headroomUs) > 150) {
                target += 1000;
                goto skip_wait_twoFuncs;
            }
        }
        ep1_in_handler_happened = false;
        target = time_us_32() + 1000 - headroomUs;

        skip_wait_twoFuncs:
 
         // Wait until n us before 1000-x us after completion of the previous ep1 transfer
        await_time32us(target);

        {
            prevHandlerHappenedTimestamp = currentHandlerHappenedTimestamp;
            currentHandlerHappenedTimestamp = systick_hw->cvr;
            uint32_t diff = (currentHandlerHappenedTimestamp > prevHandlerHappenedTimestamp ? 0x01000000 : 0)
                + (prevHandlerHappenedTimestamp - currentHandlerHappenedTimestamp); // cvr goes down, if underflow (prev=0 -> current=0x01000000), compensate
            
            if (counter < 70) {
                counter++;
                if (counter <= 20) {
                    accu20 += diff;
                }
                if (counter <= 70) {
                    accu70 += diff;
                }
                if (counter == 70) {
                    choice = ((accu70 - accu20)/50. > 5*1'000*us) ? 2 : 1;
                }
            }
        }

        // Wait until n us before 1000-x us after completion of the previous ep1 transfer
        //target = systick_hw->cvr - (1000-headroomUs)*us;
        //while ((target - systick_hw->cvr) & 0x00800000);

        // Note: for wup-028 mode, actuate func call always takes 8.25 to 9.6us        
        if (choice == 1) {
            funcsDOP.reportActuationFuncPC();
        }
        else if (choice == 2) {
            funcsDOP.reportActuationFuncSwitch();
        }

        usb_start_transfer(usb_get_endpoint_configuration(ep_in_addr()), configNoFunc.hidReportPtr, configNoFunc.inEpActualPacketSize);
    }
}

}
}