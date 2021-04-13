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

/**
  * @brief  Передача одного символа в UART.
  * @param  char data - транслируемый символ
  * @retval None
  */
void send_uart(char data);

/**
 * @brief Перенаправление библиотечной C-функции printf на USART.
 */
int _write(int file, char *data, int len)
{
	len = len;	
	send_uart(data[0]);
	return 1;
}
/******************* (C) COPYRIGHT 2020 OneTiOne  *****END OF FILE****/
