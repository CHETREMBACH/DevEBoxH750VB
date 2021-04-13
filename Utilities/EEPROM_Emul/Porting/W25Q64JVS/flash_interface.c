/**
  ******************************************************************************
  * @file    EEPROM_Emul/Porting/STM32G0/flash_interface.c
  * @author  MCD Application Team
  * @brief   This file provides all the EEPROM emulation flash interface functions.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "eeprom_emul.h"
#include "flash_interface.h"
#include "drv_qspi_w25q64.h"
#include "task_qspi_flash.h"
#include "printf_dbg.h"

/** @addtogroup EEPROM_Emulation
  * @{
  */

uint8_t __attribute__((section(".EEPROM_EMUL_ITCMRAM"))) EEPROM_EMULL_MEM[SIZE_EEPROM_EMUL_MEM];	

/* Private typedef -----------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/** @addtogroup EEPROM_Private_Functions
  * @{
  */

#define START_FLASH_SECTOR  (0)

uint32_t get_address_for_page(uint32_t page)
{
	return (page << 12) + START_PAGE_ADDRESS;
}	
	
uint32_t get_page_for_address(uint32_t address)
{
	return (address - START_PAGE_ADDRESS) >> 12;
}	

uint32_t get_previous_page(uint32_t page)
{
	if (page > START_PAGE)
	{
		return page - 1;		
	}
	else
	{
		return START_PAGE + PAGES_NUMBER - 1;
	}
}

uint32_t get_following_page(uint32_t page)
{
	if (page < ( START_PAGE + PAGES_NUMBER - 1))
	{
		return page + 1;		
	}
	else
	{
		return START_PAGE;
	}
}


/* чтение из зарезервированных секторов Flash в RAM EmulEEPROM в  */
void read_flash_to_ram_emmul_eeprom(void)
{
	BSP_QSPI_Read(&(EEPROM_EMULL_MEM[0]), START_FLASH_SECTOR*W25Q64JVS_SECTOR_SIZE, W25Q64JVS_SECTOR_SIZE);
	BSP_QSPI_Read(&(EEPROM_EMULL_MEM[W25Q64JVS_SECTOR_SIZE]), (START_FLASH_SECTOR + 1)*W25Q64JVS_SECTOR_SIZE, W25Q64JVS_SECTOR_SIZE);	
	BSP_QSPI_Read(&(EEPROM_EMULL_MEM[W25Q64JVS_SECTOR_SIZE * 2]), (START_FLASH_SECTOR + 2)*W25Q64JVS_SECTOR_SIZE, W25Q64JVS_SECTOR_SIZE);		
	BSP_QSPI_Read(&(EEPROM_EMULL_MEM[W25Q64JVS_SECTOR_SIZE * 3]), (START_FLASH_SECTOR + 3)*W25Q64JVS_SECTOR_SIZE, W25Q64JVS_SECTOR_SIZE);		
}

/**
  * @brief  Write a double word at the given address in Flash
  * @param  Address Where to write
  * @param  Data What to write
  * @retval EE_Status
  *           - EE_OK: on success
  *           - EE_WRITE_ERROR: if an error occurs
  */
EE_Status FI_WriteDoubleWord(uint32_t Address, uint64_t Data)
{
	uint8_t *Dat = (uint8_t*)&Data;
	uint32_t Flash_addres = Address - START_PAGE_ADDRESS + START_FLASH_SECTOR;
	BSP_QSPI_Write(Dat, Flash_addres, 8);
	vTaskDelay(1);
	read_flash_to_ram_emmul_eeprom();
	return EE_OK;//HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address, Data); 
}

/**
  * @brief  Erase a page in polling mode
  * @param  Page Page number
  * @param  NbPages Number of pages to erase
  * @retval EE_Status
  *           - EE_OK: on success
  *           - EE error code: if an error occurs
  */
EE_Status FI_PageErase(uint32_t Page, uint16_t NbPages)
{
  EE_Status status = EE_OK;
	for (uint16_t CntPages = 0; CntPages < NbPages; CntPages++)
	{
		BSP_QSPI_EraseBlock((Page + CntPages)*W25Q64JVS_SECTOR_SIZE, W25Q64JVS_ERASE_4K);
		vTaskDelay(500);		
	}
	read_flash_to_ram_emmul_eeprom();
  return status;
}

/**
  * @brief  Erase a page with interrupt enabled
  * @param  Page Page number
  * @param  NbPages Number of pages to erase
  * @retval EE_Status
  *           - EE_OK: on success
  *           - EE error code: if an error occurs
  */
EE_Status FI_PageErase_IT(uint32_t Page, uint16_t NbPages)
{
	return FI_PageErase(Page, NbPages);
}

/**
  * @brief  Flush the caches if needed to keep coherency when the flash content is modified
  */
void FI_CacheFlush()
{
  /* No flush needed. */
  return;
}

/**
  * @brief  Delete corrupted Flash address, can be called from NMI. No Timeout.
  * @param  Address Address of the FLASH Memory to delete
  * @retval EE_Status
  *           - EE_OK: on success
  *           - EE error code: if an error occurs
  */
EE_Status FI_DeleteCorruptedFlashAddress(uint32_t Address)
{
  EE_Status status = EE_DELETE_ERROR;
  return status;
}

/**
  * @brief  Check if the configuration is 128-bits bank or 2*64-bits bank
  * @param  None
  * @retval EE_Status
  *           - EE_OK: on success
  *           - EE error code: if an error occurs
  */
EE_Status FI_CheckBankConfig(void)
{
#if defined (FLASH_DBANK_SUPPORT)
  EE_Status status;

  /* Check the value of the DBANK user option byte */
  if (OB_USER_DUALBANK_ENABLE)
  {
    status = EE_OK;
  }
  else
  {
    status = EE_INVALID_BANK_CFG;
  }

  return status;
#else
  /* No feature 128-bits single bank, so always 64-bits dual bank */
  return EE_OK;
#endif
}

#include "CRC16.h"
/* Структура подсчета CRC */
Table crc16_table;

/**
  * @brief  This function configures CRC Instance.
  * @param  None
  * @retval None
  */
void ConfigureCrc(void)
{
	/* Формируем таблицу для заданного алгоритма. */
	MakeTable(&CRC16_ARC, &crc16_table);
}

/**
  * @brief  This function performs CRC calculation on Data and Virtual Address.
  * @param  Data value of  the eeprom variable.
  * @param  VirtAddress address of the eeprom variable.
  * @retval 16-bit CRC value computed on Data and Virtual Address.
  */
uint16_t CalculateCrc(EE_DATA_TYPE Data, uint16_t VirtAddress)
{
	uint8_t mas[sizeof(EE_DATA_TYPE) + sizeof(uint16_t)];	
	*((uint16_t*)&(mas[0])) = VirtAddress;
	*((EE_DATA_TYPE*)&(mas[sizeof(uint16_t)])) = Data;
	return ChecksumCRC(mas, sizeof(mas), &crc16_table);
}


/**
  * @brief  Read 32bits variable value.
  * @param  uint16_t VirtAddress
  * @retval uint32_t - 32bits read variable value
  */
uint32_t LoadEmuleEEPROM(uint16_t VirtAddress) 
{
	uint32_t TempVar; 
	EE_ReadVariable32bits(VirtAddress, &TempVar);
	return TempVar;
}

/**
  * @brief  Store 32bits variable value.
  * @param  uint16_t VirtAddress
  * @param  uint32_t StoreData - 32bits read variable value
  * @retval none
  */
void StoreEmuleEEPROM(uint16_t VirtAddress, uint32_t StoreData) 
{
	EE_WriteVariable32bits(VirtAddress, StoreData);
}

/**
  * @brief  Read 32bits variable value.
  * @param  uint16_t VirtAddress - виртуальный адрес переменой
  * @param  uint32_t VarDefault  - значение по умолчанию 
  * @retval uint32_t - 32bits read variable value
  */
uint32_t StartLoadEmuleEEPROM(uint16_t VirtAddress, uint32_t VarDefault) 
{
	uint32_t TempVar; 
	
	if ( EE_ReadVariable32bits(VirtAddress, &TempVar) != EE_OK )
	{
		EE_WriteVariable32bits(VirtAddress, VarDefault);
	    EE_ReadVariable32bits(VirtAddress, &TempVar);		
	}

	return TempVar;	
}

/**
  * @brief  This function init emul_eeprom.
  * @param  None
  * @retval uin8_t 0 - EE_OK: on success
  *                !0 - EE error code: if an error occurs
  */
uint8_t Init_Emul_EEPROM(void)
{
	EE_Status ee_status = EE_OK;
	
	/* инициализация QSPI FLASH */
	qFlashInit();
	
	/*Init RAM Emul EEPROM */
	read_flash_to_ram_emmul_eeprom();

	/*Init core soft Emul EEPROM */
	ee_status = EE_Init(EE_FORCED_ERASE);
	if (ee_status != EE_OK) {
		printf(" Eeprom init error!!! \n");
	}
	else
	{
		printf(" Eeprom init OK! \n");
	}

	return 0;
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
