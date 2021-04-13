/**
  ******************************************************************************
  * @file    printf_dbg.h
  * @version V1.7.0
  * @date    12-04-2021
  * @brief   Перенаправление библиотечной C-функции printf.
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 OneTiOne </center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PRINTF_DBG_H
#define __PRINTF_DBG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include <stdio.h>	
	
/**
  * @brief  Инициализация аппаратной части отладки
  * @param  None
  * @retval None
  */
void hal_debug_uart_init(void);

#ifdef __cplusplus
}
#endif
	
#endif /* __PRINTF_DBG_H */

/******************* (C) COPYRIGHT 2020 OneTiOne  *****END OF FILE****/
