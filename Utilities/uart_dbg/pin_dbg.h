/**
  ******************************************************************************
  * @file    pin_dbg.h
  * @version V1.0.0
  * @date    07-09-2020
  * @brief   Инициализация драйвера GPIO для отладки
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 OneTiOne </center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PIN_DBG_H
#define __PIN_DBG_H

#ifdef __cplusplus
extern "C" {
#endif

#if  (DBG_PIN_ENABLE == 1)

/* Includes ------------------------------------------------------------------*/
#include "main.h"	
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

#define T1_CLK_ENABLE()   __HAL_RCC_GPIOD_CLK_ENABLE()              	
#define T1_PIN            GPIO_PIN_0
#define T1_PORT           GPIOD	
#define T1_HI             T1_PORT->BSRR = T1_PIN
#define T1_LO             T1_PORT->BSRR = (uint32_t)T1_PIN << (16U);

#define T2_CLK_ENABLE()   __HAL_RCC_GPIOD_CLK_ENABLE()            	
#define T2_PIN            GPIO_PIN_1
#define T2_PORT           GPIOD	
#define T2_HI             T2_PORT->BSRR = T2_PIN
#define T2_LO             T2_PORT->BSRR = (uint32_t)T2_PIN << (16U);

#define T3_CLK_ENABLE()   __HAL_RCC_GPIOD_CLK_ENABLE()           	
#define T3_PIN            GPIO_PIN_2
#define T3_PORT           GPIOD	
#define T3_HI             T3_PORT->BSRR = T3_PIN
#define T3_LO             T3_PORT->BSRR = (uint32_t)T3_PIN << (16U);
	
#define T4_CLK_ENABLE()   __HAL_RCC_GPIOD_CLK_ENABLE()        	
#define T4_PIN            GPIO_PIN_3
#define T4_PORT           GPIOD	
#define T4_HI             T4_PORT->BSRR = T4_PIN
#define T4_LO             T4_PORT->BSRR = (uint32_t)T4_PIN << (16U);

#define T5_CLK_ENABLE()   __HAL_RCC_GPIOD_CLK_ENABLE()    	
#define T5_PIN            GPIO_PIN_4
#define T5_PORT           GPIOD	
#define T5_HI             T5_PORT->BSRR = T5_PIN
#define T5_LO             T5_PORT->BSRR = (uint32_t)T5_PIN << (16U);

#define T6_CLK_ENABLE()   __HAL_RCC_GPIOD_CLK_ENABLE()      	
#define T6_PIN            GPIO_PIN_5
#define T6_PORT           GPIOD	
#define T6_HI             T6_PORT->BSRR = T6_PIN
#define T6_LO             T6_PORT->BSRR = (uint32_t)T6_PIN << (16U);

#define T7_CLK_ENABLE()   __HAL_RCC_GPIOD_CLK_ENABLE()           	
#define T7_PIN            GPIO_PIN_6
#define T7_PORT           GPIOD	
#define T7_HI             T7_PORT->BSRR = T7_PIN
#define T7_LO             T7_PORT->BSRR = (uint32_t)T7_PIN << (16U);
	
#define T8_CLK_ENABLE()   __HAL_RCC_GPIOD_CLK_ENABLE()           	
#define T8_PIN            GPIO_PIN_7
#define T8_PORT           GPIOD	
#define T8_HI             T8_PORT->BSRR = T8_PIN
#define T8_LO             T8_PORT->BSRR = (uint32_t)T8_PIN << (16U);
	
/**
  * @brief  Инициализация GPIO для отладки.
  * @param  None
  * @retval None
  */
void hal_debug_pin_init(void);

#endif /*  (DBG_PIN_ENABLE == 1) */

#ifdef __cplusplus
}
#endif

#endif /* __PIN_DBG_H */

/******************* (C) COPYRIGHT 2020 OneTiOne  *****END OF FILE****/
