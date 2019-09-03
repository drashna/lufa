/*
             LUFA Library
     Copyright (C) Dean Camera, 2019.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2019  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  USB Device Descriptors, for library use when in USB device mode. Descriptors are special
 *  computer-readable structures which the host requests upon device enumeration, to determine
 *  the device's capabilities and functions.
 */

#include "Descriptors.h"

#define WEBUSB_ID 0x01
#define MS_OS_ID 0x02

const uint8_t PROGMEM MS_OS_Descriptor[] =
{
	MS_OS_DESCRIPTOR_SET(
		MS_OS_CONFIG_SUBSET_HEADER(0x00, // Config 0
			MS_OS_FUNCTION_SUBSET_HEADER(INTERFACE_ID_Vendor, // Interface ID
				MS_OS_COMPAT_ID_WINUSB
			)
		)
	)
};

const uint8_t PROGMEM WebUSBAllowedOrigins[] = {
	WEBUSB_ALLOWED_ORIGINS_HEADER(1, // 1 config header present
		WEBUSB_CONFIG_SUBSET_HEADER(0x00, 1, // Config 0, 1 function header
			// Config interface accessible from the web, 2 valid URLs
			WEBUSB_FUNCTION_SUBSET_HEADER(INTERFACE_ID_Vendor, URL_ID_Config, URL_ID_Localhost)
		)
	)
};

const uint8_t PROGMEM BOSDescriptor[] =
{
	BOS_DESCRIPTOR(2, // 2 capability descriptors in use
		WEBUSB_CAPABILITY_DESCRIPTOR(WEBUSB_ID, URL_ID_Config), // Vendor request ID, URL ID to take the user to
		// Required to force WinUSB driver for driverless WebUSB compatibility
		MS_OS_20_CAPABILITY_DESCRIPTOR(MS_OS_ID, sizeof(MS_OS_Descriptor)) // Vendor request ID, Descriptor set length
	)
};

/** Device descriptor structure. This descriptor, located in FLASH memory, describes the overall
 *  device characteristics, including the supported USB version, control endpoint size and the
 *  number of device configurations. The descriptor is read out by the USB host when the enumeration
 *  process begins.
 */
const USB_Descriptor_Device_t PROGMEM DeviceDescriptor =
{
	.Header                 = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},

	.USBSpecification       = VERSION_BCD(1,1,0),
	.Class                  = USB_CSCP_NoDeviceClass,
	.SubClass               = USB_CSCP_NoDeviceSubclass,
	.Protocol               = USB_CSCP_NoDeviceProtocol,

	.Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,

	.VendorID               = 0x03EB,
	.ProductID              = 0x206C,
	.ReleaseNumber          = VERSION_BCD(0,0,1),

	.ManufacturerStrIndex   = STRING_ID_Manufacturer,
	.ProductStrIndex        = STRING_ID_Product,
	.SerialNumStrIndex      = USE_INTERNAL_SERIAL,

	.NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

const USB_Descriptor_URL_t PROGMEM ConfigURL = URL_STRING_DESCRIPTOR(URL_HTTP, "www.fourwalledcubicle.com/LUFA.php");
const USB_Descriptor_URL_t PROGMEM LocalhostURL = URL_STRING_DESCRIPTOR(URL_HTTP, "localhost");

void USB_Process_BOS(void) {
	const void* Address = NULL;
	uint16_t    Size    = 0;
	
	if(!(Endpoint_IsSETUPReceived()) ||
		USB_ControlRequest.bmRequestType != (REQDIR_DEVICETOHOST | REQTYPE_VENDOR | REQREC_DEVICE)) {
		return;
	}
	switch(USB_ControlRequest.bRequest) {
		case WEBUSB_ID:
			switch(USB_ControlRequest.wIndex) {
				case WEBUSB_REQUEST_GET_ALLOWED_ORIGINS:
					Address = &WebUSBAllowedOrigins;
					Size = sizeof(WebUSBAllowedOrigins);
					break;
				case WEBUSB_REQUEST_GET_URL:
					switch(USB_ControlRequest.wValue) {
						case URL_ID_Localhost:
							Address = &LocalhostURL;
							Size = pgm_read_byte(&LocalhostURL.Header.Size);
							break;
						case URL_ID_Config:
							Address = &ConfigURL;
							Size = pgm_read_byte(&ConfigURL.Header.Size);
							break;
					}
					break;
			}
			break;
		case MS_OS_ID:
			if(USB_ControlRequest.wIndex == MS_OS_20_DESCRIPTOR_INDEX) {
				Address = &MS_OS_Descriptor;
				Size    = sizeof(MS_OS_Descriptor);
			}
			break;
		default:
			return;
	}
	if(Address != NULL) {
		Endpoint_SelectEndpoint(ENDPOINT_CONTROLEP);

		Endpoint_ClearSETUP();

		Endpoint_Write_Control_PStream_LE(Address, Size);
		Endpoint_ClearOUT();
	}
}

/** Configuration descriptor structure. This descriptor, located in FLASH memory, describes the usage
 *  of the device in one of its supported configurations, including information about any device interfaces
 *  and endpoints. The descriptor is read out by the USB host during the enumeration process when selecting
 *  a configuration so that the host may correctly communicate with the USB device.
 */
const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor =
{
	.Config =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},

			.TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
			.TotalInterfaces        = 1,

			.ConfigurationNumber    = 1,
			.ConfigurationStrIndex  = NO_DESCRIPTOR,

			.ConfigAttributes       = USB_CONFIG_ATTR_RESERVED,

			.MaxPowerConsumption    = USB_CONFIG_POWER_MA(100)
		},

	.Vendor_Interface =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

			.InterfaceNumber        = INTERFACE_ID_Vendor,
			.AlternateSetting       = 0,

			.TotalEndpoints         = 2,

			.Class                  = 0xFF,
			.SubClass               = 0xFF,
			.Protocol               = 0xFF,

			.InterfaceStrIndex      = NO_DESCRIPTOR
		},

	.Vendor_DataInEndpoint =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

			.EndpointAddress        = VENDOR_IN_EPADDR,
			.Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
			.EndpointSize           = VENDOR_IO_EPSIZE,
			.PollingIntervalMS      = 0x05
		},

	.Vendor_DataOutEndpoint =
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

			.EndpointAddress        = VENDOR_OUT_EPADDR,
			.Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
			.EndpointSize           = VENDOR_IO_EPSIZE,
			.PollingIntervalMS      = 0x05
		}
};

/** Language descriptor structure. This descriptor, located in FLASH memory, is returned when the host requests
 *  the string descriptor with index 0 (the first index). It is actually an array of 16-bit integers, which indicate
 *  via the language ID table available at USB.org what languages the device supports for its string descriptors.
 */
const USB_Descriptor_String_t PROGMEM LanguageString = USB_STRING_DESCRIPTOR_ARRAY(LANGUAGE_ID_ENG);

/** Manufacturer descriptor string. This is a Unicode string containing the manufacturer's details in human readable
 *  form, and is read out upon request by the host when the appropriate string ID is requested, listed in the Device
 *  Descriptor.
 */
const USB_Descriptor_String_t PROGMEM ManufacturerString = USB_STRING_DESCRIPTOR(L"LUFA Library");

/** Product descriptor string. This is a Unicode string containing the product's details in human readable form,
 *  and is read out upon request by the host when the appropriate string ID is requested, listed in the Device
 *  Descriptor.
 */
const USB_Descriptor_String_t PROGMEM ProductString = USB_STRING_DESCRIPTOR(L"LUFA Bulk Vendor Demo");

/** This function is called by the library when in device mode, and must be overridden (see library "USB Descriptors"
 *  documentation) by the application code so that the address and size of a requested descriptor can be given
 *  to the USB library. When the device receives a Get Descriptor request on the control endpoint, this function
 *  is called so that the descriptor details can be passed back and the appropriate descriptor sent back to the
 *  USB host.
 */
uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint16_t wIndex,
                                    const void** const DescriptorAddress)
{
	const uint8_t  DescriptorType   = (wValue >> 8);
	const uint8_t  DescriptorNumber = (wValue & 0xFF);

	const void* Address = NULL;
	uint16_t    Size    = NO_DESCRIPTOR;

	switch (DescriptorType)
	{
		case DTYPE_Device:
			Address = &DeviceDescriptor;
			Size    = sizeof(USB_Descriptor_Device_t);
			break;
		case DTYPE_Configuration:
			Address = &ConfigurationDescriptor;
			Size    = sizeof(USB_Descriptor_Configuration_t);
			break;
		case DTYPE_BOS:
			Address = &BOSDescriptor;
			Size    = sizeof(BOSDescriptor);
			break;
		case DTYPE_String:
			switch (DescriptorNumber)
			{
				case STRING_ID_Language:
					Address = &LanguageString;
					Size    = pgm_read_byte(&LanguageString.Header.Size);
					break;
				case STRING_ID_Manufacturer:
					Address = &ManufacturerString;
					Size    = pgm_read_byte(&ManufacturerString.Header.Size);
					break;
				case STRING_ID_Product:
					Address = &ProductString;
					Size    = pgm_read_byte(&ProductString.Header.Size);
					break;
			}

			break;
	}

	*DescriptorAddress = Address;
	return Size;
}

