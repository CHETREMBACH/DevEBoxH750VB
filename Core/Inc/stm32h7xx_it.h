/**
  ******************************************************************************
  * @file    stm32h7xx_it.h
  * @author  Trembach D.N.
  * @version V1.0.0
  * @date    23-08-2020
  * @brief   This file contains the headers of the interrupt handlers.
  *
  *
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; COPYRIGHT 2020 OneTiOne </center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32H7xx_IT_H
#define __STM32H7xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Private includes ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void DebugMon_Handler(void);
void SysTick_Handler(void);


#ifdef __cplusplus
}
#endif

#endif /* __STM32H7xx_IT_H */
/************************ (C) COPYRIGHT 2020 OneTiOne *****END OF FILE****/
