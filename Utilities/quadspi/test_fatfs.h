/**
  ******************************************************************************
  * File Name          : test_fatfs.h
  * Description        : This file provides test FatFS code.
  * 
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TEST_FATFS_H
#define __TEST_FATFS_H

#include "main.h"

#ifdef __cplusplus
 extern "C" {
#endif
	 
/* Includes ------------------------------------------------------------------*/
	 
/**
  * @brief  Форматирование диска.
  * @param  None 
  * @retval None
  */
void CmdFmkfs(void);

/**
  * @brief  Форматирование диска.
  * @param  None 
  * @retval None
  */
void CmdFmount(void);

/**
  * @brief  Параметры диска.
  * @param  None 
  * @retval None
  */
void CmdParam(void);

/**
  * @brief  Статус диска.
  * @param  None 
  * @retval None
  */
void CmdStatus(void);

/**
  * @brief  Запись файла.
  * @param  None 
  * @retval None
  */
void CmdFwrite(void);

/**
  * @brief  Чтение файла.
  * @param  None 
  * @retval None
  */
void CmdFread(void);

	 
#ifdef __cplusplus
}
#endif

#endif /* __TEST_FATFS_H */