/**
  ******************************************************************************
  * @file    usbd_cdcc.h
  * @author  MCD Application Team
  * @brief   header file for the usbd_cdc.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_CDCC_H
#define __USB_CDCC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"
#include  "pin_dbg.h"
#include  "usbd_cdc.h"
	
/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */

/** @defgroup usbd_cdc
  * @brief This file is the Header file for usbd_cdc.c
  * @{
  */

/** @defgroup USBD_CDCA_Exported_Defines
  * @{
  */

#define CDCC_CMD_INTERFACE_IDX                       0x5	// Index of CDCA_CMD interface	
#define CDCC_DATA_INTERFACE_IDX                      0x6	// Index of CDCA_DATA interface

#define CDCC_CMD_EP                                  0x87U  /* EP2 for CDC commands */
#define CDCC_IN_EP                                   0x86U  /* EP1 for data IN */
#define CDCC_OUT_EP                                  0x06U  /* EP1 for data OUT */


extern USBD_ClassTypeDef USBD_CDCC;
#define USBD_CDCC_CLASS &USBD_CDCC

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
uint8_t USBD_CDCC_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_CDC_ItfTypeDef *fops);
uint8_t USBD_CDCC_SetTxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff, uint32_t length);
uint8_t USBD_CDCC_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff);
uint8_t USBD_CDCC_ReceivePacket(USBD_HandleTypeDef *pdev);
uint8_t USBD_CDCC_TransmitPacket(USBD_HandleTypeDef *pdev);
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif  /* __USB_CDCC_H */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/