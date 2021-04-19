/**
  ******************************************************************************
  * File Name          : test_ram_disk.c.c
  * Description        : This file provides test code for the ram disk driver.
  * 
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ram_disk.h"
#include "test_ram_disk.h"
#include "pin_dbg.h"
#include "printf_dbg.h"
#include <stdlib.h>
#include <stdbool.h>

/* Буфер рпазмером 1 сектор для тестирования памяти  */
uint8_t ram_dmpb[RAM_SECTOR_SIZE];
uint8_t ram_dmpa[RAM_SECTOR_SIZE];

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/**
  * @brief  Функция отработки команды полное стирание памяти
  * @param  None
  * @retval None
  */
void cmd_ram_chip_erase(void)
{
	printf("\n Start erases memory... \n");	
	/* Erases the entire QSPI memory.*/
	if (ram_erease_chip() == BSP_ERROR_NONE)
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
void cmd_ram_chip_status(void)
{
	uint16_t cnt_empty_sector = 0;
	
	printf("\n Start check memory... \n");	
	printf(" Address mem: 0x000000000 - 0x%.8X\n", RAM_DISK_SIZE - 1);
	printf(" Sector number: 0 - %.4d", RAM_DISK_SIZE / RAM_SECTOR_SIZE - 1);	
	/* Подготовка буфера для сравнения */
	fill_damp(ram_dmpb, MODE_FILL, 0, RAM_SECTOR_SIZE, 0xFF);
	printf("\n Sec               00       08       16       24       32       40       48       54\n");
	printf("        Addr mem   0x00000  0x08000  0x10000  0x18000  0x20000  0x28000  0x30000  0x38000");		
	/* Цикл по всем секторам */
	for (uint32_t cntic = 0; cntic < (RAM_DISK_SIZE / RAM_SECTOR_SIZE); cntic++)
	{   
		/*  */
		if ((cntic & 0x003F) == 0) 
		{
			printf("\n%.4lu | 0x%.8lX: ", cntic, cntic*RAM_SECTOR_SIZE);
		}
		else
		{
			if ((cntic & 0x0007) == 0) 	printf(" ");			
		}
		/* Чтение сегмента */	
		ram_read(ram_dmpa, cntic*RAM_SECTOR_SIZE, RAM_SECTOR_SIZE);
		/* Анализ сегмента */		
		if (compare_mem_damp(ram_dmpa, ram_dmpb, RAM_SECTOR_SIZE))
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
	printf("\nFill sector %.4d  ", (RAM_DISK_SIZE / RAM_SECTOR_SIZE) - cnt_empty_sector);
	printf("Empty sector %.4d  ", cnt_empty_sector);	
	printf("Empty memory %2.2f %%\n", (((double)cnt_empty_sector) * 100) / (double)(RAM_DISK_SIZE / RAM_SECTOR_SIZE));	
}

/**
  * @brief  Функция отработки команды вывода информации о чипе
  * @param  None
  * @retval None
  */
void cmd_ram_chip_info(void)
{
	printf("\n RAM DISK 4Mbit  \n");
	printf("         1 chip   (size:    524 288(0x0008 0000) byte \n");	
	printf("      128 sector  (size:       4096(0x0000 1000) byte \n");
}	

/**
  * @brief  Функция отработки команд управления для chip
  * @param  cmd_chip_qflash_t* cmd_param - указатель на параметры команды
  * @retval None
  */
void cmd_ram_chip_hndlr(cmd_chip_disk_t* cmd_param)
{
	switch (cmd_param->id_mode)
	{
	case   MODE_ERASE: cmd_ram_chip_erase(); break;
	case  MODE_STATUS: cmd_ram_chip_status(); break;
	case    MODE_INFO: cmd_ram_chip_info(); break;		
	default: 	break;		
	}	
}

/**
  * @brief  Функция отработки команды стирания сектора
  * @param  cmd_sector_qflash_t* cmd_param - указатель на параметры команды
  * @retval None
  */
void cmd_ram_erase_sector(cmd_sector_disk_t* cmd_param)
{
	printf("\n Start erase sector %ld ( blok %ld )\n", cmd_param->number, (cmd_param->number) >> 4);
	printf("Address mem: 0x%.8lX - 0x%.8lX\n", (cmd_param->number)*RAM_SECTOR_SIZE, (cmd_param->number + 1)*RAM_SECTOR_SIZE - 1);
	ram_erase_block((cmd_param->number)*RAM_SECTOR_SIZE, RAM_SECTOR_SIZE);
	printf("\n Erase sector completed.\n");		
}

/**
  * @brief  Функция отработки команды записи сектора
  * @param  cmd_sector_qflash_t* cmd_param - указатель на параметры команды
  * @retval None
  */
void cmd_ram_write_sector(cmd_sector_disk_t* cmd_param)
{
	printf("\n Start write sector %ld ( blok %ld )\n", cmd_param->number, (cmd_param->number) >> 4);
	printf("Address mem: 0x%.8lX - 0x%.8lX\n", (cmd_param->number)*RAM_SECTOR_SIZE, (cmd_param->number + 1)*RAM_SECTOR_SIZE - 1);
	ram_write(ram_dmpa, (cmd_param->number)*RAM_SECTOR_SIZE, RAM_SECTOR_SIZE);
	printf("\n Write sector completed.\n");	
}


/**
  * @brief  Функция отработки команды чтения сектора
  * @param  cmd_sector_qflash_t* cmd_param - указатель на параметры команды
  * @retval None
  */
void cmd_ram_read_sector(cmd_sector_disk_t* cmd_param)
{
	printf("\n Start read sector %ld ( blok %ld )\n", cmd_param->number, (cmd_param->number) >> 4);
	printf("Address mem: 0x%.8lX - 0x%.8lX\n", (cmd_param->number)*RAM_SECTOR_SIZE, (cmd_param->number + 1)*RAM_SECTOR_SIZE - 1);
	ram_read(ram_dmpa, (cmd_param->number)*RAM_SECTOR_SIZE, RAM_SECTOR_SIZE);
	printf("\n Read sector completed.\n");	
}


/**
  * @brief  Функция отработки команды сравнения сектора с буфером
  * @param  cmd_sector_qflash_t* cmd_param - указатель на параметры команды
  * @retval None
  */
void cmd_ram_compare_sector(cmd_sector_disk_t* cmd_param)
{
	printf("\n Start compare sector %ld ( blok %ld )\n", cmd_param->number, (cmd_param->number) >> 4);
	printf("Address mem: 0x%.8lX - 0x%.8lX\n", (cmd_param->number)*RAM_SECTOR_SIZE, (cmd_param->number + 1)*RAM_SECTOR_SIZE - 1);	
	ram_read(ram_dmpa, (cmd_param->number)*RAM_SECTOR_SIZE, RAM_SECTOR_SIZE);
	if (compare_mem_damp(ram_dmpa, ram_dmpb, RAM_SECTOR_SIZE))
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
void cmd_ram_view_sector(cmd_sector_disk_t* cmd_param)
{
	printf("\n View sector %ld ( blok %ld )\n", cmd_param->number, (cmd_param->number) >> 4);
	printf("Address mem: 0x%.8lX - 0x%.8lX\n", (cmd_param->number)*RAM_SECTOR_SIZE, (cmd_param->number + 1)*RAM_SECTOR_SIZE - 1);	
	ram_read(ram_dmpb, cmd_param->number * RAM_SECTOR_SIZE, RAM_SECTOR_SIZE);
	bloc_damp_print(ram_dmpb, RAM_SECTOR_SIZE, cmd_param->number * RAM_SECTOR_SIZE);	
}

/**
  * @brief  Функция отработки команд управления сектором
  * @param  cmd_sector_qflash_t* cmd_param - указатель на параметры команды
  * @retval None
  */
void cmd_ram_sector_hndlr(cmd_sector_disk_t* cmd_param)
{
	switch (cmd_param->id_mode)
	{
	case     MODE_ERASE: cmd_ram_erase_sector(cmd_param); break;
	case     MODE_WRITE: cmd_ram_write_sector(cmd_param); break;
	case      MODE_READ: cmd_ram_read_sector(cmd_param); break;		
	case   MODE_COMPARE: cmd_ram_compare_sector(cmd_param); break;		
	case      MODE_VIEW: cmd_ram_view_sector(cmd_param); break;		
		
	default:
		break;		
	}	
}

/**
  * @brief  Функция отработки команд управления дампом
  * @param  cmd_damp_qflash_t* cmd_param - указатель на параметры команды
  * @retval None
  */
void cmd_ram_damp_hndlr(cmd_damp_disk_t* cmd_param)
{
	switch (cmd_param->id_mode)
	{
	case  MODE_RND:	
	case  MODE_INC:	
	case  MODE_DEC:		
	case  MODE_FILL:	
		fill_damp(ram_dmpa, cmd_param->id_mode, 0, RAM_SECTOR_SIZE, 0xFF);
		break;		
	case MODE_VIEW: 
		printf("\n View damp  \n");	
		bloc_damp_print(ram_dmpa, RAM_SECTOR_SIZE, 0);			
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
void parsing_cmd_ram_box(cmd_box_disk_t* cmdbox)
{
	switch( cmdbox->id_cmd_qflash )
	{
	case    CMD_CHIP:	          cmd_ram_chip_hndlr(&(cmdbox->cmd_chip)); break;
	case  CMD_SECTOR:	          cmd_ram_sector_hndlr(&(cmdbox->cmd_sector)); break;
	case    CMD_DAMP:	          cmd_ram_damp_hndlr(&(cmdbox->cmd_damp)); break;		
		
	default:
		break;		
	}	
}
