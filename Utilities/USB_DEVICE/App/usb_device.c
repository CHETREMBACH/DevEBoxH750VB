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
#include "usbd_audio.h"
#include "usbd_desc.h"
#include "usbd_audio_if.h"

USBD_HandleTypeDef USBD_Device;

/**
  * Init USB device Library, add supported class and start the library
  * @retval None
  */
void MX_USB_DEVICE_Init(void)
{
	HAL_PWREx_EnableUSBVoltageDetector();
  
	/* Init Device Library */
	USBD_Init(&USBD_Device, &AUDIO_Desc, 0);

	/* Add Supported Class */
	USBD_RegisterClass(&USBD_Device, USBD_AUDIO_CLASS);

	/* Add Interface callbacks for AUDIO Class */
	USBD_AUDIO_RegisterInterface(&USBD_Device, &USBD_AUDIO_fops);

	/* Start Device Process */
	USBD_Start(&USBD_Device);
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
