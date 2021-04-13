/**
  ******************************************************************************
  * File Name          : task_qspi_flash.h
  * Description        : This file provides code for the driver QUADSPI Flash.
  * 
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TASK_QSPI_FLASH_H
#define __TASK_QSPI_FLASH_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#define QFLASH_NOTE_SOFT_TIM   (uint32_t)(1l<<1)	 
#define QFLASH_NOTE_DELAY_TIM  (uint32_t)(1l<<2)
	 
	 
/**
  * @brief  инициализация QSPI FLASH
  * @param  None
  * @retval None
  */
void qFlashInit(void); 
	 
#ifdef __cplusplus
}
#endif

#endif /* __TASK_QSPI_FLASH_H */