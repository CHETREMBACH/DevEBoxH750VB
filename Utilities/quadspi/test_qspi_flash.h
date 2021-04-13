/**
  ******************************************************************************
  * File Name          : test_qspi_flash.h
  * Description        : This file provides code for the driver QUADSPI Flash.
  * 
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TEST_QSPI_FLASH_H
#define __TEST_QSPI_FLASH_H

#include "main.h"

#ifdef __cplusplus
 extern "C" {
#endif
	 
/* Includes ------------------------------------------------------------------*/

/* перечисление для типа параметра команды */
typedef enum
{ 
	 CMD_CHIP   = 0,
	 CMD_SECTOR,
	 CMD_FATFS,
	 CMD_DAMP
}id_cmd_qflash_e;

/* перечисление для типа режима команды */
typedef enum
{ 
	 MODE_STATUS = 0,
	 MODE_PARAM,
	 MODE_FORMAT,
	 MODE_MOUNT,
	 MODE_WRITE,
	 MODE_READ,
	 MODE_VIEW,
	 MODE_COMPARE,
	 MODE_FILL,
	 MODE_RND,
	 MODE_INC,
	 MODE_DEC,
	 MODE_ERASE,
	 MODE_INFO
}id_mode_qflash_e;	 

/* формат сообщения команды формирования дампа */
typedef struct
{ 
	 id_mode_qflash_e  id_mode;
	 uint8_t  code_fill;
}cmd_damp_qflash_t;	 

/* формат сообщения команд работы с fatfs */
typedef struct
{
	 id_mode_qflash_e  id_mode;
	 id_mode_qflash_e  id_mode_fill;
}cmd_fatfs_qflash_t;	 

/* формат сообщения команд работы с сектором */
typedef struct
{  
	 id_mode_qflash_e  id_mode;
	uint32_t  number;
	  
}cmd_sector_qflash_t;		 

/* формат сообщения команд работы с чипом */
typedef struct
{  
	 id_mode_qflash_e  id_mode;
}cmd_chip_qflash_t;		 

/* формат сообщения команды  */
typedef struct
{
	 id_cmd_qflash_e id_cmd_qflash;
	 union
	 {
		 cmd_damp_qflash_t cmd_damp;	 
		 cmd_fatfs_qflash_t cmd_fatfs;
		 cmd_sector_qflash_t cmd_sector;
		 cmd_chip_qflash_t cmd_chip;		 
	 };
}cmd_box_qflash_t;	 

/**
  * @brief  Функция парсинга и отработки команды тестирования spi_flash
  * @param  cmd_box_qflash_t* cmdbox - указатель на пакет команды
  * @retval None
  */
void parsing_cmd_box(cmd_box_qflash_t* cmdbox);	 
	 
/**
  * @brief  Функция визуализации содержимого блока памяти размером 256 байт
  * @param  uint8_t* buff_damp - указатель на буфер дампа 
  * @param  uint32_t damp_size - размер буфера  
  * @param  uint32_t addr_mem - непосредственый адрес памяти данных 
  * @retval void
  */
void BlocDampPrint(uint8_t* buff_damp, uint32_t damp_size, uint32_t addr_mem);
	 
#ifdef __cplusplus
}
#endif

#endif /* __TEST_QSPI_FLASH_H */