/**
  ******************************************************************************
  * @file    W25Q64JVS.c
  * @author  MCD Application Team
  * @brief   This file provides the W25Q64JVS QSPI driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal_qspi.h"
#include "stm32h7xx_hal_mdma.h"
#include "W25Q64JVS.h"
#include "main.h"

#define W25Q64JVS_delay_ms(a)  vTaskDelay(a)

/**
  * @brief  Return the configuration of the QSPI memory.
  * @param  pInfo pointer on the configuration structure
  * @retval QSPI memory status
  */
int32_t W25Q64JVS_GetFlashInfo(W25Q64JVS_Info_t *pInfo)
{
  pInfo->FlashSize          = W25Q64JVS_FLASH_SIZE;
  pInfo->EraseSectorSize    = W25Q64JVS_SECTOR_SIZE;
  pInfo->ProgPageSize       = W25Q64JVS_PAGE_SIZE;
  pInfo->EraseSectorsNumber = (W25Q64JVS_FLASH_SIZE/pInfo->EraseSectorSize);
  pInfo->ProgPagesNumber    = (W25Q64JVS_FLASH_SIZE/pInfo->ProgPageSize);
  return W25Q64JVS_OK;
}

/* Check if Flash busy ? */
/**
  * @brief  Soft Polling status reg and check masks set low
  *         SPI/QPI;1-0-1/4-0-4
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  uint32_t mask_check 
  * @param  uint32_t time_check
  * @param  uint32_t time_out
  *
  * @retval QSPI memory status
  */
int32_t W25Q64JVS_SoftPollingCheckLoMask(QSPI_HandleTypeDef *Ctx, W25Q64JVS_Interface_t Mode, uint32_t mask_check, uint32_t time_check, uint32_t time_out)
{
	uint8_t  buf_read[3];
	uint32_t temp_status_reg;
	uint32_t cnt_time;	
	QSPI_CommandTypeDef s_command;
	
	//printf("mask= %lu t_chk= %lu time_out= %lu\n", mask_check, time_check, time_out);
	
	cnt_time = 0;
	while (cnt_time < time_out)
	{
		/* Initialize the read flag status register command */
		s_command.InstructionMode   = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
		s_command.Instruction       = W25Q64JVS_READ_STATUS_REG_1_CMD;
		s_command.AddressMode       = QSPI_ADDRESS_NONE;
		s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
		s_command.DataMode          = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_DATA_4_LINES : QSPI_DATA_1_LINE;
		s_command.DummyCycles       = 0;
		s_command.NbData            = 3;
		s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
		s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
		s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

		/* Configure the command */
		if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		{
			return W25Q64JVS_ERROR_COMMAND;
		}

		/* Reception of the data */
		if (HAL_QSPI_Receive(Ctx, buf_read, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		{
			return W25Q64JVS_ERROR_RECEIVE;
		}		

		temp_status_reg = (uint32_t)buf_read[0] || ((uint32_t)buf_read[1]) << 8 || ((uint32_t)buf_read[1]) << 16;
		
		//printf("status %lX %X\n,", temp_status_reg, buf_read[0]);
				
		if ((temp_status_reg & mask_check) == 0) 
		{
			//printf("Ok\n");
			return W25Q64JVS_OK;
		}
			
		/* Delay */
		W25Q64JVS_delay_ms(time_check);
		
		/* Time incremets */
		cnt_time +=	time_check;
	}
	//printf("Err\n");
	return W25Q64JVS_ERROR_COMMAND;
}	

///**
//  * @brief  Polling WIP(Write In Progress) bit become to 0
//  *         SPI/QPI;1-0-1/4-0-4
//  * @param  Ctx Component object pointer
//  * @param  Mode Interface mode
//  * @retval QSPI memory status
//  */
//int32_t W25Q64JVS_AutoPollingMemReady(QSPI_HandleTypeDef *Ctx, W25Q64JVS_Interface_t Mode)
//{
//
//  QSPI_CommandTypeDef     s_command;
//  QSPI_AutoPollingTypeDef s_config;
//
//  /* Configure automatic polling mode to wait for memory ready */
//  s_command.InstructionMode   = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
//  s_command.Instruction       = W25Q64JVS_READ_STATUS_REG_1_CMD;
//  s_command.AddressMode       = QSPI_ADDRESS_NONE;
//  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
//  s_command.DataMode          = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_DATA_4_LINES : QSPI_DATA_1_LINE;
//  s_command.DummyCycles       = 0;
//  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
//  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
//  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
//
//  s_config.Match           = 0;
//  s_config.MatchMode       = QSPI_MATCH_MODE_AND;
//  s_config.Interval        = 0x20;
//  s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;
//  s_config.Mask            = W25Q64JVS_SR1_BUSY;
//  s_config.StatusBytesSize = 1;
//
//  if (HAL_QSPI_AutoPolling(Ctx, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
//  {
//    return W25Q64JVS_ERROR_AUTOPOLLING;
//  }
//
//  return W25Q64JVS_OK;
//}

/**
  * @brief  This function send a Write Enable and wait it is effective.
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */

int32_t W25Q64JVS_WriteEnable(QSPI_HandleTypeDef *Ctx, W25Q64JVS_Interface_t Mode)
{
  QSPI_CommandTypeDef     s_command;
  QSPI_AutoPollingTypeDef s_config;

  /* Enable write operations */
  s_command.InstructionMode   = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;

  s_command.Instruction       = W25Q64JVS_WRITE_ENABLE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return W25Q64JVS_ERROR_COMMAND;
  }

  /* Configure automatic polling mode to wait for write enabling */
  s_config.Match           = W25Q64JVS_SR1_WEL;
  s_config.Mask            = W25Q64JVS_SR1_WEL;
  s_config.MatchMode       = QSPI_MATCH_MODE_AND;
  s_config.StatusBytesSize = 1;
  s_config.Interval        = 0x10;
  s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  s_command.Instruction    = W25Q64JVS_READ_STATUS_REG_1_CMD;
  s_command.DataMode       = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_DATA_4_LINES : QSPI_DATA_1_LINE;

  if (HAL_QSPI_AutoPolling(Ctx, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return W25Q64JVS_ERROR_AUTOPOLLING;
  }

  return W25Q64JVS_OK;
}

/**
  * @brief  This function reset the (WEL) Write Enable Latch bit.
  *         SPI/QPI; 1-0-0/4-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
int32_t W25Q64JVS_WriteDisable(QSPI_HandleTypeDef *Ctx, W25Q64JVS_Interface_t Mode)
{
  QSPI_CommandTypeDef     s_command;
  /* Enable write operations */
  s_command.InstructionMode   = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = W25Q64JVS_WRITE_DISABLE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return W25Q64JVS_ERROR_COMMAND;
  }
  return W25Q64JVS_OK;
}


/**
  * @brief  Writes an amount of data to the QSPI memory.
  *         SPI/QPI; 1-1-1/1-1-4/4-4-4
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  pData Pointer to data to be written
  * @param  WriteAddr Write start address
  * @param  Size Size of data to write. Range 1 ~ 256
  * @retval QSPI memory status
  */

int32_t W25Q64JVS_PageProgram(QSPI_HandleTypeDef *Ctx, W25Q64JVS_Interface_t Mode, uint8_t *pData, uint32_t WriteAddr, uint32_t Size)
{
  QSPI_CommandTypeDef s_command;
  switch(Mode)
  {
  default :
  case W25Q64JVS_SPI_MODE :     /* 1-1-1 commands, Power on H/W default setting */
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction     = W25Q64JVS_PAGE_PROG_CMD;
    s_command.AddressMode     = QSPI_ADDRESS_1_LINE;
    s_command.DataMode        = QSPI_DATA_1_LINE;
    break;

  case W25Q64JVS_QPI_MODE :     /* 4-4-4 commands */
    s_command.InstructionMode = QSPI_INSTRUCTION_4_LINES;
	s_command.Instruction     = W25Q64JVS_PAGE_PROG_CMD;
    s_command.AddressMode     = QSPI_ADDRESS_4_LINES;
    s_command.DataMode        = QSPI_DATA_4_LINES;
    break;

  }

  s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
  s_command.Address           = WriteAddr;
  s_command.NbData            = Size;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
	
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return W25Q64JVS_ERROR_COMMAND;
  }
	
  if (HAL_QSPI_Transmit(Ctx, pData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return W25Q64JVS_ERROR_TRANSMIT;
  }
	
  return W25Q64JVS_OK;
}

/**
  * @brief Receive an amount of data in blocking mode.
  * @param hqspi : QSPI handle
  * @param pData : pointer to data buffer
  * @param Timeout : Timeout duration
  * @note   This function is used only in Indirect Read Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_QSPI_FastReceive(QSPI_HandleTypeDef *hqspi, uint8_t *pData, uint32_t Timeout)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint32_t addr_reg = READ_REG(hqspi->Instance->AR);
	__IO uint32_t *data_reg = &hqspi->Instance->DR;
	uint8_t  *pRxBuffFin;
		
	/* Process locked */
	__HAL_LOCK(hqspi);

	if (hqspi->State == HAL_QSPI_STATE_READY)
	{
		hqspi->ErrorCode = HAL_QSPI_ERROR_NONE;

		if (pData != NULL)
		{
			/* Update state */
			hqspi->State = HAL_QSPI_STATE_BUSY_INDIRECT_RX;

			/* Configure counters and size of the handle */
			hqspi->RxXferCount = READ_REG(hqspi->Instance->DLR) + 1U;
			hqspi->RxXferSize = READ_REG(hqspi->Instance->DLR) + 1U;
			hqspi->pRxBuffPtr = pData;

			pRxBuffFin = hqspi->pRxBuffPtr + hqspi->RxXferCount; 
			
			/* Configure QSPI: CCR register with functional as indirect read */
			MODIFY_REG(hqspi->Instance->CCR, QUADSPI_CCR_FMODE, ((uint32_t)QUADSPI_CCR_FMODE_0));

			/* Start the transfer by re-writing the address in AR register */
			WRITE_REG(hqspi->Instance->AR, addr_reg);

			while (hqspi->pRxBuffPtr < pRxBuffFin)
			{
				while(!((hqspi->Instance->SR)&(QSPI_FLAG_FT | QSPI_FLAG_TC))) {};
                *hqspi->pRxBuffPtr = *((__IO uint8_t *)data_reg);
				hqspi->pRxBuffPtr++;
			}
			/* Update QSPI state */
			hqspi->State = HAL_QSPI_STATE_READY;
		}
		else
		{
			hqspi->ErrorCode |= HAL_QSPI_ERROR_INVALID_PARAM;
			status = HAL_ERROR;
		}
	}
	else
	{
		status = HAL_BUSY;
	}

	/* Process unlocked */
	__HAL_UNLOCK(hqspi);

	return status;
}


/**
  * @brief  Reads an amount of data from the QSPI memory.
  *         SPI/QPI; 1-1-1/1-1-2/1-1-4/4-4-4
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  pData Pointer to data to be read
  * @param  ReadAddr Read start address
  * @param  Size Size of data to read
  * @retval QSPI memory status
  */

int32_t W25Q64JVS_Read(QSPI_HandleTypeDef *Ctx, W25Q64JVS_Interface_t Mode, uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
  QSPI_CommandTypeDef s_command;
  switch(Mode)
  {
  default :	  
  case W25Q64JVS_SPI_MODE:      /* 1-1-1 read commands */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = W25Q64JVS_FAST_READ_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
    s_command.DataMode          = QSPI_DATA_1_LINE;
	s_command.DummyCycles       = 8;

    break;

	  case W25Q64JVS_QPI_MODE:      /* 4-4-4 commands */
    s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
	s_command.Instruction       = W25Q64JVS_FAST_READ_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
    s_command.DataMode          = QSPI_DATA_4_LINES;
    s_command.DummyCycles       = 2; 

    break;
  }
  /* Initialize the read command */
  s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
  s_command.Address           = ReadAddr;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.NbData            = Size;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return W25Q64JVS_ERROR_COMMAND;
  }

  /* Reception of the data */
	if (HAL_QSPI_FastReceive(Ctx, pData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return W25Q64JVS_ERROR_RECEIVE;
  }

  return W25Q64JVS_OK;

}

/**
  * @brief  Reads an amount of data from the QSPI memory.
  *         SPI/QPI; 1-1-1/1-1-2/1-1-4/4-4-4
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  pData Pointer to data to be read
  * @param  ReadAddr Read start address
  * @param  Size Size of data to read
  * @retval QSPI memory status
  */

int32_t W25Q64JVS_DMARead(QSPI_HandleTypeDef *Ctx, W25Q64JVS_Interface_t Mode, uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
	QSPI_CommandTypeDef s_command;
	switch (Mode)
	{
	default :	  
	case W25Q64JVS_SPI_MODE:      /* 1-1-1 read commands */
		s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
		s_command.Instruction       = W25Q64JVS_FAST_READ_CMD;
		s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
		s_command.DataMode          = QSPI_DATA_1_LINE;
		s_command.DummyCycles       = 8;

		break;

	case W25Q64JVS_QPI_MODE:      /* 4-4-4 commands */
		s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
		s_command.Instruction       = W25Q64JVS_FAST_READ_CMD;
		s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
		s_command.DataMode          = QSPI_DATA_4_LINES;
		s_command.DummyCycles       = 2; 

		break;
	}
	/* Initialize the read command */
	s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
	s_command.Address           = ReadAddr;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.NbData            = Size;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Configure the command */
	if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return W25Q64JVS_ERROR_COMMAND;
	}
	if (HAL_QSPI_Receive_DMA(Ctx, pData) != HAL_OK)
	{
		return W25Q64JVS_ERROR_RECEIVE;
	}
	while((Ctx->RxXferCount)>0);
	return W25Q64JVS_OK;
}

/**
  * @brief  Erases the specified block of the QSPI memory.
  *         W25Q64JVS support 4K, 32K, 64K size block erase commands.
  *         SPI/QPI; 1-1-0/4-4-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  BlockAddress Block address to erase
  * @retval QSPI memory status
  */

int32_t W25Q64JVS_BlockErase(QSPI_HandleTypeDef *Ctx, W25Q64JVS_Interface_t Mode ,uint32_t BlockAddress, W25Q64JVS_Erase_t BlockSize)
{
  QSPI_CommandTypeDef s_command;
  switch(BlockSize)
  {
  default :
  case W25Q64JVS_ERASE_4K :
    s_command.Instruction     = W25Q64JVS_SUBSECTOR_ERASE_CMD_4K; /*!< 4K size Sector erase */
    break;	  
		  
  case W25Q64JVS_ERASE_32K :
    s_command.Instruction     = W25Q64JVS_SUBSECTOR_ERASE_CMD_32K;/*!< 32K size Block erase */
    break;		  
	  
  case W25Q64JVS_ERASE_64K :
	  s_command.Instruction   = W25Q64JVS_SUBSECTOR_ERASE_CMD_64K;/*!< 64K size Block erase */
    break;

  case W25Q64JVS_ERASE_CHIP :/*!< Whole chip erase     */
    return W25Q64JVS_ChipErase(Ctx, Mode);
  }
  /* Initialize the erase command */

  s_command.InstructionMode   = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.AddressMode       = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_ADDRESS_4_LINES : QSPI_ADDRESS_1_LINE; 
  s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
  s_command.Address           = BlockAddress;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  W25Q64JVS_WriteEnable(Ctx,Mode);
	
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return W25Q64JVS_ERROR_COMMAND;
  }
	
  switch(BlockSize)
  {
  default :
  case W25Q64JVS_ERASE_4K :
	  return W25Q64JVS_SoftPollingCheckLoMask(Ctx, Mode, W25Q64JVS_SR_BUSY, W25Q64JVS_SECTOR_ERASE_CHECK_TIME, W25Q64JVS_SECTOR_ERASE_TIMEOUT);
 
  case W25Q64JVS_ERASE_32K :
	  return W25Q64JVS_SoftPollingCheckLoMask(Ctx, Mode, W25Q64JVS_SR_BUSY, W25Q64JVS_BLOCK_ERASE_CHECK_TIME, W25Q64JVS_BLOCK_ERASE_TIMEOUT);

  case W25Q64JVS_ERASE_64K :
	  return W25Q64JVS_SoftPollingCheckLoMask(Ctx, Mode, W25Q64JVS_SR_BUSY, W25Q64JVS_BLOCK_ERASE_CHECK_TIME, W25Q64JVS_BLOCK_ERASE_TIMEOUT);
  }	

  return W25Q64JVS_OK;
}

/**
  * @brief  Whole chip erase.
  *         SPI/QPI; 1-0-0/4-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */

int32_t W25Q64JVS_ChipErase(QSPI_HandleTypeDef *Ctx, W25Q64JVS_Interface_t Mode)
{
	QSPI_CommandTypeDef s_command;

	/* Initialize the erase command */
	s_command.InstructionMode   = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = W25Q64JVS_CHIP_ERASE_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_NONE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return W25Q64JVS_ERROR_COMMAND;
	}
	return W25Q64JVS_SoftPollingCheckLoMask(Ctx, Mode, W25Q64JVS_SR_BUSY, W25Q64JVS_CHIP_ERASE_CHECK_TIME, W25Q64JVS_CHIP_ERASE_TIMEOUT);
}

/**
  * @brief  Read Flash Status register value
  *         SPI/QPI; 1-0-1/4-0-4
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  Value pointer to status register value
  * @retval QSPI memory status
  */
int32_t W25Q64JVS_ReadStatusRegister(QSPI_HandleTypeDef *Ctx, W25Q64JVS_Interface_t Mode, uint8_t *Value)
{
  QSPI_CommandTypeDef s_command;
  /* Initialize the read flag status register command */
  s_command.InstructionMode   = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = W25Q64JVS_READ_STATUS_REG_1_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_DATA_4_LINES : QSPI_DATA_1_LINE;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 1;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return W25Q64JVS_ERROR_COMMAND;
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(Ctx,Value, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return W25Q64JVS_ERROR_RECEIVE;
  }

  return W25Q64JVS_OK;
}

/**
  * @brief  Read Flash Status register value
  *         SPI/QPI; 1-0-1/4-0-4
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  Value pointer to status register value
  * @retval QSPI memory status
  */
int32_t W25Q64JVS_ReadStatusAllRegister(QSPI_HandleTypeDef *Ctx, W25Q64JVS_Interface_t Mode, uint8_t *Value)
{
	QSPI_CommandTypeDef s_command;
	/* Initialize the read flag status register command */
	s_command.InstructionMode   = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = W25Q64JVS_READ_STATUS_REG_1_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_DATA_4_LINES : QSPI_DATA_1_LINE;
	s_command.DummyCycles       = 0;
	s_command.NbData            = 3;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Configure the command */
	if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return W25Q64JVS_ERROR_COMMAND;
	}

	/* Reception of the data */
	if (HAL_QSPI_Receive(Ctx, Value, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return W25Q64JVS_ERROR_RECEIVE;
	}

	return W25Q64JVS_OK;
}

/**
  * @brief  This function put QSPI memory in QPI mode (Quad I/O) from SPI mode.
  *         SPI -> QPI; 1-x-x -> 4-4-4
  *         SPI; 1-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
int32_t W25Q64JVS_EnterQPIMode(QSPI_HandleTypeDef *Ctx)
{
  QSPI_CommandTypeDef s_command;

  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = W25Q64JVS_ENTER_QUAD_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return W25Q64JVS_ERROR_COMMAND;
  }

  return W25Q64JVS_OK;
}
/**
  * @brief  This function put QSPI memory in SPI mode (Single I/O) from QPI mode.
  *         QPI -> SPI; 4-4-4 -> 1-x-x
  *         QPI; 4-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
int32_t W25Q64JVS_ExitQPIMode(QSPI_HandleTypeDef *Ctx)
{
  QSPI_CommandTypeDef s_command;

  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = W25Q64JVS_EXIT_QUAD_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return W25Q64JVS_ERROR_COMMAND;
  }

  return W25Q64JVS_OK;
}

/**
  * @brief  Reads an amount of data from the QSPI memory.
  *         SPI/QPI; 1-1-1/1-1-2/1-1-4/4-4-4
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */

int32_t W25Q64JVS_EnableMemoryMappedMode(QSPI_HandleTypeDef *Ctx, W25Q64JVS_Interface_t Mode)
{
	QSPI_CommandTypeDef      s_command;
	QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;
	switch (Mode)
	{
	default :	  
	case W25Q64JVS_SPI_MODE:      /* 1-1-1 read commands */
		s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
		s_command.Instruction       = W25Q64JVS_FAST_READ_CMD;
		s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
		s_command.DataMode          = QSPI_DATA_1_LINE;
		s_command.DummyCycles       = 8;


		break;

	case W25Q64JVS_QPI_MODE:      /* 4-4-4 commands */
		s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
		s_command.Instruction       = W25Q64JVS_FAST_READ_CMD;
		s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
		s_command.DataMode          = QSPI_DATA_4_LINES;
		s_command.DummyCycles       = 2; 

		break;
	}
	/* Configure the command for the read instruction */
	s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Configure the memory mapped mode */
	s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
	s_mem_mapped_cfg.TimeOutPeriod     = 0;

	if (HAL_QSPI_MemoryMapped(Ctx, &s_command, &s_mem_mapped_cfg) != HAL_OK)
	{
		return W25Q64JVS_ERROR_MEMORYMAPPED;
	}

	return W25Q64JVS_OK;
}

/**
  * @brief  Flash reset enable command
  *         SPI/QPI; 1-0-0, 4-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
int32_t W25Q64JVS_ResetEnable(QSPI_HandleTypeDef *Ctx, W25Q64JVS_Interface_t Mode)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the reset enable command */
  s_command.InstructionMode   = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = W25Q64JVS_RESET_ENABLE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return W25Q64JVS_ERROR_COMMAND;
  }

  return W25Q64JVS_OK;
}


/**
  * @brief  Flash reset memory command
  *         SPI/QPI; 1-0-0, 4-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
int32_t W25Q64JVS_ResetMemory(QSPI_HandleTypeDef *Ctx, W25Q64JVS_Interface_t Mode)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the reset enable command */
  s_command.InstructionMode   = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = W25Q64JVS_RESET_MEMORY_CMD ;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return W25Q64JVS_ERROR_COMMAND;
  }

  return W25Q64JVS_OK;
}


/**
  * @brief  Read Flash 3 Byte IDs.
  *         Manufacturer ID, Memory type, Memory density
  *         SPI/QPI; 1-0-1/4-0-4
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @param  ID pointer to flash id value
  * @retval QSPI memory status
  */
int32_t W25Q64JVS_ReadID(QSPI_HandleTypeDef *Ctx, W25Q64JVS_Interface_t Mode, uint8_t *ID)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the read ID command */
  s_command.InstructionMode   = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = W25Q64JVS_READ_ID_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DummyCycles       = 0;
  s_command.DataMode          = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_DATA_4_LINES : QSPI_DATA_1_LINE;
  s_command.NbData            = 3;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return W25Q64JVS_ERROR_COMMAND;
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(Ctx, ID, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return W25Q64JVS_ERROR_RECEIVE;
  }

  return W25Q64JVS_OK;
}

/**
  * @brief  Program/Erases suspend. Interruption Program/Erase operations.
  *         After the device has entered Erase-Suspended mode,
  *         system can read any address except the block/sector being Program/Erased.
  *         SPI/QPI; 1-0-0/4-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */

int32_t W25Q64JVS_ProgEraseSuspend(QSPI_HandleTypeDef *Ctx, W25Q64JVS_Interface_t Mode)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the read ID command */
  s_command.InstructionMode   = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = W25Q64JVS_PROG_ERASE_SUSPEND_CMD ;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DummyCycles       = 0;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return W25Q64JVS_ERROR_COMMAND;
  }

  return W25Q64JVS_OK;
}

/**
  * @brief  Program/Erases resume.
  *         SPI/QPI; 1-0-0/4-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
int32_t W25Q64JVS_ProgEraseResume(QSPI_HandleTypeDef *Ctx, W25Q64JVS_Interface_t Mode)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the read ID command */
  s_command.InstructionMode   = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = W25Q64JVS_PROG_ERASE_RESUME_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DummyCycles       = 0;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return W25Q64JVS_ERROR_COMMAND;
  }

  return W25Q64JVS_OK;
}

/**
  * @brief  Deep power down.
  *         The device is not active and all Write/Program/Erase instruction are ignored.
  *         SPI/QPI; 1-0-0/4-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
int32_t W25Q64JVS_EnterDeepPowerDown(QSPI_HandleTypeDef *Ctx, W25Q64JVS_Interface_t Mode)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the read ID command */
  s_command.InstructionMode   = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = W25Q64JVS_ENTER_DEEP_POWER_DOWN;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DummyCycles       = 0;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return W25Q64JVS_ERROR_COMMAND;
  }

  return W25Q64JVS_OK;
}

/**
  * @brief  Release from deep power down.
  *         After CS# go high, system need wait tRES1 time for device ready.
  *         SPI/QPI; 1-0-0/4-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
int32_t W25Q64JVS_ReleaseFromDeepPowerDown(QSPI_HandleTypeDef *Ctx, W25Q64JVS_Interface_t Mode)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the read ID command */
  s_command.InstructionMode   = (Mode == W25Q64JVS_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = W25Q64JVS_RELEASE_FROM_DEEP_POWER_DOWN ;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DummyCycles       = 0;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return W25Q64JVS_ERROR_COMMAND;
  }

  return W25Q64JVS_OK;
}
