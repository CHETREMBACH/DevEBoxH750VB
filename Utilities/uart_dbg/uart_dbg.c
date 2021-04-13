/**
  ******************************************************************************
  * @file    uart_dbg.c
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

/* Includes ------------------------------------------------------------------*/

#include "uart_dbg.h"

#if ( DBG_UART_ENABLE == 1)

#include "stm32h7xx_ll_usart.h"
#include "stm32h7xx_ll_rcc.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_bus.h"

#define DBG_BaudRate     	            921600 //1843200 //115200 // 10800000 //460800 // 230400 //38400 //

/* Размер буфера диагностических сообщений */
#define DBG_UART_MAX_SIZE_BUFF       (8000) 
/* Буфер для диагностического сообщения */
volatile uint8_t buf_dbg[DBG_UART_MAX_SIZE_BUFF];
/* Индекс записи сообщения */
uint16_t index_wr_buf_mes = 0;
/* Индекс чтения сообщения */
volatile uint16_t index_rd_buf_mes = 0;

/* Размер буфера приема команд */
#define DBG_UART_MAX_SIZE_CMD_BUFF   (50)
/* Буфер для приема команды */
volatile uint8_t buf_cmd[DBG_UART_MAX_SIZE_CMD_BUFF];
/* Индекс записи команды */
volatile uint16_t index_wr_buf_cmd = 0;
/* Индекс чтения команды */
uint16_t index_rd_buf_cmd = 0;


/**
  * @brief  Инициализация аппаратной части отладки
  * @param  None
  * @retval None
  */
void hal_debug_uart_init(void)
{
	LL_USART_InitTypeDef USART_InitStruct = { 0 };
	LL_GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	LL_RCC_SetUSARTClockSource(LL_RCC_USART234578_CLKSOURCE_PCLK1);

	/* Peripheral clock enable */
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);

	/* Peripheral clock enable */
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);

	LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOD);
	/**USART3 GPIO Configuration
	PD8   ------> USART3_TX
	PD9   ------> USART3_RX
	*/
	GPIO_InitStruct.Pin = LL_GPIO_PIN_8 | LL_GPIO_PIN_9;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
	LL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/* USART3 interrupt Init */
	NVIC_SetPriority(USART3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 5));
	NVIC_EnableIRQ(USART3_IRQn);

	USART_InitStruct.PrescalerValue = LL_USART_PRESCALER_DIV1;
	USART_InitStruct.BaudRate = 921600;
	USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
	USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
	USART_InitStruct.Parity = LL_USART_PARITY_NONE;
	USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
	USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
	USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
	LL_USART_Init(USART3, &USART_InitStruct);
	LL_USART_SetTXFIFOThreshold(USART3, LL_USART_FIFOTHRESHOLD_1_8);
	LL_USART_SetRXFIFOThreshold(USART3, LL_USART_FIFOTHRESHOLD_1_8);
	LL_USART_DisableFIFO(USART3);
	LL_USART_ConfigAsyncMode(USART3);


	LL_USART_Enable(USART3);
	
	LL_USART_EnableIT_RXNE(USART3);
	
}

/**
  * @brief  Получение  одного символа из буфера UART.
  * @param  uint8_t data - транслируемый символ
  * @retval uint8_t  !0 - есть принятые данные
  *                  0 - принятых данных нет     
  */
uint8_t recv_uart(uint8_t *data)
{
	if (index_wr_buf_cmd != index_rd_buf_cmd) {
		/* чтение */
		data[0] = buf_cmd[index_rd_buf_cmd];
		/* смещение индекса чтения */
		index_rd_buf_cmd++;
		/* Проверка на переполнение  */
		if (index_rd_buf_cmd >= DBG_UART_MAX_SIZE_CMD_BUFF) {
			/* Организация циклического буфера */
			index_rd_buf_cmd = 0;
		}	
		return 1;
	}
	else
	{
		/* сброс буфера */
		data[0] = 0;
		return 0;
	}
}

/**
  * @brief  Передача одного символа в UART.
  * @param  uint8_t data - транслируемый символ
  * @retval None
  */
void send_uart(uint8_t data)
{
	if (index_rd_buf_mes == index_wr_buf_mes)
	{
		/* Данных в буфере нет - включение передачи */
        /* Загружаем символ в буфер */    
		buf_dbg[index_wr_buf_mes] = data; 
    
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
		buf_dbg[index_wr_buf_mes] = data;  
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
  * @brief  Прерывание DBG_UART.
  * @param  None
  * @retval None
  */
void USART3_IRQHandler(void)
{
		
	if ((LL_USART_IsActiveFlag_RXNE(USART3)) & (LL_USART_IsEnabledIT_RXNE(USART3)))	
	{
		/* принимаем данные        */
		buf_cmd[index_wr_buf_cmd] =  LL_USART_ReceiveData8(USART3);
		/* нкремент индекса */ 
		index_wr_buf_cmd++;
		/* Проверка на переполнение  */
		if (index_wr_buf_cmd >= DBG_UART_MAX_SIZE_CMD_BUFF)
		{
			/* Организация циклического буфера */  
			index_wr_buf_cmd = 0;    
		}		
	}
	
	if ((LL_USART_IsActiveFlag_TXE(USART3)) & (LL_USART_IsEnabledIT_TXE(USART3)))		
	{
		/* TXE flag автоматически очищается когда записываются новые данные в TDR register */
		if (index_rd_buf_mes == index_wr_buf_mes)
		{
			/* Данных в буфере нет - отключение передачи */
		    /* отключаем прерывание по передаче */
			LL_USART_DisableIT_TXE(USART3);     
		}
		else
		{
			/* Есть данные передаем и инкрементируем индекс */
		    /* Передача байта сообщения */
			LL_USART_TransmitData8( USART3, (uint16_t)(buf_dbg[index_rd_buf_mes]));      
			/* Увеличение индекса */
			index_rd_buf_mes++;
			/* Проверка на переполнение  */
			if (index_rd_buf_mes >= DBG_UART_MAX_SIZE_BUFF)
			{
				/* Организация циклического буфера */  
				index_rd_buf_mes = 0;    
			}  
		} 
	}
}

#else
	
/**
  * @brief  Инициализация аппаратной части отладки
  * @param  None
  * @retval None
  */
void hal_debug_uart_init(void){	}
	
/**
  * @brief  Передача одного символа в UART.
  * @param  uint8_t data - транслируемый символ
  * @retval None
  */
void send_uart(uint8_t data)
{
	data = data;
}		

/**
  * @brief  Получение  одного символа из буфера UART.
  * @param  uint8_t data - транслируемый символ
  * @retval uint8_t  !0 - есть принятые данные
  *                  0 - принятых данных нет     
  */
uint8_t recv_uart(uint8_t *data)	
{
	data[0] = 0;
	return 0;
}	
	
#endif	
/******************* (C) COPYRIGHT 2020 OneTiOne  *****END OF FILE****/
