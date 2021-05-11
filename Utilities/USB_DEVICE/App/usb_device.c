/**
  ******************************************************************************
  * @file           : usb_device.c
  * @brief          : This file implements the USB Device
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_msc.h"
#include "usbd_storage_if.h"
#include "usbd_composite.h"
#include "usbd_cdca_if.h"
#include "usbd_cdca.h"

/* USB Device Core handle declaration. */
USBD_HandleTypeDef hUsbDeviceFS;
void Error_Handler(void);

#define MSC_IDX                  0x0		
#define CDCA_IDX                 0x1		

USBD_ClassTypeDef* handles[2];

/**
  * Init USB device Library, add supported class and start the library
  * @retval None
  */
void MX_USB_DEVICE_Init(void)
{
	handles[MSC_IDX] =  &USBD_MSC;	
	handles[CDCA_IDX] = &USBD_CDCA;
	
	// Base Descriptor
	USB_ConfigDescType base_desc = {
    /*Configuration Descriptor*/
		0x09,
		/* bLength: Configuration Descriptor size */
		USB_DESC_TYPE_CONFIGURATION,
		/* bDescriptorType: Configuration */
		0x00,
		/* wTotalLength:no of returned bytes. Is set later by composite */
		0x03,
		/* bNumInterfaces */
		0x01,
		/* bConfigurationValue: Configuration value */
		0x02,
		/* iConfiguration: Index of string descriptor describing the configuration */
		0xC0,
		/* bmAttributes: self powered */
		0x32,
		/* MaxPower 100 mA */
    /* 09 bytes */
	};	
	
	/* Init Device Library, add supported class and start the library. */
	if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK)
	{
		Error_Handler();
	}
	// Add descriptors and class functions to composite device
	USBD_Composite_Set_Classes(handles, 2, &base_desc);
	// Define endpoints
	//MSC
	USBD_Composite_EPIN_To_Class(MSC_EPIN_ADDR, MSC_IDX);
	USBD_Composite_EPOUT_To_Class(MSC_EPOUT_ADDR, MSC_IDX);
	USBD_Composite_InterfaceToClass(MSC_INTERFACE_IDX, MSC_IDX);
	//CDC
	USBD_Composite_EPIN_To_Class(CDCA_IN_EP, CDCA_IDX);
	USBD_Composite_EPOUT_To_Class(CDCA_OUT_EP, CDCA_IDX);
	USBD_Composite_EPIN_To_Class(CDCA_CMD_EP, CDCA_IDX);

	USBD_Composite_InterfaceToClass(CDCA_CMD_INTERFACE_IDX, CDCA_IDX);
	USBD_Composite_InterfaceToClass(CDCA_DATA_INTERFACE_IDX, CDCA_IDX);

	if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_Composite) != USBD_OK)
	{
		Error_Handler();
	}

	if (USBD_CDCA_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_cdca_FS) != USBD_OK)
	{
		Error_Handler();
	}
	
	if (USBD_MSC_RegisterStorage(&hUsbDeviceFS, &USBD_Storage_Interface_fops_FS) != USBD_OK)
	{
		Error_Handler();
	}

	if (USBD_Start(&hUsbDeviceFS) != USBD_OK)
	{
		Error_Handler();
	}

	HAL_PWREx_EnableUSBVoltageDetector();
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
