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

#ifdef __cplusplus
 extern "C" {
#endif
	 
/* Includes ------------------------------------------------------------------*/
#include "test_cmd_disk.h"

/**
  * @brief  Функция парсинга и отработки команды тестирования spi_flash
  * @param  cmd_box_disk_t* cmdbox - указатель на пакет команды
  * @retval None
  */
void parsing_cmd_ram_box(cmd_box_disk_t* cmdbox);	 

	 
#ifdef __cplusplus
}
#endif

#endif /* __TEST_QSPI_FLASH_H */