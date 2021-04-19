/**
  ******************************************************************************
  * @file    ram_disk.h
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RAM_DISK_H
#define __RAM_DISK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"	
	
#define RAM_SECTOR_SIZE (4096)	
#define RAM_DISK_SIZE   (524288)	
	
/**
  * @brief  Инициализация диска в RAM.
  * @param  None
  * @retval None
  */
void ram_disk_init(void);	

/**
  * @brief  Reads an amount of data from the ram memory.
  * @param  pData     Pointer to data to be read
  * @param  ReadAddr  Read start address
  * @param  Size      Size of data to read
  * @retval BSP status
  */
int32_t ram_read(uint8_t *pData, uint32_t ReadAddr, uint32_t Size);

/**
  * @brief  Writes an amount of data to the ram memory.
  * @param  pData      Pointer to data to be written
  * @param  WriteAddr  Write start address
  * @param  Size       Size of data to write
  * @retval BSP status
  */
int32_t ram_write(uint8_t *pData, uint32_t WriteAddr, uint32_t Size);

/**
  * @brief  Erases the specified block of the ram memory.
  * @param  BlockAddress Block address to erase
  * @param  BlockSize    Erase Block size
  * @retval BSP status
  */
int32_t ram_erase_block(uint32_t BlockAddress, uint32_t BlockSize);

/**
  * @brief  Erases the entire ram memory.
  * @retval BSP status
  */
int32_t ram_erease_chip(void);	

#ifdef __cplusplus
}
#endif

#endif /* __RAM_DISK_H */
/******************* (C) COPYRIGHT 2021 OneTiOne  *****END OF FILE****/
