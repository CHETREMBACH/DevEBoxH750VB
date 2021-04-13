/**
  ******************************************************************************
  * File Name          : task_qspi_flash.c
  * Description        : This file provides code for the driver QUADSPI Flash.
  * 
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "drv_qspi_w25q64.h"
#include "task_qspi_flash.h"
#include "test_fatfs.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "main.h"
#include "pin_dbg.h"
#include "test_qspi_flash.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static uint32_t    qFlashNotifiedValue;     
TaskHandle_t       qFlashHandleTask;        
TimerHandle_t      qFlashxSoftTimer;            /*  Програмный таймер периодического контроля     */
TimerHandle_t      qFlashxUpdateTimer;          /*  Програмный таймер контроля ожидания ресурсов  */
QueueHandle_t      xCmdQueue;
cmd_box_qflash_t   cmd_box;

uint8_t data_buf[256];

/**
 * @brief  функция отправки сообщения команды
 * @param  cmd_box_qflash_t *cmd_box_qflash - указатель на сообщение команды
 * @retval none
 */
void send_cmd_box(cmd_box_qflash_t *cmd_box_qflash)
{
	/* Проверка очереди */
	if (xCmdQueue != NULL)
	{
		xQueueSend(xCmdQueue, cmd_box_qflash, (TickType_t) 0);
	}	
}

/**
  * @brief  QSPI_FLASH control - вызывается с частотой 10 Гц
  * @param  None
  * @retval None
  */
void qFlashCntrl(void)
{
	/* Проверка очереди */
	if (xCmdQueue != NULL)
	{
		if ( xQueueReceive(xCmdQueue, &cmd_box, (TickType_t) 0) == pdPASS)
		{
			/* Получена команда */
			parsing_cmd_box(&cmd_box);
		}
	}
}

/**
  * @brief  Функция отработки програмного таймера 
  * @param  TimerHandle_t pxTimer - указатель на таймер вызвавщий функцию
  * @retval None
  */
void qFlashTimCallback(TimerHandle_t pxTimer)
{
	if ((qFlashHandleTask) != NULL)
	{
		if (qFlashxSoftTimer == pxTimer)
		{/* Програмный таймер периодического контроля */
			xTaskNotify( qFlashHandleTask,				/* Указатель на уведомлюемую задачу                         */
					  QFLASH_NOTE_SOFT_TIM,				/* Значения уведомления                                     */
					  eSetBits );                       /* Текущее уведомление добавляются к уже прописанному       */
		}
    
		if (qFlashxUpdateTimer == pxTimer)
		{/* Таймер контроля ожидания ресурсов */
			xTaskNotify( qFlashHandleTask,				/* Указатель на уведомлюемую задачу                         */
					  QFLASH_NOTE_DELAY_TIM,            /* Значения уведомления                                     */
					  eSetBits );                       /* Текущее уведомление добавляются к уже прописанному       */
		}  
	}  
}

/**
  * @brief  задача сторожевого таймера
  * @param  pvParameters not used
  * @retval None
  */
void qFlash_Task(void * pvParameters)
{  
	/* Открытие очереди для получения команд */
	if (xCmdQueue == NULL)
	{
		xCmdQueue = xQueueCreate(10, sizeof(cmd_box_qflash_t));
	}
	
	/* Открытие таймера периодического контроля  */
	qFlashxSoftTimer = xTimerCreate( "QTmPrd",	/* Текстовое имя, не используется в RTOS kernel. */
                                  100,		    /* Период таймера в тиках. */
                                  pdTRUE,		/* Время будет автоматически перезагружать себя, когда оно истечет. */
                                  NULL,		/* В функцию параметры не передаем */
                                  qFlashTimCallback); /* Указатель на функцию , которую вызывает таймер. */  
  
    /* Открытие таймера контроля ожидания ресурсов */
	qFlashxUpdateTimer = xTimerCreate( "QTmDly",		/* Текстовое имя, не используется в RTOS kernel. */
                                    10000,		/* Период таймера в тиках. */
                                    pdTRUE,		/* Время будет автоматически перезагружать себя, когда оно истечет. */
                                    NULL,		/* В функцию параметры не передаем */
                                    qFlashTimCallback); /* Указатель на функцию , которую вызывает таймер. */  

	/*  */
	xTimerStart(qFlashxSoftTimer, 0);   

  
	for (;;)
	{
		/* Обнуляем сообщение */
		qFlashNotifiedValue = 0;
		/*================================== Проверка наличия сообщений ========================================*/
		xTaskNotifyWait(0x00000000,			/* Не очищайте биты уведомлений при входе               */
		                0xFFFFFFFF,			/* Сбросить значение уведомления до 0 при выходе        */
		                &(qFlashNotifiedValue),			/* Значение уведомленное передается в  NotifiedValue    */
		                portMAX_DELAY); /* Блокировка задачи до появления уведомления           */
    
    
        /* Получено уведомление. Проверяем, какие биты были установлены. */
		/*=========== Уведомление от таймер периодического контроля  ==============*/
		if (((qFlashNotifiedValue) & QFLASH_NOTE_SOFT_TIM) != 0)
		{  
			qFlashCntrl();
		}    
		/*======== Уведомление от таймера контроля ожидания ресурсов  =============*/
		if (((qFlashNotifiedValue) & QFLASH_NOTE_DELAY_TIM) != 0)
		{  

		}      
	} 
}

/**
  * @brief  инициализация QSPI FLASH
  * @param  None
  * @retval None
  */
void qFlashInit(void)
{

	uint8_t id_code[3];

	printf("\n Start Init W25Q64JVS QSPI Drive... \n\n");	
	
	/*Initialize the QSPI in memory mapped mode*/
	BSP_QSPI_Init();
	BSP_QSPI_ConfigFlash(W25Q64JVS_QPI_MODE);
	BSP_QSPI_ReadID(id_code);
	
	if ((id_code[0] == 0xEF) && (id_code[1] == 0x40) && (id_code[2] == 0x17))
	{
		printf(" Init Drive Complite. \n\n");
	}
	else
	{
		printf(" Error Init Drive. \n\n");
	}	
	
	/* Инициализация задачи */ 
	xTaskCreate(qFlash_Task, (const char*)"QS_FLSH_TSK", configMINIMAL_STACK_SIZE * 12, NULL, TreadPrioBelowNormal, &qFlashHandleTask);	
}



