/**
  ******************************************************************************
  * @file    usb_dbg.h
  * @version V1.1.0
  * @date    03-01-2022
  * @brief   Инициализация драйвера отладки/терминала через USB
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2022 OneTiOne </center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_DBG_H
#define __USB_DBG_H

#ifdef __cplusplus
extern "C" {
#endif

#if ( DBG_USB_ENABLE == 1)	
	
/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx.h"
	
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

	
/* Размер буфера диагностических сообщений */
#define DBG_USB_MAX_SIZE_BUFF       (8000) 
/* Буфер для диагностического сообщения */
extern volatile uint8_t usbBufDbg[DBG_USB_MAX_SIZE_BUFF];
/* Индекс записи сообщения */
extern 	uint16_t idxWrUsbBufDbg;
/* Индекс чтения сообщения */
extern 	volatile uint16_t idxRdUsbBufDbg;

/* Размер буфера приема команд */
#define DBG_USB_MAX_SIZE_CMD_BUFF   (50)
/* Буфер для приема команды */
extern 	volatile uint8_t usbBufCmd[DBG_USB_MAX_SIZE_CMD_BUFF];
/* Индекс записи команды */
extern 	volatile uint16_t idxWrUsbBufCmd;
/* Индекс чтения команды */
extern 	uint16_t idxRdUsbBufCmd;
	
/**
  * @brief  Передача одного символа в USB.
  * @param  char* data - транслируемый символ
  * @retval None
  */
__STATIC_INLINE void send_usb(char* data)
{
   /* Загружаем символ в буфер */    
   usbBufDbg[idxWrUsbBufDbg] = *data;  
   /* Проверка на переполнение  */
   if (idxWrUsbBufDbg < (DBG_USB_MAX_SIZE_BUFF - 1))
   {
   	/* Увеличение индекса */
   	idxWrUsbBufDbg++;
   } 
   else
   {
   	/* Организация циклического буфера */  
   	idxWrUsbBufDbg = 0;    
   }        
}	
	
/**
  * @brief  Инициализация задачи контроля терминала отладки USB
  * @param  None
  * @retval None
  */
void usbDebugInit(void);	
	
#endif 	/* DBG_USB_ENABLE == 1 */
	
#ifdef __cplusplus
}
#endif

#endif /* __USB_DBG_H */
/******************* (C) COPYRIGHT 2022 OneTiOne  *****END OF FILE****/
