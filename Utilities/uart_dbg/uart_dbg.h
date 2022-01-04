/**
  ******************************************************************************
  * @file    uart_dbg.h
  * @version V1.7.0
  * @date    12-04-2021
  * @brief   Инициализация драйвера для запуска UART в режиме отладки
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 OneTiOne </center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART_DBG_H
#define __UART_DBG_H

#ifdef __cplusplus
extern "C" {
#endif

#if ( DBG_UART_ENABLE == 1 )
	
/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx.h"
#include "stm32h7xx_ll_usart.h"
#include "stm32h7xx_ll_rcc.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_bus.h"
	
	
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define DBG_BaudRate     	            921600 //1843200 //115200 // 10800000 //460800 // 230400 //38400 //	
	
/* Размер буфера диагностических сообщений */
#define DBG_UART_MAX_SIZE_BUFF       (8000) 
/* Буфер для диагностического сообщения */
extern volatile uint8_t buf_dbg[DBG_UART_MAX_SIZE_BUFF];
	/* Индекс записи сообщения */
extern 	uint16_t index_wr_buf_mes;
	/* Индекс чтения сообщения */
extern 	volatile uint16_t index_rd_buf_mes;

	/* Размер буфера приема команд */
#define DBG_UART_MAX_SIZE_CMD_BUFF   (50)
	/* Буфер для приема команды */
extern 	volatile uint8_t buf_cmd[DBG_UART_MAX_SIZE_CMD_BUFF];
	/* Индекс записи команды */
extern 	volatile uint16_t index_wr_buf_cmd;
	/* Индекс чтения команды */
extern 	uint16_t index_rd_buf_cmd;
	
	
/**
  * @brief  Передача одного символа в UART.
  * @param  char* data - транслируемый символ
  * @retval None
  */
__STATIC_INLINE void send_uart(char* data)
{
	if (index_rd_buf_mes == index_wr_buf_mes)
	{
		/* Данных в буфере нет - включение передачи */
		/* Загружаем символ в буфер */    
		buf_dbg[index_wr_buf_mes] = *data; 

		/* Проверка на переполнение  */
		if (index_wr_buf_mes < (DBG_UART_MAX_SIZE_BUFF - 1))
		{
			/* Увеличение индекса */
			index_wr_buf_mes++;
		} 
		else
		{
			/* Организация циклического буфера */  
			index_wr_buf_mes = 0;    
		}    

		/* включаем прерывание по передаче */
		LL_USART_EnableIT_TXE(USART3); 

	}
	else
	{
		/* Есть данные загружаем данные и инкрементируем индекс */
		/* Загружаем символ в буфер */    
		buf_dbg[index_wr_buf_mes] = *data;  
		/* Проверка на переполнение  */
		if (index_wr_buf_mes < (DBG_UART_MAX_SIZE_BUFF - 1))
		{
			/* Увеличение индекса */
			index_wr_buf_mes++;
		} 
		else
		{
			/* Организация циклического буфера */  
			index_wr_buf_mes = 0;    
		}        
	}
}
	
/**
  * @brief  Инициализация аппаратной части uart отладки
  * @param  None
  * @retval None
  */
void hal_debug_uart_init(void);	
	
#endif /* DBG_UART_ENABLE == 1 */

#ifdef __cplusplus
}
#endif

#endif
/******************* (C) COPYRIGHT 2020 OneTiOne  *****END OF FILE****/
