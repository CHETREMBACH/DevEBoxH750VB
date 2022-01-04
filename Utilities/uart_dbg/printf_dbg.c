/**
  ******************************************************************************
  * @file    printf_dbg.c
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

/* Includes ------------------------------------------------------------------*/
#include "printf_dbg.h"

#if  (DBG_UART_ENABLE == 1)
#include "uart_dbg.h"
#endif  /* (DBG_UART_ENABLE == 1) */  

#if  (DBG_USB_ENABLE == 1)
#include "usb_dbg.h"
#endif  /* (DBG_USB_ENABLE == 1) */  


/**
  * @brief  Инициализация аппаратной части отладки
  * @param  None
  * @retval None
  */
void dbgHardSetup(void)
{

#if  (DBG_UART_ENABLE == 1)
	/* Инициализация аппаратной части отладки по uart */
	hal_debug_uart_init();
#endif  /* (DBG_UART_ENABLE == 1) */  
  
#if  (DBG_USB_ENABLE == 1)  
	/* Функция создания сокета UDP для отладки */
	usbDebugInit();
#endif  /* (DBG_USB_ENABLE == 1) */  
}


/**
 * @brief Перенаправление библиотечной C-функции printf на USART.
 */
int _write(int file, char *data, int len)
{
	len = len;	
#if  (DBG_UART_ENABLE == 1)
	send_uart(data);
#endif  /* (DBG_UART_ENABLE == 1) */  
  
#if  (DBG_USB_ENABLE == 1)  
	send_usb(data);	
#endif  /* (DBG_UDP_ENABLE == 1) */  

	return 1;
}

/******************* (C) COPYRIGHT 2020 OneTiOne  *****END OF FILE****/
