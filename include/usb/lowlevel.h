/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef USB__LOWLEVEL_H
#define USB__LOWLEVEL_H

#include "usb/common.h"



// Struct in which we keep the endpoint configuration
typedef void (*usb_ep_handler)(uint8_t *buf, uint16_t len);
struct usb_endpoint_configuration {
    const struct usb_endpoint_descriptor *descriptor;
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
    const struct usb_device_descriptor *device_descriptor;
    const struct usb_interface_descriptor *interface_descriptor;
    const struct usb_configuration_descriptor *config_descriptor;
    const char *lang_descriptor;
    const char **descriptor_strings;
    // USB num endpoints is 16
    struct usb_endpoint_configuration endpoints[USB_NUM_ENDPOINTS];
};



#define EP0_IN_ADDR  (USB_DIR_IN  | 0)
#define EP0_OUT_ADDR (USB_DIR_OUT | 0)
#define EP1_IN_ADDR  (USB_DIR_IN  | 1)
#define EP2_OUT_ADDR (USB_DIR_OUT | 2)

/* Mandatory control endpoint */
static const struct usb_endpoint_descriptor ep0_out = {
        .bLength          = sizeof(struct usb_endpoint_descriptor),
        .bDescriptorType  = USB_DT_ENDPOINT,
        .bEndpointAddress = EP0_OUT_ADDR, // EP0, OUT from host (device to host)
        .bmAttributes     = USB_TRANSFER_TYPE_CONTROL,
        .wMaxPacketSize   = 64,
        .bInterval        = 0
};
static const struct usb_endpoint_descriptor ep0_in = {
        .bLength          = sizeof(struct usb_endpoint_descriptor),
        .bDescriptorType  = USB_DT_ENDPOINT,
        .bEndpointAddress = EP0_IN_ADDR, // EP0, IN from host (host to device)
        .bmAttributes     = USB_TRANSFER_TYPE_CONTROL,
        .wMaxPacketSize   = 64,
        .bInterval        = 0
};

/* Standard GCC to USB adapter endpoints */
static const struct usb_endpoint_descriptor ep1_in = {
        .bLength          = sizeof(struct usb_endpoint_descriptor),
        .bDescriptorType  = USB_DT_ENDPOINT,
        .bEndpointAddress = EP1_IN_ADDR, // EP1, IN from host (device to host)
        .bmAttributes     = USB_TRANSFER_TYPE_INTERRUPT,
        .wMaxPacketSize   = 37,
        .bInterval        = 1 // 1000 Hz reporting
};
static const struct usb_endpoint_descriptor ep2_out = {
        .bLength          = sizeof(struct usb_endpoint_descriptor),
        .bDescriptorType  = USB_DT_ENDPOINT,
        .bEndpointAddress = EP2_OUT_ADDR, // EP2, OUT from host (host to device)
        .bmAttributes     = USB_TRANSFER_TYPE_INTERRUPT,
        .wMaxPacketSize   = 5,
        .bInterval        = 1 // 1000 Hz rumble
};



// Descriptors
static const struct usb_device_descriptor device_descriptor = {
        .bLength         = sizeof(struct usb_device_descriptor),
        .bDescriptorType = USB_DT_DEVICE,
        .bcdUSB          = 0x0200, // USB 2.0 device
        .bDeviceClass    = 0,      // Specified in interface descriptor
        .bDeviceSubClass = 0,      // No subclass
        .bDeviceProtocol = 0,      // No protocol
        .bMaxPacketSize0 = 64,     // Max packet size for ep0
        .idVendor        = 0xA57E, // Your vendor id
        .idProduct       = 0xE019, // Your product ID
        .bcdDevice       = 0,      // No device revision number
        .iManufacturer   = 1,      // Manufacturer string index
        .iProduct        = 2,      // Product string index
        .iSerialNumber   = 3,      // Serial number index (used by some softwares as the device name... somehow)
        .bNumConfigurations = 1    // One configuration
};

static const struct usb_interface_descriptor interface_descriptor = {
        .bLength            = sizeof(struct usb_interface_descriptor),
        .bDescriptorType    = USB_DT_INTERFACE,
        .bInterfaceNumber   = 0,
        .bAlternateSetting  = 0,
        .bNumEndpoints      = 2,    // Interface has 2 endpoints
        .bInterfaceClass    = 0xff, // Vendor specific endpoint
        .bInterfaceSubClass = 0,
        .bInterfaceProtocol = 0,
        .iInterface         = 0
};

static const struct usb_configuration_descriptor config_descriptor = {
        .bLength         = sizeof(struct usb_configuration_descriptor),
        .bDescriptorType = USB_DT_CONFIG,
        .wTotalLength    = (sizeof(config_descriptor) +
                            sizeof(interface_descriptor) +
                            sizeof(ep1_in) +
                            sizeof(ep2_out)),
        .bNumInterfaces  = 1,
        .bConfigurationValue = 1, // Configuration 1
        .iConfiguration = 0,      // No string
        .bmAttributes = 0xc0,     // attributes: self powered, no remote wakeup
        .bMaxPower = 0x32         // 100ma
};



/* WinUSB descriptors */
// From https://www.silabs.com/community/mcu/32-bit/forum.topic.html/using_microsoft_osd-lMCD

#define EXTENDED_COMPATIBILITY_ID 4
#define EXT_COMP_DESC_SIZE sizeof(struct t_ext_comp_desc)
#define NUM_INTERFACES_WITH_EXTENDED_COMPATIBILITY 1

struct extended_compatibility_interface_descriptor {
	uint8_t firstInterfaceNumber; /**< Interface number for which an extended compatibility feature descriptor is defined */
	uint8_t reserved1;
	char compatibleID[8]; /**< String describing the compatible id */
	char subCompatibleID[8]; /**< String describing the sub compatible id */
	uint8_t reserved2[6];
};

const struct t_ext_comp_desc{
	uint32_t length; /**< Size of this struct = 16 + bCount*24 */
	uint16_t bcdVersion; /**< 1.00 -> 0x0100 */
	uint16_t index;	/**< Command index - 0x04 for extended compatibility id */
	uint8_t count;		/**< Number of interfaces for which an extended compatibility feature descriptor is defined */
	uint8_t reserved1[7];
	struct extended_compatibility_interface_descriptor ecid[NUM_INTERFACES_WITH_EXTENDED_COMPATIBILITY];

} __attribute__((packed)) extendedCompatibilityIdFeatureDescriptor =
{
	0x00000028, //LSB 40 (40 0 0 0)
	0x0100, // LSB BCD (1 0)
	0x0004, // LSB 4 ( 0 4 ) EXTENDED_COMPATIBILITY_ID,
	1, //NUM_INTERFACES_WITH_EXTENDED_COMPATIBILITY,
	{0, 0, 0, 0, 0, 0, 0},

	{//Start of array initialization
		{//Start of struct initialization
		        0, //YOUR_INTERFACE_NUMBER_1
			0x01, // Reserved
			{0x57, 0x49, 0x4E, 0x55, 0x53, 0x42, 0x00, 0x00 },//"WINUSB\0\0", /** Other IDs define other compatible functions */
			{0, 0, 0, 0, 0, 0, 0, 0}, /**< Some compatible IDs require this “sub compatible ID”  */
			{0, 0, 0, 0, 0, 0},
		},
		/** You can add more interfaces (or functions with multiple interfaces)
		 * which support extended compatibility here.
		 */
	}
};


static const char lang_descriptor[] = {
        4,         // bLength
        0x03,      // bDescriptorType == String Descriptor
        0x09, 0x04 // language id = us english
};

static const char *descriptor_strings[] = {
        "SSBM Arte",
        "Pico Frame1/B0XX",
        "Pico Frame1/B0XX"
};

#define WINUSB_VENDOR_CODE 0xAF

int usb_lowlevel_init(void);

#endif