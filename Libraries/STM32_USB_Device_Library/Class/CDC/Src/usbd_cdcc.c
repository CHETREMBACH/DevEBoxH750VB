/**
  ******************************************************************************
  * @file    usbd_cdcc.c
  * @author  MCD Application Team
  * @brief   This file provides the high layer firmware functions to manage the
  *          following functionalities of the USB CDC Class:
  *           - Initialization and Configuration of high and low layer
  *           - Enumeration as CDC Device (and enumeration for each implemented memory interface)
  *           - OUT/IN data transfer
  *           - Command IN transfer (class requests management)
  *           - Error management
  *
  *  @verbatim
  *
  *          ===================================================================
  *                                CDC Class Driver Description
  *          ===================================================================
  *           This driver manages the "Universal Serial Bus Class Definitions for Communications Devices
  *           Revision 1.2 November 16, 2007" and the sub-protocol specification of "Universal Serial Bus
  *           Communications Class Subclass Specification for PSTN Devices Revision 1.2 February 9, 2007"
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Enumeration as CDC device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
  *             - Requests management (as described in section 6.2 in specification)
  *             - Abstract Control Model compliant
  *             - Union Functional collection (using 1 IN endpoint for control)
  *             - Data interface class
  *
  *           These aspects may be enriched or modified for a specific user application.
  *
  *            This driver doesn't implement the following aspects of the specification
  *            (but it is possible to manage these features with some modifications on this driver):
  *             - Any class-specific aspect relative to communication classes should be managed by user application.
  *             - All communication classes other than PSTN are not managed
  *
  *  @endverbatim
  *
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

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdcc.h"
#include "usbd_ctlreq.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_CDC
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_CDCC_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_CDCC_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_CDCC_Private_Macros
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_CDCC_Private_FunctionPrototypes
  * @{
  */

static uint8_t USBD_CDCC_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
uint8_t USBD_CDCC_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_CDCC_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_CDCC_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_CDCC_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_CDCC_EP0_RxReady(USBD_HandleTypeDef *pdev);

static uint8_t *USBD_CDCC_GetFSCfgDesc(uint16_t *length);
static uint8_t *USBD_CDCC_GetHSCfgDesc(uint16_t *length);
static uint8_t *USBD_CDCC_GetOtherSpeedCfgDesc(uint16_t *length);
static uint8_t *USBD_CDCC_GetOtherSpeedCfgDesc(uint16_t *length);
uint8_t *USBD_CDCC_GetDeviceQualifierDescriptor(uint16_t *length);

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CDCC_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/**
  * @}
  */

/** @defgroup USBD_CDCC_Private_Variables
  * @{
  */


/* CDC interface class callbacks structure */
USBD_ClassTypeDef  USBD_CDCC =
{
  USBD_CDCC_Init,
  USBD_CDCC_DeInit,
  USBD_CDCC_Setup,
  NULL,                 /* EP0_TxSent, */
  USBD_CDCC_EP0_RxReady,
  USBD_CDCC_DataIn,
  USBD_CDCC_DataOut,
  NULL,
  NULL,
  NULL,
  USBD_CDCC_GetHSCfgDesc,
  USBD_CDCC_GetFSCfgDesc,
  USBD_CDCC_GetOtherSpeedCfgDesc,
  USBD_CDCC_GetDeviceQualifierDescriptor,
};

/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CDCC_CfgHSDesc[USB_CDC_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /* Configuration Descriptor */
  0x09,                                       /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,                /* bDescriptorType: Configuration */
  USB_CDC_CONFIG_DESC_SIZ,                    /* wTotalLength:no of returned bytes */
  0x00,
  0x02,                                       /* bNumInterfaces: 2 interface */
  0x01,                                       /* bConfigurationValue: Configuration value */
  0x00,                                       /* iConfiguration: Index of string descriptor describing the configuration */
#if (USBD_SELF_POWERED == 1U)
  0xC0,                                       /* bmAttributes: Bus Powered according to user configuration */
#else
  0x80,                                       /* bmAttributes: Bus Powered according to user configuration */
#endif
  USBD_MAX_POWER,                             /* MaxPower 100 mA */
  /******** IAD should be positioned just before the CDC interfaces ******
  		 IAD to associate the two CDC interfaces */
  
  0x08, /* bLength */
  0x0B, /* bDescriptorType */
  CDCC_CMD_INTERFACE_IDX, /* bFirstInterface */
  0x02, /* bInterfaceCount */
  0x02, /* bFunctionClass */
  0x02, /* bFunctionSubClass */
  0x01, /* bFunctionProtocol */
  0x00, /* iFunction (Index of string descriptor describing this function) */
  /* 08 bytes */
  /* Interface Descriptor */
  0x09,                                       /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,                    /* bDescriptorType: Interface */
  CDCC_CMD_INTERFACE_IDX,                      /* bInterfaceNumber: Number of Interface */
  0x00,                                       /* bAlternateSetting: Alternate setting */
  0x01,                                       /* bNumEndpoints: One endpoints used */
  0x02,                                       /* bInterfaceClass: Communication Interface Class */
  0x02,                                       /* bInterfaceSubClass: Abstract Control Model */
  0x01,                                       /* bInterfaceProtocol: Common AT commands */
  0x00,                                       /* iInterface: */
//  /* Configuration Descriptor */
  /* Header Functional Descriptor */
  0x05,                                       /* bLength: Endpoint Descriptor size */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x00,                                       /* bDescriptorSubtype: Header Func Desc */
  0x10,                                       /* bcdCDC: spec release number */
  0x01,
//  0x09,                                       /* bLength: Configuration Descriptor size */
  /* Call Management Functional Descriptor */
  0x05,                                       /* bFunctionLength */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x01,                                       /* bDescriptorSubtype: Call Management Func Desc */
  0x00,                                       /* bmCapabilities: D0+D1 */
  CDCC_DATA_INTERFACE_IDX,                     /* bDataInterface: 1 */
//  USB_DESC_TYPE_CONFIGURATION,                /* bDescriptorType: Configuration */
  /* ACM Functional Descriptor */
  0x04,                                       /* bFunctionLength */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x02,                                       /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,                                       /* bmCapabilities */
//  USB_CDCC_CONFIG_DESC_SIZ,                    /* wTotalLength:no of returned bytes */
  /* Union Functional Descriptor */
  0x05,                                       /* bFunctionLength */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x06,                                       /* bDescriptorSubtype: Union func desc */
  CDCC_CMD_INTERFACE_IDX,                      /* bMasterInterface: Communication class interface */
  CDCC_DATA_INTERFACE_IDX,                     /* bSlaveInterface0: Data Class Interface */
//  0x00,
  /* Endpoint 2 Descriptor */
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  CDCC_CMD_EP,                                 /* bEndpointAddress */
  0x03,                                       /* bmAttributes: Interrupt */
  LOBYTE(CDC_CMD_PACKET_SIZE),                /* wMaxPacketSize: */
  HIBYTE(CDC_CMD_PACKET_SIZE),
  CDC_HS_BINTERVAL,                           /* bInterval: */
  /*---------------------------------------------------------------------------*/
//  0x02,                                       /* bNumInterfaces: 2 interface */
  /* Data class interface descriptor */
  0x09,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE,                    /* bDescriptorType: */
  CDCC_DATA_INTERFACE_IDX,                     /* bInterfaceNumber: Number of Interface */
  0x00,                                       /* bAlternateSetting: Alternate setting */
  0x02,                                       /* bNumEndpoints: Two endpoints used */
  0x0A,                                       /* bInterfaceClass: CDC */
  0x00,                                       /* bInterfaceSubClass: */
  0x00,                                       /* bInterfaceProtocol: */
  0x00,                                       /* iInterface: */
//  0x01,                                       /* bConfigurationValue: Configuration value */
  /* Endpoint OUT Descriptor */
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  CDCC_OUT_EP,                                 /* bEndpointAddress */
  0x02,                                       /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),        /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),
  0x00,                                       /* bInterval: ignore for Bulk transfer */
//  0x00,                                       /* iConfiguration: Index of string descriptor describing the configuration */
  /* Endpoint IN Descriptor */
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  CDCC_IN_EP,                                  /* bEndpointAddress */
  0x02,                                       /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),        /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),
  0x00                                        /* bInterval: ignore for Bulk transfer */
};


/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CDCC_CfgFSDesc[USB_CDC_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /* Configuration Descriptor */
  0x09,                                       /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,                /* bDescriptorType: Configuration */
  USB_CDC_CONFIG_DESC_SIZ,                    /* wTotalLength:no of returned bytes */
  0x00,
  0x02,                                       /* bNumInterfaces: 2 interface */
  0x01,                                       /* bConfigurationValue: Configuration value */
  0x00,                                       /* iConfiguration: Index of string descriptor describing the configuration */
#if (USBD_SELF_POWERED == 1U)
  0xC0,                                       /* bmAttributes: Bus Powered according to user configuration */
#else
  0x80,                                       /* bmAttributes: Bus Powered according to user configuration */
#endif
  USBD_MAX_POWER,                             /* MaxPower 100 mA */

  /******** IAD should be positioned just before the CDC interfaces ******
  		 IAD to associate the two CDC interfaces */
  
  0x08, /* bLength */
  0x0B, /* bDescriptorType */
  CDCC_CMD_INTERFACE_IDX, /* bFirstInterface */
  0x02, /* bInterfaceCount */
  0x02, /* bFunctionClass */
  0x02, /* bFunctionSubClass */
  0x01, /* bFunctionProtocol */
  0x00, /* iFunction (Index of string descriptor describing this function) */
  /* 08 bytes */	
  /*---------------------------------------------------------------------------*/

  /* Interface Descriptor */
  0x09,                                       /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,                    /* bDescriptorType: Interface */
  CDCC_CMD_INTERFACE_IDX,                      /* bInterfaceNumber: Number of Interface */
  0x00,                                       /* bAlternateSetting: Alternate setting */
  0x01,                                       /* bNumEndpoints: One endpoints used */
  0x02,                                       /* bInterfaceClass: Communication Interface Class */
  0x02,                                       /* bInterfaceSubClass: Abstract Control Model */
  0x01,                                       /* bInterfaceProtocol: Common AT commands */
  0x00,                                       /* iInterface: */

  /* Header Functional Descriptor */
  0x05,                                       /* bLength: Endpoint Descriptor size */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x00,                                       /* bDescriptorSubtype: Header Func Desc */
  0x10,                                       /* bcdCDC: spec release number */
  0x01,

  /* Call Management Functional Descriptor */
  0x05,                                       /* bFunctionLength */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x01,                                       /* bDescriptorSubtype: Call Management Func Desc */
  0x00,                                       /* bmCapabilities: D0+D1 */
  0x01,                                       /* bDataInterface: 1 */

  /* ACM Functional Descriptor */
  0x04,                                       /* bFunctionLength */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x02,                                       /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,                                       /* bmCapabilities */

  /* Union Functional Descriptor */
  0x05,                                       /* bFunctionLength */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x06,                                       /* bDescriptorSubtype: Union func desc */
  CDCC_CMD_INTERFACE_IDX,                      /* bMasterInterface: Communication class interface */
  CDCC_DATA_INTERFACE_IDX,                     /* bSlaveInterface0: Data Class Interface */

  /* Endpoint 2 Descriptor */
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  CDCC_CMD_EP,                                 /* bEndpointAddress */
  0x03,                                       /* bmAttributes: Interrupt */
  LOBYTE(CDC_CMD_PACKET_SIZE),                /* wMaxPacketSize: */
  HIBYTE(CDC_CMD_PACKET_SIZE),
  CDC_FS_BINTERVAL,                           /* bInterval: */
  /*---------------------------------------------------------------------------*/

  /* Data class interface descriptor */
  0x09,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE,                    /* bDescriptorType: */
  CDCC_DATA_INTERFACE_IDX,                     /* bInterfaceNumber: Number of Interface */
  0x00,                                       /* bAlternateSetting: Alternate setting */
  0x02,                                       /* bNumEndpoints: Two endpoints used */
  0x0A,                                       /* bInterfaceClass: CDC */
  0x00,                                       /* bInterfaceSubClass: */
  0x00,                                       /* bInterfaceProtocol: */
  0x00,                                       /* iInterface: */

  /* Endpoint OUT Descriptor */
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  CDCC_OUT_EP,                                 /* bEndpointAddress */
  0x02,                                       /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),        /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
  0x00,                                       /* bInterval: ignore for Bulk transfer */

  /* Endpoint IN Descriptor */
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  CDCC_IN_EP,                                  /* bEndpointAddress */
  0x02,                                       /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),        /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
  0x00                                        /* bInterval: ignore for Bulk transfer */
};
__ALIGN_BEGIN static uint8_t USBD_CDCC_OtherSpeedCfgDesc[USB_CDC_CONFIG_DESC_SIZ] __ALIGN_END =
{
  0x09,                                       /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION,
  USB_CDC_CONFIG_DESC_SIZ,
  0x00,
  0x02,                                       /* bNumInterfaces: 2 interfaces */
  0x01,                                       /* bConfigurationValue: */
  0x04,                                       /* iConfiguration: */
#if (USBD_SELF_POWERED == 1U)
  0xC0,                                       /* bmAttributes: Bus Powered according to user configuration */
#else
  0x80,                                       /* bmAttributes: Bus Powered according to user configuration */
#endif
  USBD_MAX_POWER,                             /* MaxPower 100 mA */
  /******** IAD should be positioned just before the CDC interfaces ******
  		 IAD to associate the two CDC interfaces */
  
  0x08, /* bLength */
  0x0B, /* bDescriptorType */
  CDCC_CMD_INTERFACE_IDX, /* bFirstInterface */
  0x02, /* bInterfaceCount */
  0x02, /* bFunctionClass */
  0x02, /* bFunctionSubClass */
  0x01, /* bFunctionProtocol */
  0x00, /* iFunction (Index of string descriptor describing this function) */
  /* 08 bytes */
  /* Interface Descriptor */
  0x09,                                       /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,                    /* bDescriptorType: Interface */
  CDCC_CMD_INTERFACE_IDX,                      /* bInterfaceNumber: Number of Interface */
  0x00,                                       /* bAlternateSetting: Alternate setting */
  0x01,                                       /* bNumEndpoints: One endpoints used */
  0x02,                                       /* bInterfaceClass: Communication Interface Class */
  0x02,                                       /* bInterfaceSubClass: Abstract Control Model */
  0x01,                                       /* bInterfaceProtocol: Common AT commands */
  0x00,                                       /* iInterface: */

  /* Header Functional Descriptor */
  0x05,                                       /* bLength: Endpoint Descriptor size */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x00,                                       /* bDescriptorSubtype: Header Func Desc */
  0x10,                                       /* bcdCDC: spec release number */
  0x01,

  /* Call Management Functional Descriptor */
  0x05,                                       /* bFunctionLength */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x01,                                       /* bDescriptorSubtype: Call Management Func Desc */
  0x00,                                       /* bmCapabilities: D0+D1 */
  CDCC_DATA_INTERFACE_IDX,                     /* bDataInterface: 1 */

  /* ACM Functional Descriptor */
  0x04,                                       /* bFunctionLength */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x02,                                       /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,                                       /* bmCapabilities */

  /* Union Functional Descriptor */
  0x05,                                       /* bFunctionLength */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x06,                                       /* bDescriptorSubtype: Union func desc */
  CDCC_CMD_INTERFACE_IDX,                      /* bMasterInterface: Communication class interface */
  CDCC_DATA_INTERFACE_IDX,                     /* bSlaveInterface0: Data Class Interface */

  /* Endpoint 2 Descriptor */
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  CDCC_CMD_EP,                                 /* bEndpointAddress */
  0x03,                                       /* bmAttributes: Interrupt */
  LOBYTE(CDC_CMD_PACKET_SIZE),                /* wMaxPacketSize: */
  HIBYTE(CDC_CMD_PACKET_SIZE),
  CDC_FS_BINTERVAL,                           /* bInterval: */
  /*---------------------------------------------------------------------------*/

  /* Data class interface descriptor */
  0x09,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE,                    /* bDescriptorType: */
  CDCC_DATA_INTERFACE_IDX,                     /* bInterfaceNumber: Number of Interface */
  0x00,                                       /* bAlternateSetting: Alternate setting */
  0x02,                                       /* bNumEndpoints: Two endpoints used */
  0x0A,                                       /* bInterfaceClass: CDC */
  0x00,                                       /* bInterfaceSubClass: */
  0x00,                                       /* bInterfaceProtocol: */
  0x00,                                       /* iInterface: */

  /*Endpoint OUT Descriptor*/
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  CDCC_OUT_EP,                                 /* bEndpointAddress */
  0x02,                                       /* bmAttributes: Bulk */
  0x40,                                       /* wMaxPacketSize: */
  0x00,
  0x00,                                       /* bInterval: ignore for Bulk transfer */

  /*Endpoint IN Descriptor*/
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  CDCC_IN_EP,                                  /* bEndpointAddress */
  0x02,                                       /* bmAttributes: Bulk */
  0x40,                                       /* wMaxPacketSize: */
  0x00,
  0x00                                        /* bInterval */
};

/**
  * @}
  */

USBD_CDC_HandleTypeDef CDCC_usbd;
USBD_CDC_ItfTypeDef *fops_CDCC_p;
USBD_CDC_HandleTypeDef *hcdcc;

/** @defgroup USBD_CDCC_Private_Functions
  * @{
  */

/**
  * @brief  USBD_CDCC_Init
  *         Initialize the CDC interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_CDCC_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	UNUSED(cfgidx);

	if (pdev->dev_speed == USBD_SPEED_HIGH)
	{
		/* Open EP IN */
		(void)USBD_LL_OpenEP(pdev,
			CDCC_IN_EP,
			USBD_EP_TYPE_BULK,
			CDC_DATA_HS_IN_PACKET_SIZE);

		pdev->ep_in[CDCC_IN_EP & 0xFU].is_used = 1U;

		/* Open EP OUT */
		(void)USBD_LL_OpenEP(pdev,
			CDCC_OUT_EP,
			USBD_EP_TYPE_BULK,
			CDC_DATA_HS_OUT_PACKET_SIZE);

		pdev->ep_out[CDCC_OUT_EP & 0xFU].is_used = 1U;

		/* Set bInterval for CDC CMD Endpoint */
		pdev->ep_in[CDCC_CMD_EP & 0xFU].bInterval = CDC_HS_BINTERVAL;
	}
	else
	{
		/* Open EP IN */
		(void)USBD_LL_OpenEP(pdev,
			CDCC_IN_EP,
			USBD_EP_TYPE_BULK,
			CDC_DATA_FS_IN_PACKET_SIZE);

		pdev->ep_in[CDCC_IN_EP & 0xFU].is_used = 1U;

		/* Open EP OUT */
		(void)USBD_LL_OpenEP(pdev,
			CDCC_OUT_EP,
			USBD_EP_TYPE_BULK,
			CDC_DATA_FS_OUT_PACKET_SIZE);

		pdev->ep_out[CDCC_OUT_EP & 0xFU].is_used = 1U;

		/* Set bInterval for CMD Endpoint */
		pdev->ep_in[CDCC_CMD_EP & 0xFU].bInterval = CDC_FS_BINTERVAL;
	}

	/* Open Command IN EP */
	(void)USBD_LL_OpenEP(pdev, CDCC_CMD_EP, USBD_EP_TYPE_INTR, CDC_CMD_PACKET_SIZE);
	pdev->ep_in[CDCC_CMD_EP & 0xFU].is_used = 1U;
	
	if (fops_CDCC_p == NULL)	
	{
		return (uint8_t)USBD_FAIL;
	}	
	
	hcdcc = &CDCC_usbd;

	if (hcdcc == NULL)
	{
		//pdev->pClassData = NULL;
		return (uint8_t)USBD_EMEM;
	}
	//pdev->pClassData = (void *)hcdc;		

	/* Init  physical Interface components */
	fops_CDCC_p->Init();

	/* Init Xfer states */
	hcdcc->TxState = 0U;
	hcdcc->RxState = 0U;

	if (pdev->dev_speed == USBD_SPEED_HIGH)
	{
		/* Prepare Out endpoint to receive next packet */
		(void)USBD_LL_PrepareReceive(pdev,
			CDCC_OUT_EP,
			hcdcc->RxBuffer,
			CDC_DATA_HS_OUT_PACKET_SIZE);
	}
	else
	{
		/* Prepare Out endpoint to receive next packet */
		(void)USBD_LL_PrepareReceive(pdev,
			CDCC_OUT_EP,
			hcdcc->RxBuffer,
			CDC_DATA_FS_OUT_PACKET_SIZE);
	}
	return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CDCC_Init
  *         DeInitialize the CDC layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
uint8_t USBD_CDCC_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	UNUSED(cfgidx);

	/* Close EP IN */
	(void)USBD_LL_CloseEP(pdev, CDCC_IN_EP);
	pdev->ep_in[CDCC_IN_EP & 0xFU].is_used = 0U;

	/* Close EP OUT */
	(void)USBD_LL_CloseEP(pdev, CDCC_OUT_EP);
	pdev->ep_out[CDCC_OUT_EP & 0xFU].is_used = 0U;

	/* Close Command IN EP */
	(void)USBD_LL_CloseEP(pdev, CDCC_CMD_EP);
	pdev->ep_in[CDCC_CMD_EP & 0xFU].is_used = 0U;
	pdev->ep_in[CDCC_CMD_EP & 0xFU].bInterval = 0U;

	/* DeInit  physical Interface components */
	if (fops_CDCC_p != NULL)	
	{
		fops_CDCC_p->DeInit();	
	}	
	return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CDCC_Setup
  *         Handle the CDC specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_CDCC_Setup(USBD_HandleTypeDef *pdev,
                              USBD_SetupReqTypedef *req)
{
  uint16_t len;
  uint8_t ifalt = 0U;
  uint16_t status_info = 0U;
  USBD_StatusTypeDef ret = USBD_OK;
	
  if (hcdcc == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS:
      if (req->wLength != 0U)
      {
        if ((req->bmRequest & 0x80U) != 0U)
        {
	      fops_CDCC_p->Control(req->bRequest, (uint8_t *)hcdcc->data, req->wLength);

          len = MIN(CDC_REQ_MAX_DATA_SIZE, req->wLength);
          (void)USBD_CtlSendData(pdev, (uint8_t *)hcdcc->data, len);
        }
        else
        {
          hcdcc->CmdOpCode = req->bRequest;
          hcdcc->CmdLength = (uint8_t)req->wLength;

          (void)USBD_CtlPrepareRx(pdev, (uint8_t *)hcdcc->data, req->wLength);
        }
      }
      else
      {
	      fops_CDCC_p->Control(req->bRequest, (uint8_t *)req, 0U);
      }
      break;

    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void)USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_GET_INTERFACE:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void)USBD_CtlSendData(pdev, &ifalt, 1U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_SET_INTERFACE:
          if (pdev->dev_state != USBD_STATE_CONFIGURED)
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_CLEAR_FEATURE:
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
  }
  return (uint8_t)ret;
}

/**
  * @brief  USBD_CDCC_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t USBD_CDCC_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{	
	T1_HI;
	if (hcdcc == NULL)
	{
		return (uint8_t)USBD_FAIL;
	}

	if ((hcdcc->TxLength) && ((hcdcc->TxLength & 63) == 0))
	{
		hcdcc->TxLength = 0;
		/* Send ZLP */
		(void)USBD_LL_Transmit(pdev, epnum, NULL, 0U);
	}
	else
	{
		hcdcc->TxState = 0U;

		if (fops_CDCC_p != NULL)
		{
			if (fops_CDCC_p->TransmitCplt != NULL) 
			{
				fops_CDCC_p->TransmitCplt(hcdcc->TxBuffer, &hcdcc->TxLength, epnum);
			}
		}
	}
	T1_LO;
	return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CDCC_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t USBD_CDCC_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  T2_HI;	
  if (hcdcc == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  /* Get the received data length */
  hcdcc->RxLength = USBD_LL_GetRxDataSize(pdev, epnum);

  /* USB data will be immediately processed, this allow next USB traffic being
  NAKed till the end of the application Xfer */
	if (fops_CDCC_p != NULL) 
	{
		fops_CDCC_p->Receive(hcdcc->RxBuffer, &hcdcc->RxLength);
	}
  T2_LO;
  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CDCC_EP0_RxReady
  *         Handle EP0 Rx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_CDCC_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
  if (hcdcc == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  if ((fops_CDCC_p != NULL) && (hcdcc->CmdOpCode != 0xFFU))
  {
    fops_CDCC_p->Control(hcdcc->CmdOpCode, (uint8_t *)hcdcc->data, (uint16_t)hcdcc->CmdLength);
    hcdcc->CmdOpCode = 0xFFU;
  }
  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CDCC_GetFSCfgDesc
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_CDCC_GetFSCfgDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_CDCC_CfgFSDesc);
  return USBD_CDCC_CfgFSDesc;
}

/**
  * @brief  USBD_CDCC_GetHSCfgDesc
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_CDCC_GetHSCfgDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_CDCC_CfgHSDesc);
  return USBD_CDCC_CfgHSDesc;
}

/**
  * @brief  USBD_CDCC_GetOtherSpeedCfgDesc
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_CDCC_GetOtherSpeedCfgDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_CDCC_OtherSpeedCfgDesc);
  return USBD_CDCC_OtherSpeedCfgDesc;
}

/**
  * @brief  USBD_CDCC_GetDeviceQualifierDescriptor
  *         return Device Qualifier descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
uint8_t *USBD_CDCC_GetDeviceQualifierDescriptor(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_CDCC_DeviceQualifierDesc);
  return USBD_CDCC_DeviceQualifierDesc;
}

/**
  * @brief  USBD_CDCC_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: CD  Interface callback
  * @retval status
  */
uint8_t USBD_CDCC_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_CDC_ItfTypeDef *fops)
{
  if (fops == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  fops_CDCC_p = fops;	
  //pdev->pUserData = fops;

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CDCC_SetTxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Tx Buffer
  * @retval status
  */
uint8_t USBD_CDCC_SetTxBuffer(USBD_HandleTypeDef *pdev,
                             uint8_t *pbuff, uint32_t length)
{
  if (hcdcc == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  hcdcc->TxBuffer = pbuff;
  hcdcc->TxLength = length;

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CDCC_SetRxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Rx Buffer
  * @retval status
  */
uint8_t USBD_CDCC_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff)
{
  if (hcdcc == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  hcdcc->RxBuffer = pbuff;

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CDCC_TransmitPacket
  *         Transmit packet on IN endpoint
  * @param  pdev: device instance
  * @retval status
  */
uint8_t USBD_CDCC_TransmitPacket(USBD_HandleTypeDef *pdev)
{
  USBD_StatusTypeDef ret = USBD_BUSY;

  if (hcdcc == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  if (hcdcc->TxState == 0U)
  {
    /* Tx Transfer in progress */
    hcdcc->TxState = 1U;

    /* Update the packet total length */
    pdev->ep_in[CDCC_IN_EP & 0xFU].total_length = hcdcc->TxLength;

    /* Transmit next packet */
    (void)USBD_LL_Transmit(pdev, CDCC_IN_EP, hcdcc->TxBuffer, hcdcc->TxLength);

    ret = USBD_OK;
  }

  return (uint8_t)ret;
}

/**
  * @brief  USBD_CDCC_ReceivePacket
  *         prepare OUT Endpoint for reception
  * @param  pdev: device instance
  * @retval status
  */
uint8_t USBD_CDCC_ReceivePacket(USBD_HandleTypeDef *pdev)
{
  if (hcdcc == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  if (pdev->dev_speed == USBD_SPEED_HIGH)
  {
    /* Prepare Out endpoint to receive next packet */
    (void)USBD_LL_PrepareReceive(pdev, CDCC_OUT_EP, hcdcc->RxBuffer,
                                 CDC_DATA_HS_OUT_PACKET_SIZE);
  }
  else
  {
    /* Prepare Out endpoint to receive next packet */
    (void)USBD_LL_PrepareReceive(pdev, CDCC_OUT_EP, hcdcc->RxBuffer,
                                 CDC_DATA_FS_OUT_PACKET_SIZE);
  }
  return (uint8_t)USBD_OK;
}
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
