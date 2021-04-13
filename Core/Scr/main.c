/**
  ******************************************************************************
  * @file    main.c 
  * @author  Trembach D.N.
  * @version V1.0.0
  * @date    23-08-2020
  * @brief   
  *
  *
  ******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; COPYRIGHT 2020 OneTiOne </center></h2>
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Kernel includes. */
#include "printf_dbg.h"
#include "pin_dbg.h"
#include "cmd_process.h"
#include "flash_interface.h"

volatile const char __version__[] = "H750VB";    
volatile const char __date__[] = __DATE__;
volatile const char __time__[] = __TIME__;



/**
 * @brief  Start Thread 
 * @param  None
 * @retval None
 */
void system_thread(void *arg)
{ 
    #define CODE_STORE_VAR  (5589)
	uint32_t temp_var = 0;
	
	/*Инициализация аппаратной части отладки */
    hal_debug_uart_init();
	hal_debug_pin_init();
	
	// Информационная шапка программы
	printf("______________________________________________\r\n");
	printf("\r\n");
	printf("   %s \r\n", __version__);
	printf("   DATA: %s \r\n", __date__);
	printf("   TIME: %s \r\n", __time__);
	printf("   CPU FREQ = %.9lu Hz \r\n", SystemCoreClock);  
	printf("______________________________________________\r\n"); 
	
	/*Инициализация Emul EEPROM */
	Init_Emul_EEPROM();
	/*Чтение даннных */
	temp_var = StartLoadEmuleEEPROM(CODE_STORE_VAR, 193);	
	
	//Инициализация задачи диагностического терминала 
	xTaskCreate(terminal_task, (const char*)"CmdTrmnl", configMINIMAL_STACK_SIZE * 5, NULL, TreadPrioNormal, NULL);
	
	for (;;) {
		vTaskDelay(1000);
	}
}

/**
* @brief  CPU L1-Cache enable.
* @param  None
* @retval None
*/
static void CPU_CACHE_Enable(void)
{
	/* Enable I-Cache */
	SCB_EnableICache();

	/* Enable D-Cache */
	SCB_EnableDCache();
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	/* MCU Configuration--------------------------------------------------------*/
    /* Enable the CPU Cache */
	CPU_CACHE_Enable();	
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();	
	/* Init thread */
	xTaskCreate(system_thread, (const char*)"SysTask", configMINIMAL_STACK_SIZE * 2, NULL, TreadPrioNormal, NULL);	
	/* Start scheduler */
	vTaskStartScheduler();  
	/* We should never get here as control is now taken by the scheduler */
	NVIC_SystemReset();
	return 0;
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
	/* User can add his own implementation to report the HAL error return state */
	while (1) ;
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
	/* User can add his own implementation to report the file name and line number, tex:  */
	printf("Wrong parameters value: file %s on line %lu\r\n", file, line);
	while (1) ;
}
#endif /* USE_FULL_ASSERT */
/************************ COPYRIGHT 2020 OneTiOne *****END OF FILE****/
