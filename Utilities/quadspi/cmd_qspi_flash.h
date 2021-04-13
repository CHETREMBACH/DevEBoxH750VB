 /**
  ******************************************************************************
  * @file    cmd_qspi_flash.h
  * @author  Trembach D.N.
  * @version V1.0.0
  * @date    27-09-2020
  * @brief   файл функций обработки команд управления и тестирования qspi ram  
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 OneTiOne </center></h2>
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CMD_QSPI_FLASH
#define __CMD_QSPI_FLASH

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "cmd_process.h"

/**
  * @brief  Инициализация указателя на команды управления и тестирования qspi ram  
  * @param  none
  * @retval none
  */
void qspi_flash_cmd_init();

#ifdef __cplusplus
}
#endif

#endif /* __CMD_QSPI_FLASH */

/******************* (C) COPYRIGHT 2020 OneTiOne  *****END OF FILE****/

