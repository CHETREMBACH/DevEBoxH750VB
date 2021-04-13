/**
  ******************************************************************************
  * File Name          : test_qspi_flash.c.c
  * Description        : This file provides test code for the driver QUADSPI Flash.
  * 
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "task_qspi_flash.h"
#include "test_qspi_flash.h"
#include "drv_qspi_w25q64.h"
#include "test_fatfs.h"
#include "printf_dbg.h"
#include <stdlib.h>
#include <stdbool.h>
#include "pin_dbg.h"

/* Буфер рпазмером 1 сектор для тестирования памяти  */
uint8_t dampb[W25Q64JVS_SECTOR_SIZE];
uint8_t dampa[W25Q64JVS_SECTOR_SIZE];


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
uint16_t DataDampPrint(uint8_t* buff_damp, uint32_t index_damp, uint32_t addr_mem)
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
void BlocDampPrint(uint8_t* buff_damp, uint32_t damp_size, uint32_t addr_mem)
{  	printf("----------   00          04          08          0C             10          14          18          1C         \n");	
	for (uint32_t damp_cnt = 0; damp_cnt < (damp_size>>5); damp_cnt++)
	{
		DataDampPrint(buff_damp, damp_cnt << 5, addr_mem + (damp_cnt << 5));
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
void fill_damp(uint8_t* pdamp, id_mode_qflash_e mode_fill, uint16_t offset_damp, uint16_t size_damp, uint8_t code_fill)
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

/**
  * @brief  Функция отработки команды полное стирание памяти
  * @param  None
  * @retval None
  */
void cmd_chip_erase(void)
{
	printf("\n Start erases memory... \n");	
	/* Erases the entire QSPI memory.*/
	if (BSP_QSPI_EraseChip() == BSP_ERROR_NONE)
	{
		printf("Erases memory completed.\n");
	}
	else
	{
		printf("Error erases memory.\n");			
	}
}

/**
  * @brief  Функция отработки команды поиска непустых секторов
  * @param  None
  * @retval None
  */
void cmd_chip_status(void)
{
	uint16_t cnt_empty_sector = 0;
	
	printf("\n Start check memory... \n");	
	printf(" Address mem: 0x000000000 - 0x%.8X\n", W25Q64JVS_FLASH_SIZE - 1);
	printf(" Sector number: 0 - %.4d", W25Q64JVS_FLASH_SIZE / W25Q64JVS_SECTOR_SIZE - 1);	
	/* Подготовка буфера для сравнения */
	fill_damp(dampa, MODE_FILL, 0, W25Q64JVS_SECTOR_SIZE, 0xFF);
	printf("\n Sec               00       08       16       24       32       40       48       54\n");
	printf("        Addr mem   0x00000  0x08000  0x10000  0x18000  0x20000  0x28000  0x30000  0x38000");		
	/* Цикл по всем секторам */
	for (uint32_t cntic = 0; cntic < (W25Q64JVS_FLASH_SIZE / W25Q64JVS_SECTOR_SIZE); cntic++)
	{   
		/*  */
		if ((cntic & 0x003F) == 0) 
		{
			printf("\n%.4lu | 0x%.8lX: ", cntic, cntic*W25Q64JVS_SECTOR_SIZE);
		}
		else
		{
			if ((cntic & 0x0007) == 0) 	printf(" ");			
		}
		/* Чтение сегмента */	
		BSP_QSPI_Read(dampb, cntic*W25Q64JVS_SECTOR_SIZE, W25Q64JVS_SECTOR_SIZE);
		/* Анализ сегмента */		
		if (compare_mem_damp(dampa, dampb, W25Q64JVS_SECTOR_SIZE))
		{
			cnt_empty_sector++;			
			printf("*");
		}
		else
		{
			printf("-");			
		}
	}
	/* вывод статуса */
	printf("\nFill sector %.4d  ", (W25Q64JVS_FLASH_SIZE / W25Q64JVS_SECTOR_SIZE) - cnt_empty_sector);
	printf("Empty sector %.4d  ", cnt_empty_sector);	
	printf("Empty memory %2.2f %%\n", (((double)cnt_empty_sector) * 100) / (double)(W25Q64JVS_FLASH_SIZE / W25Q64JVS_SECTOR_SIZE));	
}

/**
  * @brief  Функция отработки команды вывода информации о чипе
  * @param  None
  * @retval None
  */
void cmd_chip_info(void)
{
	printf("\n W25Q64JVS 128Mbit   2,7-3,6V supply \n");
	printf("         1 chip   (size: 16 777 216(0x0100 0000) byte | \n");	
	printf("     65536 page   (size:        256(0x0000 0100) byte |           |)\n");
	printf("      4096 sector (size:       4096(0x0000 1000) byte | 16 page   |)\n");
	printf("       256 block  (size:      65536(0x0001 0000) byte | 16 sector | 256 page )\n");	
	printf(" SPI FLASH memory with dual/quad SPI & QPI \n");
	printf("  100 000 erase/program cycles \n");
	printf("  20-year data repention \n");	
}	

/**
  * @brief  Функция отработки команд управления для chip
  * @param  cmd_chip_qflash_t* cmd_param - указатель на параметры команды
  * @retval None
  */
void cmd_chip_hndlr(cmd_chip_qflash_t* cmd_param)
{
	switch (cmd_param->id_mode)
	{
	case   MODE_ERASE: cmd_chip_erase(); break;
	case  MODE_STATUS: cmd_chip_status(); break;
	case    MODE_INFO: cmd_chip_info(); break;		
	default: 	break;		
	}	
}

/**
  * @brief  Функция отработки команды стирания сектора
  * @param  cmd_sector_qflash_t* cmd_param - указатель на параметры команды
  * @retval None
  */
void cmd_erase_sector(cmd_sector_qflash_t* cmd_param)
{
	printf("\n Start erase sector %ld ( blok %ld )\n", cmd_param->number, (cmd_param->number) >> 4);
	printf("Address mem: 0x%.8lX - 0x%.8lX\n", (cmd_param->number)*W25Q64JVS_SECTOR_SIZE, (cmd_param->number + 1)*W25Q64JVS_SECTOR_SIZE - 1);
	BSP_QSPI_EraseBlock((cmd_param->number)*W25Q64JVS_SECTOR_SIZE, W25Q64JVS_ERASE_4K);
	printf("\n Erase sector completed.\n");		
}

/**
  * @brief  Функция отработки команды записи сектора
  * @param  cmd_sector_qflash_t* cmd_param - указатель на параметры команды
  * @retval None
  */
void cmd_write_sector(cmd_sector_qflash_t* cmd_param)
{
	printf("\n Start write sector %ld ( blok %ld )\n", cmd_param->number, (cmd_param->number) >> 4);
	printf("Address mem: 0x%.8lX - 0x%.8lX\n", (cmd_param->number)*W25Q64JVS_SECTOR_SIZE, (cmd_param->number + 1)*W25Q64JVS_SECTOR_SIZE - 1);
	BSP_QSPI_Write(dampa, (cmd_param->number)*W25Q64JVS_SECTOR_SIZE, W25Q64JVS_SECTOR_SIZE);
	printf("\n Write sector completed.\n");	
}


/**
  * @brief  Функция отработки команды чтения сектора
  * @param  cmd_sector_qflash_t* cmd_param - указатель на параметры команды
  * @retval None
  */
void cmd_read_sector(cmd_sector_qflash_t* cmd_param)
{
	printf("\n Start read sector %ld ( blok %ld )\n", cmd_param->number, (cmd_param->number) >> 4);
	printf("Address mem: 0x%.8lX - 0x%.8lX\n", (cmd_param->number)*W25Q64JVS_SECTOR_SIZE, (cmd_param->number + 1)*W25Q64JVS_SECTOR_SIZE - 1);
	BSP_QSPI_Read(dampb, (cmd_param->number)*W25Q64JVS_SECTOR_SIZE, W25Q64JVS_SECTOR_SIZE);
	//BSP_QSPI_DMARead(dampb, cmd_param->address & 0xFFFFF000, W25Q64JVS_SECTOR_SIZE);
	printf("\n Read sector completed.\n");	
}


/**
  * @brief  Функция отработки команды сравнения сектора с буфером
  * @param  cmd_sector_qflash_t* cmd_param - указатель на параметры команды
  * @retval None
  */
void cmd_compare_sector(cmd_sector_qflash_t* cmd_param)
{
	printf("\n Start compare sector %ld ( blok %ld )\n", cmd_param->number, (cmd_param->number) >> 4);
	printf("Address mem: 0x%.8lX - 0x%.8lX\n", (cmd_param->number)*W25Q64JVS_SECTOR_SIZE, (cmd_param->number + 1)*W25Q64JVS_SECTOR_SIZE - 1);	
	BSP_QSPI_Read(dampb, (cmd_param->number)*W25Q64JVS_SECTOR_SIZE, W25Q64JVS_SECTOR_SIZE);
	if (compare_mem_damp(dampa, dampb, W25Q64JVS_SECTOR_SIZE))
	{
		printf("\n Compare sector completed - sector matches the dump.\n");				
	}
	else
	{
		printf("\n Compare sector completed - sector does not match the dump.\n");				
	}
}

/**
  * @brief  Функция отработки команды вывода содержимого сектора на терминал
  * @param  cmd_sector_qflash_t* cmd_param - указатель на параметры команды
  * @retval None
  */
void cmd_view_sector(cmd_sector_qflash_t* cmd_param)
{
	printf("\n View sector %ld ( blok %ld )\n", cmd_param->number, (cmd_param->number) >> 4);
	printf("Address mem: 0x%.8lX - 0x%.8lX\n", (cmd_param->number)*W25Q64JVS_SECTOR_SIZE, (cmd_param->number + 1)*W25Q64JVS_SECTOR_SIZE - 1);	
	BSP_QSPI_Read(dampb, cmd_param->number * W25Q64JVS_SECTOR_SIZE, W25Q64JVS_SECTOR_SIZE);
	BlocDampPrint(dampb, W25Q64JVS_SECTOR_SIZE, cmd_param->number * W25Q64JVS_SECTOR_SIZE);	
}

/**
  * @brief  Функция отработки команд управления сектором
  * @param  cmd_sector_qflash_t* cmd_param - указатель на параметры команды
  * @retval None
  */
void cmd_sector_hndlr(cmd_sector_qflash_t* cmd_param)
{
	switch (cmd_param->id_mode)
	{
	case     MODE_ERASE: cmd_erase_sector(cmd_param); break;
	case     MODE_WRITE: cmd_write_sector(cmd_param); break;
	case      MODE_READ: cmd_read_sector(cmd_param); break;		
	case   MODE_COMPARE: cmd_compare_sector(cmd_param); break;		
	case      MODE_VIEW: cmd_view_sector(cmd_param); break;		
		
	default:
		break;		
	}	
}

/**
  * @brief  Функция отработки команд управления дампом
  * @param  cmd_damp_qflash_t* cmd_param - указатель на параметры команды
  * @retval None
  */
void cmd_damp_hndlr(cmd_damp_qflash_t* cmd_param)
{
	switch (cmd_param->id_mode)
	{
	case  MODE_RND:	
	case  MODE_INC:	
	case  MODE_DEC:		
	case MODE_FILL:	
		fill_damp(dampa, cmd_param->id_mode, 0, W25Q64JVS_SECTOR_SIZE, 0xFF);
		break;		
	case MODE_VIEW: 
		printf("\n View damp  \n");	
		BlocDampPrint(dampa, W25Q64JVS_SECTOR_SIZE,0);			
		break;		
	default:
		break;		
	}	
}

/**
  * @brief  Функция парсинга и отработки команды тестирования spi_flash
  * @param  cmd_box_qflash_t* cmdbox - указатель на пакет команды
  * @retval None
  */
void parsing_cmd_box(cmd_box_qflash_t* cmdbox)
{
	switch( cmdbox->id_cmd_qflash )
	{
	case    CMD_CHIP:	          cmd_chip_hndlr(&(cmdbox->cmd_chip)); break;
	case  CMD_SECTOR:	          cmd_sector_hndlr(&(cmdbox->cmd_sector)); break;
	case    CMD_DAMP:	          cmd_damp_hndlr(&(cmdbox->cmd_damp)); break;		
		
	default:
		break;		
	}	
}
