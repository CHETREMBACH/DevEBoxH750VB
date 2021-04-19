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
#include "pin_dbg.h"

/* Буфер рпазмером 1 сектор для тестирования памяти  */
uint8_t dampb[W25Q64JVS_SECTOR_SIZE];
uint8_t dampa[W25Q64JVS_SECTOR_SIZE];

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/**
  * @brief  Функция отработки команды полное стирание памяти
  * @param  None
  * @retval None
  */
void cmd_flash_chip_erase(void)
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
void cmd_flash_chip_status(void)
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
void cmd_flash_chip_info(void)
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
void cmd_flash_chip_hndlr(cmd_chip_disk_t* cmd_param)
{
	switch (cmd_param->id_mode)
	{
	case   MODE_ERASE: cmd_flash_chip_erase(); break;
	case  MODE_STATUS: cmd_flash_chip_status(); break;
	case    MODE_INFO: cmd_flash_chip_info(); break;		
	default: 	break;		
	}	
}

/**
  * @brief  Функция отработки команды стирания сектора
  * @param  cmd_sector_qflash_t* cmd_param - указатель на параметры команды
  * @retval None
  */
void cmd_flash_erase_sector(cmd_sector_disk_t* cmd_param)
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
void cmd_flash_write_sector(cmd_sector_disk_t* cmd_param)
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
void cmd_flash_read_sector(cmd_sector_disk_t* cmd_param)
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
void cmd_flash_compare_sector(cmd_sector_disk_t* cmd_param)
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
void cmd_flash_view_sector(cmd_sector_disk_t* cmd_param)
{
	printf("\n View sector %ld ( blok %ld )\n", cmd_param->number, (cmd_param->number) >> 4);
	printf("Address mem: 0x%.8lX - 0x%.8lX\n", (cmd_param->number)*W25Q64JVS_SECTOR_SIZE, (cmd_param->number + 1)*W25Q64JVS_SECTOR_SIZE - 1);	
	BSP_QSPI_Read(dampb, cmd_param->number * W25Q64JVS_SECTOR_SIZE, W25Q64JVS_SECTOR_SIZE);
	bloc_damp_print(dampb, W25Q64JVS_SECTOR_SIZE, cmd_param->number * W25Q64JVS_SECTOR_SIZE);	
}

/**
  * @brief  Функция отработки команд управления сектором
  * @param  cmd_sector_qflash_t* cmd_param - указатель на параметры команды
  * @retval None
  */
void cmd_flash_sector_hndlr(cmd_sector_disk_t* cmd_param)
{
	switch (cmd_param->id_mode)
	{
	case     MODE_ERASE: cmd_flash_erase_sector(cmd_param); break;
	case     MODE_WRITE: cmd_flash_write_sector(cmd_param); break;
	case      MODE_READ: cmd_flash_read_sector(cmd_param); break;		
	case   MODE_COMPARE: cmd_flash_compare_sector(cmd_param); break;		
	case      MODE_VIEW: cmd_flash_view_sector(cmd_param); break;		
		
	default:
		break;		
	}	
}

/**
  * @brief  Функция отработки команд управления дампом
  * @param  cmd_damp_qflash_t* cmd_param - указатель на параметры команды
  * @retval None
  */
void cmd_flash_damp_hndlr(cmd_damp_disk_t* cmd_param)
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
		bloc_damp_print(dampa, W25Q64JVS_SECTOR_SIZE,0);			
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
void parsing_cmd_flash_box(cmd_box_disk_t* cmdbox)
{
	switch( cmdbox->id_cmd_qflash )
	{
	case    CMD_CHIP:	          cmd_flash_chip_hndlr(&(cmdbox->cmd_chip)); break;
	case  CMD_SECTOR:	          cmd_flash_sector_hndlr(&(cmdbox->cmd_sector)); break;
	case    CMD_DAMP:	          cmd_flash_damp_hndlr(&(cmdbox->cmd_damp)); break;		
		
	default:
		break;		
	}	
}
