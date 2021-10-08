/**
  ******************************************************************************
  * @file       stm32h7xx_hal_msp.c
  * @author     Trembach D.N.
  * @version    V1.5.0
  * @date       25-02-2020
  * @brief      Файл инициализации HAL.
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

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();	
	
	/** Supply configuration update enable
	*/
	HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
	/** Configure the main internal regulator output voltage
	*/
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
	/** Macro to configure the PLL clock source
	*/
	__HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);
	/** Initializes the RCC Oscillators according to the specified parameters
	* in the RCC_OscInitTypeDef structure.
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 5;
	RCC_OscInitStruct.PLL.PLLN = 160;
	RCC_OscInitStruct.PLL.PLLP = 2;
	RCC_OscInitStruct.PLL.PLLQ = 16;
	RCC_OscInitStruct.PLL.PLLR = 2;
	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
	RCC_OscInitStruct.PLL.PLLFRACN = 0;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
	                            | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2
	                            | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
	RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		Error_Handler();
	}
	
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3 | RCC_PERIPHCLK_USB;
	PeriphClkInitStruct.PLL3.PLL3M = 1;
	PeriphClkInitStruct.PLL3.PLL3N = 48;
	PeriphClkInitStruct.PLL3.PLL3P = 2;
	PeriphClkInitStruct.PLL3.PLL3Q = 8;
	PeriphClkInitStruct.PLL3.PLL3R = 2;
	PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_3;
	PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
	PeriphClkInitStruct.PLL3.PLL3FRACN = 0;

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3;
	PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
	
	RCC_PeriphCLKInitTypeDef Periph2clkInitStruct = { 0 };
	
	/** Enables PLL2P clock output
	*/
	__HAL_RCC_PLL2CLKOUT_ENABLE(RCC_PLL2_DIVP);
	/** Initializes the peripherals clock
	*/
	Periph2clkInitStruct.PLL2.PLL2M = 25;
	Periph2clkInitStruct.PLL2.PLL2N = 480;
	Periph2clkInitStruct.PLL2.PLL2P = 2;
	Periph2clkInitStruct.PLL2.PLL2Q = 2;
	Periph2clkInitStruct.PLL2.PLL2R = 2;
	Periph2clkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_0;
	Periph2clkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
	Periph2clkInitStruct.PLL2.PLL2FRACN = 4096;
	if (HAL_RCCEx_PeriphCLKConfig(&Periph2clkInitStruct) != HAL_OK)
	{
		Error_Handler();
	}	
	
	HAL_RCC_MCOConfig(RCC_MCO2, RCC_MCO2SOURCE_PLL2PCLK, RCC_MCODIV_4);	
	
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };	
	

	/*Configure GPIO pin : PC9 */
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);	
	
}

/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
	__HAL_RCC_SYSCFG_CLK_ENABLE();

	/* System interrupt init*/
	/* PendSV_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);

	/* Configure the system clock */
	SystemClock_Config();
	SystemCoreClockUpdate();
}
/************************ COPYRIGHT 2020 OneTiOne *****END OF FILE****/
