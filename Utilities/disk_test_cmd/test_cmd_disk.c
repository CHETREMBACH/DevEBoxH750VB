/**
  ******************************************************************************
  * File Name          : test_cmd_disk.c
  * Description        : This file provides test code for the disk driver.
  * 
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "test_cmd_disk.h"
#include "pin_dbg.h"
#include "printf_dbg.h"
#include <stdlib.h>


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/**
  * @brief  Вывод строки дампа памяти на экран
  * @param  uint8_t* buff_damp - указатель на буфер дампа 
  * @param  uint16_t index_damp - индекс старта выдачи данных  
  * @param  uint32_t addr_mem - непосредственый адрес памяти данных   
  * @retval uint16_t - число обработанных данных
  */
uint16_t data_damp_print(uint8_t* buff_damp, uint32_t index_damp, uint32_t addr_mem)
{
	printf("0x%.8lX: ", addr_mem);
	for (uint32_t contic = 0; contic < 32; contic++)
	{
		/* отступ между банками */
		if (contic == 16) printf("   ");
		/* Формирование строки HEX */
		printf(" %.2X", buff_damp[index_damp + contic]);
	}
	printf("\n");
    return 32;  
}


/**
  * @brief  Функция визуализации содержимого блока памяти размером 256 байт
  * @param  uint8_t* buff_damp - указатель на буфер дампа 
  * @param  uint32_t damp_size - размер буфера  
  * @param  uint32_t addr_mem - непосредственый адрес памяти данных 
  * @retval void
  */
void bloc_damp_print(uint8_t* buff_damp, uint32_t damp_size, uint32_t addr_mem)
{  	printf("----------   00          04          08          0C             10          14          18          1C         \n");	
	for (uint32_t damp_cnt = 0; damp_cnt < (damp_size>>5); damp_cnt++)
	{
		data_damp_print(buff_damp, damp_cnt << 5, addr_mem + (damp_cnt << 5));
		if ((damp_cnt&0x00000003) == 0)
		{
			vTaskDelay(3);
		}
	}
}


/**
  * @brief  Функция визуализации содержимого блока памяти размером 256 байт
  * @param  uint8_t* pdampa - указатель на буфер дампа 
  * @param  uint8_t* pdampb - указатель на буфер дампа   
  * @param  uint32_t damp_size - размер буфера  
  * @retval bool - true полное совпадение
  *               false нет совпадения 
  */
bool compare_mem_damp(uint8_t* pdampa, uint8_t* pdampb, uint32_t  size_damp)
{
	for (uint32_t damp_cnt = 0; damp_cnt < size_damp; damp_cnt++)	
	{
		if (pdampa[damp_cnt] != pdampb[damp_cnt])
		{
			return false;
		}
	}
	return true;
}

/**
  * @brief  Функция заполнения блока памяти размером до 0xFFFF байт
  * @param  uint8_t* pdamp - указатель на буфер дампа 
  * @param  id_mode_qflash_e mode_fill - режим заполнения
  * @param  uint16_t offset_damp - смещение заполнения
  * @param  uint16_t size_damp - размер буфера  
  * @param  uint8_t code_fill - код заполнения 
  * @retval none
  */
void fill_damp(uint8_t* pdamp, id_mode_disk_e mode_fill, uint16_t offset_damp, uint16_t size_damp, uint8_t code_fill)
{
	uint8_t temp_fill = code_fill;
	switch (mode_fill)
	{
	case    MODE_RND:
		srand(temp_fill);
		for (uint16_t damp_cnt = offset_damp; damp_cnt < size_damp + offset_damp; damp_cnt++)	pdamp[damp_cnt] = (uint8_t)rand();	
		printf("\n The buffer is filled from 0x%.4X to 0x%.4X with the value RANDOM  \n", offset_damp, size_damp + offset_damp);
		break;		
	case    MODE_INC: 
		for (uint16_t damp_cnt = offset_damp; damp_cnt < size_damp; damp_cnt++)	pdamp[damp_cnt] = temp_fill--;	
		printf("\n The buffer is filled from 0x%.4X to 0x%.4X with the value INC  \n", offset_damp, size_damp + offset_damp);
		break;		
	case    MODE_DEC:
		for (uint16_t damp_cnt = offset_damp; damp_cnt < size_damp; damp_cnt++)	pdamp[damp_cnt] = temp_fill--;
		printf("\n The buffer is filled from 0x%.4X to 0x%.4X with the value DEC  \n", offset_damp, size_damp + offset_damp);		
		break;		
	case   MODE_FILL:
	default:
		for (uint16_t damp_cnt = offset_damp; damp_cnt < size_damp; damp_cnt++)	pdamp[damp_cnt] = temp_fill;
		printf("\n The buffer is filled from 0x%.4X to 0x%.4X with the value 0x%.2X  \n", offset_damp, size_damp + offset_damp, temp_fill);
		break;
	}
}
