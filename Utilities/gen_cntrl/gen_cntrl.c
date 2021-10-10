/**
  ******************************************************************************
  * @file    gen_cntrl.c
  * @author  Trembach D.N.
  * @version V1.0.0
  * @date    10-10-2021
  * @brief   Файл содержит функции контроля генератора сигнала на выводе MCO2 PC9 
  *
  *
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; COPYRIGHT 2021 OneTiOne < / center>< / h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gen_cntrl.h"
#include "printf_dbg.h"
#include "cmd_hlp.h"


#define  TIMER_NOTE           (1l<<0)  /* Периодическое уведомление по таймеру      */
#define  MODE_NOTE            (1l<<1)  /* Получение из пакета данных                */
#define  CONFIG_NOTE          (1l<<2)  /* Запрос на конфигурирование                */

/* Перечесление форматов выхода генератора */
typedef enum {
	LOW_MD_OUT,
	HIGH_MD_OUT,
	GEN_MD_OUT
} mode_out_type;

/* Перечесление состояний генератора */
typedef enum {
	GEN_DIS,
	GEN_ENA,	
	GEN_PWM,
	GEN_SLEEP
} fsm_gen_type;

TimerHandle_t SoftTimer; /*  Програмный таймер  */
TimerHandle_t ModeTimer; /*  Програмный таймер  */
TaskHandle_t  HandleTask; /*   */
static uint32_t NotifiedValue; /*                  */
        
uint32_t div_p = 0;          /* PLL2P : Division factor for system clock.*/
uint32_t div_mco = 0;        /* Division factor for GPIO MCO.  */ 
uint32_t factor_pll_n = 0;   /* Multiplication factor for PLL2 VCO output clock(4...512) */  
uint32_t factor_pll_f = 0;   /* Fractional Part Of The Multiplication Factor(0...8191) */
uint32_t time_gen_work = 0;  /*  */  
uint32_t time_gen_sleep = 0; /*  */
uint32_t freq = 0;           /*  */ 

fsm_gen_type fsm_gen = GEN_DIS;

/**
  * @brief  Функция инициализации вывода MCO2.
  * @ param None
  * @retval None
  */
void InitGPIOGen( void )
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };	

	/*Configure GPIO pin : PC9 */
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);		
}

/**
  * @brief  Функция инициализации вывода MCO2.
  * @ param None
  * @retval None
  */
void SetGPIOGen(mode_out_type gpio_mode)
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };	
	
	/*Configure GPIO pin : PC9 */	
	switch (gpio_mode)
	{
	case HIGH_MD_OUT:
		GPIO_InitStruct.Pin = GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);		
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
		break;
	case GEN_MD_OUT: break;
		GPIO_InitStruct.Pin = GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);				
	default:
	case 	LOW_MD_OUT: 
		GPIO_InitStruct.Pin = GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);		
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);				
		break;		
	}
}

/**
  * @brief  Функция расчета коэффициентов pll по заданной частоте.
  * @param  uint32_t freq 
  * @param  uint32_t* div_p - PLL2P: Division factor for system clock.
  * @param  uint32_t* div_mco - Division factor for GPIO MCO.  
  * @param  uint32_t* factor_pll_n - Multiplication factor for PLL2 VCO output clock (4...512)   
  * @param  uint32_t* factor_pll_f  - Fractional Part Of The Multiplication Factor (0...8191) 
  * @retval None
  */
void CalcFactorGen(uint32_t freq, uint32_t* div_p,uint32_t* div_mco, uint32_t* factor_pll_n, uint32_t* factor_pll_f)
{	
	uint32_t freq_pll = 0;	
	*div_p = 0;
	*div_mco = 0;	
	*factor_pll_n = 0;	
	*factor_pll_f = 0;	
	
	const uint32_t freq_size_n = 400000000/400;		
	const double freq_size_f = 400000000.0/400.0/8192.0;		
	
	/* Ограничение максимальная частота */
	if (freq > 200000000) freq = 200000000;
   
	/* Ограничение минимальная частота */
	if (freq < 250000) freq = 250000;	
	
	/* определение делителей частоты PLL */
	*div_p = 400000000 / freq;
	
	(*div_p)++;
	
	if (*div_p > 128) 
	{
		*div_mco = 15;
		*div_p = (400000000 / 15) / freq;
		(*div_p)++; 
	}
	else
	{
		*div_mco = 1;
	}

	/* расчетная частота PLL */
	freq_pll = freq * (*div_p) * (*div_mco);
	
	*factor_pll_n = freq_pll / freq_size_n;
	*factor_pll_f = (uint16_t)(((double)(freq_pll % freq_size_n)) / freq_size_f);
}

/**
  * @brief  Функция инициализации генератора с заданными парасетрами.
  * @param  uint32_t div_p - PLL2P: Division factor for system clock.
  * @param  uint32_t div_mco - Division factor for GPIO MCO.  
  * @param  uint32_t factor_pll_n - Multiplication factor for PLL2 VCO output clock (4...512)   
  * @param  uint32_t factor_pll_f  - Fractional Part Of The Multiplication Factor (0...8191) 
  * @retval None
  */
void InitGenPll(uint32_t div_p, uint32_t div_mco, uint32_t factor_pll_n, uint32_t factor_pll_f)
{
	RCC_PeriphCLKInitTypeDef PeriphclkInitStruct = { 0 };
	
	/** Enables PLL2P clock output
	*/
	__HAL_RCC_PLL2CLKOUT_ENABLE(RCC_PLL2_DIVP);
	/** Initializes the peripherals clock
	*/
	
	/* Специально для включения MCO */
	PeriphclkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
	PeriphclkInitStruct.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_PLL2;
	/*--------------*/
	PeriphclkInitStruct.PLL2.PLL2M = 25;
	PeriphclkInitStruct.PLL2.PLL2N = factor_pll_n;
	PeriphclkInitStruct.PLL2.PLL2P = div_p;
	PeriphclkInitStruct.PLL2.PLL2Q = 2;
	PeriphclkInitStruct.PLL2.PLL2R = 2;
	PeriphclkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_0;
	PeriphclkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
	PeriphclkInitStruct.PLL2.PLL2FRACN = factor_pll_f;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphclkInitStruct) != HAL_OK)
	{
		Error_Handler();
	}	
	
	if (div_mco == 1)
	{
	   HAL_RCC_MCOConfig(RCC_MCO2, RCC_MCO2SOURCE_PLL2PCLK, RCC_MCODIV_1);			
	}
	else
	{
	   HAL_RCC_MCOConfig(RCC_MCO2, RCC_MCO2SOURCE_PLL2PCLK, RCC_MCODIV_15);			
	}
}

/**
  * @brief  Функция отработки профиля генерации.
  * @ param  None
  * @retval None
  */
void CntrlModeGen(void)
{
	/* Функция конфигурирования */
	if (freq > 0)
	{
		if (time_gen_work > 0)
		{
			if (time_gen_sleep > 0)
			{
				if (fsm_gen == GEN_SLEEP)
				{
					fsm_gen = GEN_PWM;							
					SetGPIOGen(LOW_MD_OUT);		
					CalcFactorGen(freq, &div_p, &div_mco, &factor_pll_n, &factor_pll_f);
					InitGenPll(div_p, div_mco, factor_pll_n, factor_pll_f);
					SetGPIOGen(GEN_MD_OUT);						
					xTimerChangePeriod(ModeTimer, time_gen_work, 0);
					xTimerReset(ModeTimer, 0);						
				}
				else
				{
					fsm_gen = GEN_SLEEP;							
					SetGPIOGen(LOW_MD_OUT);							
					xTimerChangePeriod(ModeTimer, time_gen_sleep, 0);
					xTimerReset(ModeTimer, 0);						
				}
			}
			else
			{
				xTimerStop(ModeTimer, 0);	
				fsm_gen = GEN_ENA;						
				SetGPIOGen(LOW_MD_OUT);	
				CalcFactorGen(freq, &div_p, &div_mco, &factor_pll_n, &factor_pll_f);
				InitGenPll(div_p, div_mco, factor_pll_n, factor_pll_f);
				SetGPIOGen(GEN_MD_OUT);	
			}
		} 
		else
		{
			xTimerStop(ModeTimer, 0);	
			fsm_gen = GEN_DIS;
			SetGPIOGen(LOW_MD_OUT);	
		}
	}
	else
	{
		xTimerStop(ModeTimer, 0);	
		fsm_gen = GEN_DIS;
		SetGPIOGen(LOW_MD_OUT);	
	}
}

/**
  * @brief  Функция отработки програмного таймера 
  * @param  None
  * @retval None
  */
void SendConfigNote(void)
{
	if ((HandleTask) != NULL)
	{
		/* Таймер контроля обновления данных */
		xTaskNotify( HandleTask,
			/* Указатель на уведомлюемую задачу                         */
			CONFIG_NOTE,
			/* Значения уведомления                                     */
			eSetBits); /* Текущее уведомление добавляются к уже прописанному       */
	}  
}

/**
  * @brief  Задача основного автомата состояний.
  * @param  pvParameters not used
  * @retval None
  */
void CntrlGenTask(void * pvParameters)
{
	for(;  ;)
	{
		/* Обнуляем сообщение */
		NotifiedValue = 0;  
		xTaskNotifyWait(0x00000000, 	/* Не очищайте биты уведомлений при входе               */
			0xFFFFFFFF,     			/* Сбросить значение уведомления до 0 при выходе        */
			&(NotifiedValue),			/* Значение уведомленное передается в  NotifiedValue    */
			portMAX_DELAY);             /* Блокировка задачи до появления уведомления           */
    
        /*  Проверка наличия сообщений  */
        /* Получено уведомление. Проверяем, какие биты были установлены. */
		if (((NotifiedValue) & TIMER_NOTE) != 0)
		{  
			
		}    
		
		/* Получено уведомление. Проверяем, какие биты были установлены. */
		if (((NotifiedValue) & MODE_NOTE) != 0)
		{  
			/* Функция отработки профиля генерации. */
			CntrlModeGen();
		}   	
		
		/* Получено уведомление. Проверяем, какие биты были установлены. */
		if (((NotifiedValue) & CONFIG_NOTE) != 0)
		{  
			/* Функция конфигурирования */
			if (freq > 0)
			{
				if (time_gen_work > 0)
				{
					if (time_gen_sleep > 0)
					{
						fsm_gen = GEN_SLEEP;							
						SetGPIOGen(LOW_MD_OUT);							
						xTimerChangePeriod(ModeTimer, time_gen_sleep, 0);
						xTimerReset(ModeTimer, 0);						
					}
					else
					{
						xTimerStop(ModeTimer, 0);	
						fsm_gen = GEN_ENA;						
						SetGPIOGen(LOW_MD_OUT);	
						CalcFactorGen(freq, &div_p, &div_mco, &factor_pll_n, &factor_pll_f);
						InitGenPll(div_p, div_mco, factor_pll_n, factor_pll_f);
						SetGPIOGen(GEN_MD_OUT);	
					}
				} 
				else
				{
					xTimerStop(ModeTimer, 0);	
					fsm_gen = GEN_DIS;
					SetGPIOGen(LOW_MD_OUT);	
				}
			}
			else
			{
				xTimerStop(ModeTimer, 0);	
				fsm_gen = GEN_DIS;
				SetGPIOGen(LOW_MD_OUT);	
			}
		}   			
	} 	
}

/**
  * @brief  Функция отработки програмного таймера 
  * @param  TimerHandle_t pxTimer - указатель на таймер вызвавщий функцию
  * @retval None
  */
void SoftTimerCallback(TimerHandle_t pxTimer)
{
	/* Функция обработки програмного таймера.*/
	if ((HandleTask) != NULL)
	{
		if (SoftTimer == pxTimer)
		{
			/* Таймер контроля обновления данных */
			xTaskNotify( HandleTask,   /* Указатель на уведомлюемую задачу                         */
				TIMER_NOTE,            /* Значения уведомления                                     */
				eSetBits);             /* Текущее уведомление добавляются к уже прописанному       */
		}
		
		if (ModeTimer == pxTimer)
		{
			/* Таймер контроля режима работы индикации */
			xTaskNotify( HandleTask,   /* Указатель на уведомлюемую задачу                         */
				MODE_NOTE,             /* Значения уведомления                                     */
				eSetBits);             /* Текущее уведомление добавляются к уже прописанному       */
		}		
	}  
}

/**
  * @brief  Инициализация и запуск задачи контроля генератора.
  * @param  None 
  * @retval None
  */
void InitCntrlGenTask(void)
{
		
	/* Функция инициализации вывода MCO2. */
	InitGPIOGen();
	SendConfigNote();	
	
	/* Открытие таймер контроля  */
	SoftTimer = xTimerCreate( "TmSf",	/* Текстовое имя, не используется в RTOS kernel. */
		50,	      	                    /* Период таймера в тиках. */
		pdTRUE,		                    /* Время будет автоматически перезагружать себя, когда оно истечет. */
		NULL,		                    /* В функцию параметры не передаем */
		SoftTimerCallback);             /* Указатель на функцию , которую вызывает таймер. */  
	
	xTimerStart(SoftTimer, 0);   	
	
	
	/* Открытие таймер режима  */
	ModeTimer = xTimerCreate( "TmMD",	/* Текстовое имя, не используется в RTOS kernel. */
		1,   		                /* Период таймера в тиках. */
		pdTRUE,                  		/* Время будет автоматически перезагружать себя, когда оно истечет. */
		NULL,                   		/* В функцию параметры не передаем */
		SoftTimerCallback);             /* Указатель на функцию , которую вызывает таймер. */  
	
	xTimerStart(ModeTimer, 0);	
	
	/* Запуск задачи контроля платы */
	xTaskCreate(CntrlGenTask, "GEN", configMINIMAL_STACK_SIZE * 5, NULL, TreadPrioNormal, &HandleTask);	
}

/**
  * @brief  обработчик команды задания параметров генератора
  * @param  cmd_parametr_t *parametr - указатель на параметр команды
  * @retval uint16_t - возвращаемое значение
  */
uint16_t generator_cmd_handler(cmd_parametr_t *parametr)
{
	if (parametr != NULL)
	{
		if (strcmp(parametr->var_ch, "freq") == 0)
		{
			freq = parametr[1].var_u32;	
			SendConfigNote();
			return 0;	
		}
    
		if (strcmp(parametr->var_ch, "time_on") == 0)
		{
			time_gen_work = parametr[1].var_u32;
			SendConfigNote();	
			return 0;	
		}    
       
		if (strcmp(parametr->var_ch, "time_off") == 0)    
		{ 
			time_gen_sleep = parametr[1].var_u32;
			SendConfigNote();
			return 0;	
		}
	}    

	printf("Invalid parameter\r\n");    
	return 0;					
}

/*================= команда статуса freertos =============================*/
const cmd_t generator_cmd = 
{ 
	/* имя ( мнемомика ) команды        */   
	"gen",
	/* тип параметра команды            */
	VR_STRING,
	VR_UINT32,
	VR_NONE,
	VR_NONE,
	VR_NONE,
	/* краткое описание команды         */
	"Generator control command",
	/* расширенное описание команды     */ 
	"the command is for setting the generator mode.\r\n  command format: >gen <STRING> <UINT32>\r\n   freq - generator frequency Hz. \r\n   time_on - working time ms. \r\n   time_off - time off work ms.",
	/* указатель на обработчик команд   */    		
	generator_cmd_handler
};

/**
  * @brief  Инициализация указателя на команды управления генератором
  * @param  none
  * @retval none
  */
void generator_cmd_init()
{
	add_terminal_cmd(&generator_cmd);   
}
/************************ (C) COPYRIGHT ONE TO ONE *****END OF FILE****/