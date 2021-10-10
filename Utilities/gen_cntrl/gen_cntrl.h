/**
  ******************************************************************************
  * @file    gen_cntrl.h
  * @author  Trembach D.N.
  * @version V1.0.0
  * @date    10-10-2021
  * @brief   Файл содержит функции контроля генератора сигнала на выводе MCO2 PC9 
  *
  *
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; COPYRIGHT 2021 OneTiOne < / center>< / h2>
  ******************************************************************************
  */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GEN_CNTRL_H
#define __GEN_CNTRL_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/**
  * @brief  Инициализация и запуск задачи контроля генератора.
  * @param  None 
  * @retval None
  */
void InitCntrlGenTask(void);

/**
  * @brief  Инициализация указателя на команды управления генератором
  * @param  none
  * @retval none
  */
void generator_cmd_init();

#endif /* __GEN_CNTRL_H */
/************************ (C) COPYRIGHT ONE TO ONE *****END OF FILE****/
