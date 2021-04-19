/**
  ******************************************************************************
  * @file    ram_disk.c
  * @version V1.0.0
  * @date    07-09-2020
  * @brief   Инициализация драйвера ram_drv 
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2021 OneTiOne </center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ram_disk.h"
#include "printf_dbg.h"
#include "stm32h7xx_hal.h"

/* Allocate the memory for the heap. */
uint8_t __attribute__((section(".RAM_DISK"))) damp_disk[RAM_DISK_SIZE];	

/**
  * @brief  Инициализация диска в RAM.
  * @param  None
  * @retval None
  */
void ram_disk_init(void)
{
	/* clear ram disk */
	for (uint32_t cnt_byte = 0; cnt_byte < RAM_DISK_SIZE; cnt_byte++)
	{
		damp_disk[cnt_byte] = 0xFF;
	}
}

/**
  * @brief  Reads an amount of data from the ram memory.
  * @param  pData     Pointer to data to be read
  * @param  ReadAddr  Read start address
  * @param  Size      Size of data to read
  * @retval BSP status
  */
int32_t ram_read(uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
	int32_t ret = 0;
	for (uint32_t cnt_byte = 0; cnt_byte < Size; cnt_byte++)
	{
		pData[cnt_byte] = damp_disk[ReadAddr + cnt_byte];
	}	
	return ret;
}

/**
  * @brief  Writes an amount of data to the ram memory.
  * @param  pData      Pointer to data to be written
  * @param  WriteAddr  Write start address
  * @param  Size       Size of data to write
  * @retval BSP status
  */
int32_t ram_write(uint8_t *pData, uint32_t WriteAddr, uint32_t Size)
{
	int32_t ret = 0;
	for (uint32_t cnt_byte = 0; cnt_byte < Size; cnt_byte++)
	{
	  damp_disk[WriteAddr + cnt_byte] = pData[cnt_byte];
	}	
	/* Return BSP status */
	return ret;
}

/**
  * @brief  Erases the specified block of the ram memory.
  * @param  BlockAddress Block address to erase
  * @param  BlockSize    Erase Block size
  * @retval BSP status
  */
int32_t ram_erase_block(uint32_t BlockAddress, uint32_t BlockSize)
{
	int32_t ret = 0;
	for (uint32_t cnt_byte = 0; cnt_byte < BlockSize; cnt_byte++)
	{
		damp_disk[BlockAddress + cnt_byte] = 0xff;
	}
	/* Return BSP status */
	return ret;
}

/**
  * @brief  Erases the entire ram memory.
  * @retval BSP status
  */
int32_t ram_erease_chip(void)
{
	int32_t ret = 0;
	ram_disk_init();
	/* Return BSP status */
	return ret;
}	

/******************* (C) COPYRIGHT 2020 OneTiOne  *****END OF FILE****/
