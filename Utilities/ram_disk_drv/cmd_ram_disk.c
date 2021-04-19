/**
 ******************************************************************************
 * @file    cmd_ram_disk.c
 * @author  Trembach D.N.
 * @version V1.0.0
 * @date    19-04-2021
 * @brief   файл функций обработки команд управления и тестирования ram disk  
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2021 OneTiOne </center></h2>
 ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "ram_disk.h"
#include "cmd_ram_disk.h"
#include "test_ram_disk.h"

cmd_box_disk_t temp_box_disk;

/**
 * @brief  обработчик команды операции с сектором
 * @param  cmd_parametr_t *parametr - указатель на параметр команды
 * @retval uint16_t - возвращаемое значение
 */
uint16_t sector_ram_cmd_handler(cmd_parametr_t *parametr)
{
	if (parametr != NULL)
	{
		temp_box_disk.id_cmd_qflash = CMD_SECTOR;
		temp_box_disk.cmd_sector.number = parametr[1].var_u32;
		
		if (strcmp(parametr->var_ch, "erase") == 0)
		{
			temp_box_disk.cmd_sector.id_mode = MODE_ERASE;
			parsing_cmd_ram_box(&temp_box_disk);	
			return 0;
		}
    
		if (strcmp(parametr->var_ch, "wr") == 0)    
		{  
			temp_box_disk.cmd_sector.id_mode = MODE_WRITE;
			parsing_cmd_ram_box(&temp_box_disk);	
			return 0;
		}
    
		if (strcmp(parametr->var_ch, "rd") == 0)    
		{  
			temp_box_disk.cmd_sector.id_mode = MODE_READ;
			parsing_cmd_ram_box(&temp_box_disk);	
			return 0;			
		}	
		
		if (strcmp(parametr->var_ch, "cmp") == 0)    
		{  
			temp_box_disk.cmd_sector.id_mode = MODE_COMPARE;
			parsing_cmd_ram_box(&temp_box_disk);	
			return 0;		
		}	
		
		if (strcmp(parametr->var_ch, "view") == 0)    
		{  
			temp_box_disk.cmd_sector.id_mode = MODE_VIEW;
			parsing_cmd_ram_box(&temp_box_disk);		
			return 0;	
		}		
	}    
  
	printf("Invalid parameter\r\n");    
	return 0;					
}

/*================= команда помощи =============================*/
const cmd_t sector_ram_cmd = 
{ 
	/* имя ( мнемомика ) команды        */   
	"sector_ram",
	/* тип параметра команды            */
	VR_STRING,
	VR_UINT32,
	VR_NONE,
	VR_NONE,
	VR_NONE,
	/* краткое описание команды         */
	"Sector control command",
	/* расширенное описание команды     */ 
	"the command to start the operation with a sector.\r\n  command format: >sector_ram <STRING> <U32>\r\n   erase - erase sector, \r\n   wr - write sector, \r\n   rd - read sector, \r\n   cmp - compare sector, \r\n   view - viev sector, \r\n  <U32> - number 0 - 4095.\r\n",
	/* указатель на обработчик команд   */    		
	sector_ram_cmd_handler
};

/**
 * @brief  обработчик команды операции с данными ( буфером чтения/записи )
 * @param  cmd_parametr_t *parametr - указатель на параметр команды
 * @retval uint16_t - возвращаемое значение
 */
uint16_t damp_ram_handler(cmd_parametr_t *parametr)
{
	if (parametr != NULL)
	{
		temp_box_disk.id_cmd_qflash = CMD_DAMP;
		temp_box_disk.cmd_damp.code_fill = parametr[1].var_u08;
		
		if (strcmp(parametr->var_ch, "fill") == 0)
		{
			temp_box_disk.cmd_damp.id_mode = MODE_FILL;
			parsing_cmd_ram_box(&temp_box_disk);	
			return 0;
		}
    
		if (strcmp(parametr->var_ch, "rnd") == 0)    
		{  
			temp_box_disk.cmd_damp.id_mode = MODE_RND;
			parsing_cmd_ram_box(&temp_box_disk);	
			return 0;
		}
    
		if (strcmp(parametr->var_ch, "inc") == 0)    
		{  
			temp_box_disk.cmd_damp.id_mode = MODE_INC;
			parsing_cmd_ram_box(&temp_box_disk);	
			return 0;			
		}	
		
		if (strcmp(parametr->var_ch, "dec") == 0)    
		{  
			temp_box_disk.cmd_damp.id_mode = MODE_DEC;
			parsing_cmd_ram_box(&temp_box_disk);		
			return 0;	
		}	
		
		if (strcmp(parametr->var_ch, "view") == 0)    
		{  
			temp_box_disk.cmd_damp.id_mode = MODE_VIEW;
			parsing_cmd_ram_box(&temp_box_disk);	
			return 0;		
		}		
	}    
  
	printf("Invalid parameter\r\n");    
	return 0;					
}

/*================= команда помощи =============================*/
const cmd_t damp_ram_cmd = 
{ 
	/* имя ( мнемомика ) команды        */   
	"damp_ram",
	/* тип параметра команды            */
	VR_STRING,
	VR_UINT16,
	VR_UINT16,
	VR_NONE,
	VR_NONE,
	/* краткое описание команды         */
	"Damp buffer control command",
	/* расширенное описание команды     */ 
	"the command is intended to set the buffer damp operating mode.\r\n  command format: >damp_ram <STRING> <U16> <U16>\r\n   fill - filling with code, \r\n   rnd - filling with random, \r\n   inc - incremental padding, \r\n   dec - decremental padding, \r\n   view - viev damp, \r\n  <U16> - start address 0 - 4095, \r\n  <U16> - start address 0 - 4095.\r\n",
	/* указатель на обработчик команд   */    		
	damp_ram_handler
};

/**
 * @brief  обработчик команды операции с чипом
 * @param  cmd_parametr_t *parametr - указатель на параметр команды
 * @retval uint16_t - возвращаемое значение
 */
uint16_t ram_cmd_handler(cmd_parametr_t *parametr)
{
	if (parametr != NULL)
	{
		temp_box_disk.id_cmd_qflash = CMD_CHIP;
		
		if (strcmp(parametr->var_ch, "erase") == 0)
		{
			temp_box_disk.cmd_chip.id_mode = MODE_ERASE;
			parsing_cmd_ram_box(&temp_box_disk);	
			return 0;		
		}
    
		if (strcmp(parametr->var_ch, "status") == 0)    
		{  
			temp_box_disk.cmd_chip.id_mode = MODE_STATUS;
			parsing_cmd_ram_box(&temp_box_disk);	
			return 0;		
		}
		
		if (strcmp(parametr->var_ch, "info") == 0)    
		{  
			temp_box_disk.cmd_chip.id_mode = MODE_INFO;
			parsing_cmd_ram_box(&temp_box_disk);	
			return 0;		
		}		
	}    
  
	printf("Invalid parameter\r\n");    
	return 0;					
}

/*================= команда помощи =============================*/
const cmd_t ram_cmd = 
{ 
	/* имя ( мнемомика ) команды        */   
	"ram",
	/* тип параметра команды            */
	VR_STRING,
	VR_NONE,
	VR_NONE,
	VR_NONE,
	VR_NONE,
	/* краткое описание команды         */
	"Chip flash control command",
	/* расширенное описание команды     */ 
	"the command is intended to set the chip flash operating mode.\r\n  command format: >ram <STRING>\r\n   erase - erase chip, \r\n   status - status chip flash, \r\n   info - info chip flash.\r\n",
	/* указатель на обработчик команд   */    		
	ram_cmd_handler
};

/**
  * @brief  Инициализация указателя на команды управления и тестирования ram  
  * @param  none
  * @retval none
  */
void ram_disk_cmd_init()
{
	/* Инициализация диска в RAM.*/
	ram_disk_init();
	
	add_terminal_cmd(&ram_cmd);
	add_terminal_cmd(&damp_ram_cmd);	
	add_terminal_cmd(&sector_ram_cmd);	
}

/******************* (C) COPYRIGHT 2020 OneTiOne  *****END OF FILE****/