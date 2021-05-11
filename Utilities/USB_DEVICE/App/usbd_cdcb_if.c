/**
  ******************************************************************************
  * @file           : usbd_cdcb_if.c
  * @version        : v1.0_Cube
  * @brief          : Usb device for Virtual Com Port.
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
#include "usbd_cdcb_if.h"
#include "usbd_cdcb.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_CDCA_IF
  * @{
  */

/** @defgroup USBD_CDCA_IF_Private_Variables USBD_CDCA_IF_Private_Variables
  * @brief Private variables.
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t RxBufferCDCB[APP_RX_DATA_CDCB_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t TxBufferCDCB[APP_TX_DATA_CDCB_SIZE];

/**
  * @}
  */

/** @defgroup USBD_CDCA_IF_Exported_Variables USBD_CDCA_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */
extern USBD_HandleTypeDef hUsbDeviceFS;

/**
  * @}
  */

/** @defgroup USBD_CDCA_IF_Private_FunctionPrototypes USBD_CDCA_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CDCB_Init_FS(void);
static int8_t CDCB_DeInit_FS(void);
static int8_t CDCB_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDCB_Receive_FS(uint8_t* pbuf, uint32_t *Len);
static int8_t CDCB_TransmitCplt_FS(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_Interface_fops_cdcb_FS =
{
  CDCB_Init_FS,
  CDCB_DeInit_FS,
  CDCB_Control_FS,
  CDCB_Receive_FS,
  CDCB_TransmitCplt_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDCB_Init_FS(void)
{
  /* Set Application Buffers */
	USBD_CDCB_SetTxBuffer(&hUsbDeviceFS, TxBufferCDCB, 0);
	USBD_CDCB_SetRxBuffer(&hUsbDeviceFS, RxBufferCDCB);
  return (USBD_OK);
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDCB_DeInit_FS(void)
{
  return (USBD_OK);
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDCB_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  switch(cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:   break;
    case CDC_GET_ENCAPSULATED_RESPONSE:   break;
    case CDC_SET_COMM_FEATURE:            break;
    case CDC_GET_COMM_FEATURE:            break;
    case CDC_CLEAR_COMM_FEATURE:          break;
  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
    case CDC_SET_LINE_CODING:            break;
    case CDC_GET_LINE_CODING:            break;
    case CDC_SET_CONTROL_LINE_STATE:     break;
    case CDC_SEND_BREAK:                 break;
  default: break;
  }
  return (USBD_OK);
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDCB_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  USBD_CDCB_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
  CDCB_Transmit_FS(Buf, *Len);  
  USBD_CDCB_ReceivePacket(&hUsbDeviceFS);
  return (USBD_OK);
}

extern USBD_CDC_HandleTypeDef     *hcdcb;

/**
  * @brief  CDCA_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDCB_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  if (hcdcb->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDCB_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  result = USBD_CDCB_TransmitPacket(&hUsbDeviceFS);
  return result;
}

/**
  * @brief  CDCA_TransmitCplt_FS
  *         Data transmited callback
  *
  *         @note
  *         This function is IN transfer complete callback used to inform user that
  *         the submitted Data is successfully sent over USB.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDCB_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
  uint8_t result = USBD_OK;
  UNUSED(Buf);
  UNUSED(Len);
  UNUSED(epnum);
  return result;
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
