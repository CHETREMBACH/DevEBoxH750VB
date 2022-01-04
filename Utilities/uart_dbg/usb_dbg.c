/**
  ******************************************************************************
  * @file    usb_dbg.c
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

/* Includes ------------------------------------------------------------------*/


#include "usb_dbg.h"
#include "main.h"

#if ( DBG_USB_ENABLE == 1)

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

/* Буфер для диагностического сообщения */
volatile uint8_t usbBufDbg[DBG_USB_MAX_SIZE_BUFF];
/* Индекс записи сообщения */
uint16_t idxWrUsbBufDbg = 0;
/* Индекс чтения сообщения */
volatile uint16_t idxRdUsbBufDbg = 0;

/* Буфер для приема команды */
volatile uint8_t usbBufCmd[DBG_USB_MAX_SIZE_CMD_BUFF];
/* Индекс записи команды */
volatile uint16_t idxWrUsbBufCmd = 0;
/* Индекс чтения команды */
uint16_t idxRdUsbBufCmd = 0;

#define USBDB_NOTE  (1l<<0) 

static uint32_t    usbDebugNotifiedValue; 
TimerHandle_t      usbDebugSoftTimer;
TaskHandle_t       usbDebugHandleTask;

/**
  * @brief  Получение  одного символа из буфера USB.
  * @param  uint8_t data - транслируемый символ
  * @retval uint8_t  !0 - есть принятые данные
  *                  0 - принятых данных нет     
  */
uint8_t recv_usb(uint8_t *data)
{
	if (idxWrUsbBufCmd != idxRdUsbBufCmd) 
	{
		/* чтение */
		data[0] = usbBufCmd[idxRdUsbBufCmd];
		/* смещение индекса чтения */
		idxRdUsbBufCmd++;
		/* Проверка на переполнение  */
		if (idxRdUsbBufCmd >= DBG_USB_MAX_SIZE_CMD_BUFF)
		{
			/* Организация циклического буфера */
			idxRdUsbBufCmd = 0;
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
  * @brief  Прием данных из USB.
  * @param  none
  * @retval none 
  */
void usbDebugRecv(uint8_t* Buf, uint32_t *Len)
{
	uint32_t idxBuf = 0;  
	
	while (idxBuf < *Len)
	{
		/* принимаем данные        */
		usbBufCmd[idxWrUsbBufCmd] = Buf[idxBuf];
		/* нкремент индекса */ 
		idxWrUsbBufCmd++;
		idxBuf++;
		/* Проверка на переполнение  */
		if (idxWrUsbBufCmd >= DBG_USB_MAX_SIZE_CMD_BUFF)
		{
			/* Организация циклического буфера */  
			idxWrUsbBufCmd = 0;    
		}
	}
}

/**
  * @brief  Функция отработки програмного таймера
  * @param  TimerHandle_t pxTimer - указатель на таймер вызвавщий функцию
  * @retval None
  */
void usbDebugTimCallback(TimerHandle_t pxTimer)
{
	/* Функция обработки програмного таймера.*/
	if ((usbDebugHandleTask) != NULL)
	{
		if (usbDebugSoftTimer == pxTimer)
		{
			/* Таймер контроля внешнего сторожевого таймера */
			xTaskNotify( usbDebugHandleTask,       /* Указатель на уведомлюемую задачу                         */
				USBDB_NOTE,            /* Значения уведомления                                     */
				eSetBits); /* Текущее уведомление добавляются к уже прописанному       */
		}
	}  
}

/**
  * @brief  задача отладки/терминала через USB
  * @param  pvParameters not used
  * @retval None
  */
void usbDebugTask(void * pvParameters)
{  
	/* Открытие програмный таймер  */
	usbDebugSoftTimer = xTimerCreate( "USBDBTIM",      /* Текстовое имя, не используется в RTOS kernel. */
		10,                   /* Период таймера в тиках. */
		pdTRUE,               /* Время будет автоматически перезагружать себя, когда оно истечет. */
		NULL,                 /* В функцию параметры не передаем */
		usbDebugTimCallback); /* Указатель на функцию , которую вызывает таймер. */  
  
	xTimerStart(usbDebugSoftTimer, 0);   
  
	for (;;)
	{
		/* Обнуляем сообщение */
		usbDebugNotifiedValue = 0;
		/*================================== Проверка наличия сообщений ========================================*/
		xTaskNotifyWait(0x00000000,          /* Не очищайте биты уведомлений при входе               */
			0xFFFFFFFF,                      /* Сбросить значение уведомления до 0 при выходе        */
			&(usbDebugNotifiedValue),        /* Значение уведомленное передается в  NotifiedValue    */
			portMAX_DELAY); /* Блокировка задачи до появления уведомления           */
    
        /* Получено уведомление. Проверяем, какие биты были установлены. */
		if (((usbDebugNotifiedValue) & USBDB_NOTE) != 0)
		{  
            /* если есть данные в буфере отправляем */ 
			if (idxRdUsbBufDbg != idxWrUsbBufDbg)
			{
			    /* проверка есть ли переход через ноль */
				if (idxRdUsbBufDbg > idxWrUsbBufDbg)
				{
					CDC_Transmit_FS((uint8_t*)&(usbBufDbg[idxRdUsbBufDbg]), (DBG_USB_MAX_SIZE_BUFF - idxRdUsbBufDbg));
					idxRdUsbBufDbg = 0;
				}
				
				CDC_Transmit_FS((uint8_t*)&(usbBufDbg[idxRdUsbBufDbg]), (idxWrUsbBufDbg - idxRdUsbBufDbg));
				idxRdUsbBufDbg = idxWrUsbBufDbg;
			}
		}    
	} 
}

/**
  * @brief  Инициализация задачи контроля терминала отладки USB
  * @param  None
  * @retval None
  */
void usbDebugInit(void)
{
	/* Инициализация апаратный ресурсов USB  */	
	MX_USB_DEVICE_Init();	
	/* Запуск задачи */
	xTaskCreate(usbDebugTask, "USBDB", configMINIMAL_STACK_SIZE, NULL, TreadPrioLow, &usbDebugHandleTask);  	 
}

#endif	
/******************* (C) COPYRIGHT 2022 OneTiOne  *****END OF FILE****/
