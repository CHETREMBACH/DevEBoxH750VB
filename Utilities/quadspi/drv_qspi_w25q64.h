/**
  ******************************************************************************
  * @file    drv_qspi_w25q64.h
  * @author  
  * @brief   This file contains the common defines and functions prototypes for
  *          the drv_qspi_w25q64.c driver.
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_QSPI_W25Q64_H
#define __DRV_QSPI_W25Q64_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal_qspi.h"
#include "W25Q64JVS.h"
#include "main.h"	 

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H743I_EVAL
  * @{
  */

/** @addtogroup STM32H743I_EVAL_QSPI
  * @{
  */
/* Exported types ------------------------------------------------------------*/
/** @defgroup STM32H743I_EVAL_QSPI_Exported_Types QSPI Exported Types
  * @{
  */
#define BSP_QSPI_Info_t                 W25Q64JVS_Info_t
#define BSP_QSPI_Interface_t            W25Q64JVS_Interface_t
#define BSP_QSPI_DualFlash_t            W25Q64JVS_DualFlash_t
#define BSP_QSPI_Erase_t                W25Q64JVS_Erase_t

typedef enum
{
  QSPI_ACCESS_NONE = 0,                 /*!<  Instance not initialized,             */
  QSPI_ACCESS_INDIRECT,                 /*!<  Instance use indirect mode access     */
  QSPI_ACCESS_MMP                       /*!<  Instance use Memory Mapped Mode read  */
} BSP_QSPI_Access_t;

typedef struct
{
  BSP_QSPI_Access_t    IsInitialized;   /*!<  Instance access Flash method     */
  BSP_QSPI_Interface_t InterfaceMode;   /*!<  Flash Interface mode of Instance */
  uint32_t             DualFlashMode;   /*!<  Flash dual mode                  */
  uint32_t             IsMspCallbacksValid;
} BSP_QSPI_Ctx_t;


typedef struct
{
  QSPI_HandleTypeDef   hqspi;	
  BSP_QSPI_Access_t    IsInitialized;   /*!<  Instance access Flash method     */
  BSP_QSPI_Interface_t InterfaceMode;   /*!<  Flash Interface mode of Instance */
  uint32_t             DualFlashMode;   /*!<  Flash dual mode                  */
  uint32_t             IsMspCallbacksValid;
} BSP_QSPI_Hndlr_t;	 
	 
typedef struct
{
  BSP_QSPI_Interface_t        InterfaceMode;   /*!<  Current Flash Interface mode */
  BSP_QSPI_DualFlash_t        DualFlashMode;   /*!<  Dual Flash mode              */
} BSP_QSPI_Init_t;

typedef struct
{
  uint32_t FlashSize;
  uint32_t ClockPrescaler;
  uint32_t SampleShifting;
  uint32_t DualFlashMode;
}MX_QSPI_Init_t;
#if (USE_HAL_QSPI_REGISTER_CALLBACKS == 1)
typedef struct
{
 void(*pMspInitCb)(pQSPI_CallbackTypeDef);
 void(*pMspDeInitCb)(pQSPI_CallbackTypeDef);
}BSP_QSPI_Cb_t;
#endif /* (USE_HAL_QSPI_REGISTER_CALLBACKS == 1) */

/* Exported constants --------------------------------------------------------*/

/* QSPI instances number */
#define QSPI_INSTANCES_NUMBER         1U

/* Definition for QSPI modes */
#define BSP_QSPI_SPI_MODE            (BSP_QSPI_Interface_t)W25Q64JVS_SPI_MODE      /* 1 Cmd Line, 1 Address Line and 1 Data Line    */
#define BSP_QSPI_QPI_MODE            (BSP_QSPI_Interface_t)W25Q64JVS_QPI_MODE      /* 4 Cmd Lines, 4 Address Lines and 4 Data Lines */

/* Definition for QSPI transfer rates */
#define BSP_QSPI_STR_TRANSFER        (BSP_QSPI_Transfer_t)W25Q64JVS_STR_TRANSFER /* Single Transfer Rate */
#define BSP_QSPI_DTR_TRANSFER        (BSP_QSPI_Transfer_t)W25Q64JVS_DTR_TRANSFER /* Double Transfer Rate */

/* Definition for QSPI dual flash mode */
#define BSP_QSPI_DUALFLASH_DISABLE   (BSP_QSPI_DualFlash_t)W25Q64JVS_DUALFLASH_DISABLE   /* Dual flash mode enabled  */
/* Definition for QSPI Flash ID */
#define BSP_QSPI_FLASH_ID            QSPI_FLASH_ID_1

/* QSPI erase types */
#define BSP_QSPI_ERASE_4K            (W25Q64JVS_Erase_t)W25Q64JVS_ERASE_4K
#define BSP_QSPI_ERASE_32K           (W25Q64JVS_Erase_t)W25Q64JVS_ERASE_32K
#define BSP_QSPI_ERASE_64K           (W25Q64JVS_Erase_t)W25Q64JVS_ERASE_64K
#define BSP_QSPI_ERASE_CHIP          (W25Q64JVS_Erase_t)W25Q64JVS_ERASE_CHIP

/* QSPI block sizes */
#define BSP_QSPI_BLOCK_4K            W25Q64JVS_SECTOR_4K
#define BSP_QSPI_BLOCK_32K           W25Q64JVS_BLOCK_32K
#define BSP_QSPI_BLOCK_64K           W25Q64JVS_BLOCK_64K

/* Definition for QSPI clock resources */
#define QSPI_CLK_ENABLE()              __HAL_RCC_QSPI_CLK_ENABLE()
#define QSPI_CLK_DISABLE()             __HAL_RCC_QSPI_CLK_DISABLE()
#define QSPI_CLK_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOB_CLK_ENABLE()
#define QSPI_BK1_CS_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()
#define QSPI_BK1_D0_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOD_CLK_ENABLE()
#define QSPI_BK1_D1_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOD_CLK_ENABLE()
#define QSPI_BK1_D2_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOE_CLK_ENABLE()
#define QSPI_BK1_D3_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOD_CLK_ENABLE()

#define QSPI_MDMA_CLK_ENABLE()         __HAL_RCC_MDMA_CLK_ENABLE()
	 
#define QSPI_FORCE_RESET()             __HAL_RCC_QSPI_FORCE_RESET()
#define QSPI_RELEASE_RESET()           __HAL_RCC_QSPI_RELEASE_RESET()

/* Definition for QSPI Pins */
	/**QUADSPI GPIO Configuration  
	PB2      ------> QUADSPI_CLK
	PB6      ------> QUADSPI_BK1_NCS 
	PD11     ------> QUADSPI_BK1_IO0
	PD12     ------> QUADSPI_BK1_IO1  
	PE2      ------> QUADSPI_BK1_IO2
	PD13     ------> QUADSPI_BK1_IO3
	*/	 
	 
#define QSPI_CLK_PIN                  GPIO_PIN_2
#define QSPI_CLK_GPIO_PORT            GPIOB
#define QSPI_CLK_GPIO_ALTERNATE       GPIO_AF9_QUADSPI
/* Bank 1 */					      
#define QSPI_BK1_CS_PIN               GPIO_PIN_6
#define QSPI_BK1_CS_GPIO_PORT         GPIOB
#define QSPI_BK1_CS_GPIO_ALTERNATE    GPIO_AF10_QUADSPI
#define QSPI_BK1_D0_PIN               GPIO_PIN_11
#define QSPI_BK1_D0_GPIO_PORT         GPIOD
#define QSPI_BK1_D0_GPIO_ALTERNATE    GPIO_AF9_QUADSPI
#define QSPI_BK1_D1_PIN               GPIO_PIN_12
#define QSPI_BK1_D1_GPIO_PORT         GPIOD
#define QSPI_BK1_D1_GPIO_ALTERNATE    GPIO_AF9_QUADSPI
#define QSPI_BK1_D2_PIN               GPIO_PIN_2
#define QSPI_BK1_D2_GPIO_PORT         GPIOE
#define QSPI_BK1_D2_GPIO_ALTERNATE    GPIO_AF9_QUADSPI
#define QSPI_BK1_D3_PIN               GPIO_PIN_13
#define QSPI_BK1_D3_GPIO_PORT         GPIOD
#define QSPI_BK1_D3_GPIO_ALTERNATE    GPIO_AF9_QUADSPI

/* W25Q64JVS memory */
/* Size of the flash */
#define QSPI_FLASH_SIZE            24     /* Address bus width to access whole memory space */
#define QSPI_PAGE_SIZE             256

/* QSPI Base Address */
#define QSPI_BASE_ADDRESS          0x90000000
	 
extern BSP_QSPI_Hndlr_t    pqspi;
/* Exported functions --------------------------------------------------------*/

int32_t BSP_QSPI_Init(void);
int32_t BSP_QSPI_DeInit(void);
#if (USE_HAL_QSPI_REGISTER_CALLBACKS == 1)
int32_t BSP_QSPI_RegisterMspCallbacks (uint32_t Instance, BSP_QSPI_Cb_t *CallBacks);
int32_t BSP_QSPI_RegisterDefaultMspCallbacks (uint32_t Instance);
#endif /* (USE_HAL_QSPI_REGISTER_CALLBACKS == 1) */
int32_t BSP_QSPI_Read(uint8_t *pData, uint32_t ReadAddr, uint32_t Size);
int32_t BSP_QSPI_DMARead(uint8_t *pData, uint32_t ReadAddr, uint32_t Size);
int32_t BSP_QSPI_Write(uint8_t *pData, uint32_t WriteAddr, uint32_t Size);
int32_t BSP_QSPI_EraseBlock(uint32_t BlockAddress, BSP_QSPI_Erase_t BlockSize);
int32_t BSP_QSPI_EraseChip(void);
int32_t BSP_QSPI_GetStatus(void);
int32_t BSP_QSPI_GetAllStatus(uint8_t *reg_statue);
int32_t BSP_QSPI_GetInfo(BSP_QSPI_Info_t *pInfo);
int32_t BSP_QSPI_EnableMemoryMappedMode(void);
int32_t BSP_QSPI_DisableMemoryMappedMode(void);
int32_t BSP_QSPI_ReadID(uint8_t *Id);
int32_t BSP_QSPI_ConfigFlash(BSP_QSPI_Interface_t Mode);

/* These functions can be modified in case the current settings
   need to be changed for specific application needs */
HAL_StatusTypeDef MX_QSPI_Init(QSPI_HandleTypeDef *hQspi, MX_QSPI_Init_t *Config);
	 
#ifdef __cplusplus
}
#endif

#endif /* __DRV_QSPI_W25Q128_H */

/************************ (C)  *****END OF FILE****/
