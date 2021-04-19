 /**
  ******************************************************************************
  * @file    cmd_ram_disk.h
  * @author  Trembach D.N.
  * @version V1.0.0
  * @date    19-04-2021
  * @brief   файл функций обработки команд управления и тестирования ram disk  
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2021 OneTiOne </center></h2>
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CMD_RAM_DISK
#define __CMD_RAM_DISK

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "cmd_process.h"

/**
  * @brief  Инициализация указателя на команды управления и тестирования ram  
  * @param  none
  * @retval none
  */
void ram_disk_cmd_init();

#ifdef __cplusplus
}
#endif

#endif /* __CMD_RAM_DISK */

/******************* (C) COPYRIGHT 2021 OneTiOne  *****END OF FILE****/

