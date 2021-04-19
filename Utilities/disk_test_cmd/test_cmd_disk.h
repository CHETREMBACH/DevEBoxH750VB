/**
  ******************************************************************************
  * File Name          : test_cmd_disk.h
  * Description        : This file provides test code for the disk driver.
  * 
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TEST_CMD_DISK_H
#define __TEST_CMD_DISK_H

#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>	

/* перечисление для типа параметра команды */
typedef enum
{ 
	 CMD_CHIP   = 0,
	 CMD_SECTOR,
	 CMD_FATFS,
	 CMD_DAMP
}id_cmd_disk_e;

/* перечисление для типа режима команды */
typedef enum
{ 
	 MODE_STATUS = 0,
	 MODE_PARAM,
	 MODE_FORMAT,
	 MODE_MOUNT,
	 MODE_WRITE,
	 MODE_READ,
	 MODE_VIEW,
	 MODE_COMPARE,
	 MODE_FILL,
	 MODE_RND,
	 MODE_INC,
	 MODE_DEC,
	 MODE_ERASE,
	 MODE_INFO
}id_mode_disk_e;	 

/* формат сообщения команды формирования дампа */
typedef struct
{ 
	 id_mode_disk_e  id_mode;
	 uint8_t  code_fill;
}cmd_damp_disk_t;	 

/* формат сообщения команд работы с fatfs */
typedef struct
{
	 id_mode_disk_e  id_mode;
	 id_mode_disk_e  id_mode_fill;
}cmd_fatfs_disk_t;	 

/* формат сообщения команд работы с сектором */
typedef struct
{  
	 id_mode_disk_e  id_mode;
	 uint32_t  number;
}cmd_sector_disk_t;		 

/* формат сообщения команд работы с чипом */
typedef struct
{  
	 id_mode_disk_e  id_mode;
}cmd_chip_disk_t;		 

/* формат сообщения команды  */
typedef struct
{
	 id_cmd_disk_e id_cmd_qflash;
	 union
	 {
		 cmd_damp_disk_t cmd_damp;	 
		 cmd_fatfs_disk_t cmd_fatfs;
		 cmd_sector_disk_t cmd_sector;
		 cmd_chip_disk_t cmd_chip;		 
	 };
}cmd_box_disk_t;	 
	 
/**
  * @brief  Вывод строки дампа памяти на экран
  * @param  uint8_t* buff_damp - указатель на буфер дампа 
  * @param  uint16_t index_damp - индекс старта выдачи данных  
  * @param  uint32_t addr_mem - непосредственый адрес памяти данных   
  * @retval uint16_t - число обработанных данных
  */
uint16_t data_damp_print(uint8_t* buff_damp, uint32_t index_damp, uint32_t addr_mem);

/**
  * @brief  Функция визуализации содержимого блока памяти размером 256 байт
  * @param  uint8_t* buff_damp - указатель на буфер дампа 
  * @param  uint32_t damp_size - размер буфера  
  * @param  uint32_t addr_mem - непосредственый адрес памяти данных 
  * @retval void
  */
void bloc_damp_print(uint8_t* buff_damp, uint32_t damp_size, uint32_t addr_mem);

/**
  * @brief  Функция визуализации содержимого блока памяти размером 256 байт
  * @param  uint8_t* pdampa - указатель на буфер дампа 
  * @param  uint8_t* pdampb - указатель на буфер дампа   
  * @param  uint32_t damp_size - размер буфера  
  * @retval bool - true полное совпадение
  *               false нет совпадения 
  */
bool compare_mem_damp(uint8_t* pdampa, uint8_t* pdampb, uint32_t  size_damp);

/**
  * @brief  Функция заполнения блока памяти размером до 0xFFFF байт
  * @param  uint8_t* pdamp - указатель на буфер дампа 
  * @param  id_mode_qflash_e mode_fill - режим заполнения
  * @param  uint16_t offset_damp - смещение заполнения
  * @param  uint16_t size_damp - размер буфера  
  * @param  uint8_t code_fill - код заполнения 
  * @retval none
  */
void fill_damp(uint8_t* pdamp, id_mode_disk_e mode_fill, uint16_t offset_damp, uint16_t size_damp, uint8_t code_fill);
	 
#ifdef __cplusplus
}
#endif

#endif /* __TEST_CMD_DISK_H */