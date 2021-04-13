/**
  ******************************************************************************
  * @file    drv_qspi_w25q128.c
  * @author  MCD Application Team
  * @brief   This file includes a standard driver for the QSPI memory .
  ******************************************************************************
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "drv_qspi_w25q64.h"
#include "stm32h7xx_hal_mdma.h"

uint8_t QspiMdmaBuf[4096] __attribute__((section(".MdmaSection"))); /* Buffers */
BSP_QSPI_Hndlr_t    pqspi;

/* Private functions ---------------------------------------------------------*/

static void QSPI_MspInit(QSPI_HandleTypeDef *hQspi);
static void QSPI_MspDeInit(QSPI_HandleTypeDef *hSspi);
static int32_t QSPI_ResetMemory(void);

/**
  * @brief  Initializes the QSPI interface.
  * @param  Instance   QSPI Instance
  * @retval BSP status
  */
int32_t BSP_QSPI_Init(void)
{
	BSP_QSPI_Init_t Init;
	int32_t ret = BSP_ERROR_NONE;
	BSP_QSPI_Info_t pInfo;
	MX_QSPI_Init_t qspi_init;
	static const uint32_t Prescaler = 19;
	
	Init.InterfaceMode = W25Q64JVS_SPI_MODE;
	Init.DualFlashMode = W25Q64JVS_DUALFLASH_DISABLE;	

	/* Check if instance is already initialized */
	if (pqspi.IsInitialized == QSPI_ACCESS_NONE)
	{
#if (USE_HAL_QSPI_REGISTER_CALLBACKS == 1)
		/* Register the QSPI MSP Callbacks */
		if (pqspi.IsMspCallbacksValid == 0UL)
		{
			if (BSP_QSPI_RegisterDefaultMspCallbacks(Instance) != BSP_ERROR_NONE)
			{
				ret = BSP_ERROR_PERIPH_FAILURE;
			}
		}
#else
		/* Msp QSPI initialization */
		QSPI_MspInit(&(pqspi.hqspi));
#endif /* USE_HAL_QSPI_REGISTER_CALLBACKS */

		/* STM32 QSPI interface initialization */
		(void)W25Q64JVS_GetFlashInfo(&pInfo);
		qspi_init.ClockPrescaler = Prescaler;
		qspi_init.DualFlashMode  = QSPI_DUALFLASH_DISABLE;
		qspi_init.FlashSize      = (uint32_t)POSITION_VAL((uint32_t)pInfo.FlashSize) - 1U;
		qspi_init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;

		if (MX_QSPI_Init(&(pqspi.hqspi), &qspi_init) != HAL_OK)
		{
			ret = BSP_ERROR_PERIPH_FAILURE;
		}/* QSPI memory reset */
		else if(QSPI_ResetMemory() != BSP_ERROR_NONE)
		{
			ret = BSP_ERROR_COMPONENT_FAILURE;
		}
		else if(W25Q64JVS_SoftPollingCheckLoMask(&(pqspi.hqspi), pqspi.InterfaceMode, W25Q64JVS_SR_BUSY, W25Q64JVS_BUSY_CHECK_TIME, W25Q64JVS_BUSY_TIMEOUT) != W25Q64JVS_OK)
		{
			ret = BSP_ERROR_COMPONENT_FAILURE;
		}
	}
	/* Return BSP status */
	return ret;
}

/**
  * @brief  De-Initializes the QSPI interface.
  * @retval BSP status
  */
int32_t BSP_QSPI_DeInit(void)
{
  int32_t ret = BSP_ERROR_NONE;

    if(pqspi.IsInitialized == QSPI_ACCESS_MMP)
    {
      if(BSP_QSPI_DisableMemoryMappedMode() != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
    }

    if(ret == BSP_ERROR_NONE)
    {
      /* Set default QSPI_Ctx values */
      pqspi.IsInitialized = QSPI_ACCESS_NONE;
      pqspi.InterfaceMode = BSP_QSPI_SPI_MODE;
      pqspi.DualFlashMode = QSPI_DUALFLASH_ENABLE;

#if (USE_HAL_QSPI_REGISTER_CALLBACKS == 0)
      QSPI_MspDeInit(&(pqspi.hqspi));
#endif /* (USE_HAL_QSPI_REGISTER_CALLBACKS == 0) */

      /* Call the DeInit function to reset the driver */
      if (HAL_QSPI_DeInit(&(pqspi.hqspi)) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Initializes the QSPI interface.
  * @param  hQspi       QSPI handle
  * @param  Config      QSPI configuration structure
  * @retval BSP status
  */
__weak HAL_StatusTypeDef MX_QSPI_Init(QSPI_HandleTypeDef *hQspi, MX_QSPI_Init_t *Config)
{
  /* QSPI initialization */
  /* QSPI freq = SYSCLK /(1 + ClockPrescaler) Mhz */
  hQspi->Instance                = QUADSPI;
  hQspi->Init.ClockPrescaler     = Config->ClockPrescaler;
  hQspi->Init.FifoThreshold      = 1;
  hQspi->Init.SampleShifting     = Config->SampleShifting;
  hQspi->Init.FlashSize          = Config->FlashSize;
  hQspi->Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE; /* Min 50ns for nonRead */
  hQspi->Init.ClockMode          = QSPI_CLOCK_MODE_0;
  hQspi->Init.FlashID            = QSPI_FLASH_ID_1;
  hQspi->Init.DualFlash          = Config->DualFlashMode;

  return HAL_QSPI_Init(hQspi);
}

#if (USE_HAL_QSPI_REGISTER_CALLBACKS == 1)
/**
  * @brief Default BSP QSPI Msp Callbacks
  * @param Instance      QSPI Instance
  * @retval BSP status
  */
int32_t BSP_QSPI_RegisterDefaultMspCallbacks (uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if(Instance >= QSPI_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Register MspInit/MspDeInit Callbacks */
    if(HAL_QSPI_RegisterCallback(&(pqspi.hqspi), HAL_QSPI_MSPINIT_CB_ID, QSPI_MspInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_QSPI_RegisterCallback(&(pqspi.hqspi), HAL_QSPI_MSPDEINIT_CB_ID, QSPI_MspDeInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      pqspi.IsMspCallbacksValid = 1U;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief BSP QSPI Msp Callback registering
  * @param Instance     QSPI Instance
  * @param CallBacks    pointer to MspInit/MspDeInit callbacks functions
  * @retval BSP status
  */
int32_t BSP_QSPI_RegisterMspCallbacks (uint32_t Instance, BSP_QSPI_Cb_t *CallBacks)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check if the instance is supported */
  if(Instance >= QSPI_INSTANCES_NUMBER)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Register MspInit/MspDeInit Callbacks */
    if(HAL_QSPI_RegisterCallback(&(pqspi.hqspi), HAL_QSPI_MSPINIT_CB_ID, CallBacks->pMspInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_QSPI_RegisterCallback(&(pqspi.hqspi), HAL_QSPI_MSPDEINIT_CB_ID, CallBacks->pMspDeInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      pqspi.IsMspCallbacksValid = 1U;
    }
  }

  /* Return BSP status */
  return ret;
}
#endif /* (USE_HAL_QSPI_REGISTER_CALLBACKS == 1) */

/**
  * @brief  Reads an amount of data from the QSPI memory.
  * @param  pData     Pointer to data to be read
  * @param  ReadAddr  Read start address
  * @param  Size      Size of data to read
  * @retval BSP status
  */
int32_t BSP_QSPI_Read(uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
  int32_t ret = BSP_ERROR_NONE;
	
	if (W25Q64JVS_Read(&(pqspi.hqspi), pqspi.InterfaceMode, pData, ReadAddr, Size) != W25Q64JVS_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Reads an amount of data from the QSPI memory.
  * @param  pData     Pointer to data to be read
  * @param  ReadAddr  Read start address
  * @param  Size      Size of data to read
  * @retval BSP status
  */
int32_t BSP_QSPI_DMARead(uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
  int32_t ret = BSP_ERROR_NONE;
	
	if (W25Q64JVS_DMARead(&(pqspi.hqspi), pqspi.InterfaceMode, QspiMdmaBuf, ReadAddr, Size) != W25Q64JVS_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
	
	for (uint32_t cntic = 0; cntic < (Size>>2); cntic++)
	{
		((uint32_t*)pData)[cntic] = ((uint32_t*)QspiMdmaBuf)[cntic];
    }	
	
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Writes an amount of data to the QSPI memory.
  * @param  pData      Pointer to data to be written
  * @param  WriteAddr  Write start address
  * @param  Size       Size of data to write
  * @retval BSP status
  */
int32_t BSP_QSPI_Write( uint8_t *pData, uint32_t WriteAddr, uint32_t Size)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t end_addr, current_size, current_addr;
  uint8_t *write_data;

    /* Calculation of the size between the write address and the end of the page */
    current_size = W25Q64JVS_PAGE_SIZE - (WriteAddr % W25Q64JVS_PAGE_SIZE);

    /* Check if the size of the data is less than the remaining place in the page */
    if (current_size > Size)
    {
      current_size = Size;
    }

    /* Initialize the address variables */
    current_addr = WriteAddr;
    end_addr = WriteAddr + Size;
    write_data = pData;

    /* Perform the write page by page */
    do
    {
   
      /* Check if Flash busy ? */
	  if(W25Q64JVS_SoftPollingCheckLoMask(&(pqspi.hqspi), pqspi.InterfaceMode, W25Q64JVS_SR_BUSY, W25Q64JVS_BUSY_CHECK_TIME, W25Q64JVS_BUSY_TIMEOUT) != W25Q64JVS_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }/* Enable write operations */
      else if(W25Q64JVS_WriteEnable(&(pqspi.hqspi), pqspi.InterfaceMode) != W25Q64JVS_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }/* Issue page program command */
      else if(W25Q64JVS_PageProgram(&(pqspi.hqspi), pqspi.InterfaceMode, write_data, current_addr, current_size) != W25Q64JVS_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }/* Configure automatic polling mode to wait for end of program */
	    else if(W25Q64JVS_SoftPollingCheckLoMask(&(pqspi.hqspi), pqspi.InterfaceMode, W25Q64JVS_SR_BUSY, W25Q64JVS_PAGE_PROGRAM_CHECK_TIME, W25Q64JVS_PAGE_PROGRAM_TIMEOUT) != W25Q64JVS_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else
      {
        /* Update the address and size variables for next page programming */
        current_addr += current_size;
        write_data += current_size;
        current_size = ((current_addr + W25Q64JVS_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : W25Q64JVS_PAGE_SIZE;
      }

    } while ((current_addr < end_addr) && (ret == BSP_ERROR_NONE));

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Erases the specified block of the QSPI memory.
  *         W25Q64JVS support 4K, 32K, 64K size block erase commands.
  * @param  BlockAddress Block address to erase
  * @param  BlockSize    Erase Block size
  * @retval BSP status
  */
int32_t BSP_QSPI_EraseBlock(uint32_t BlockAddress, BSP_QSPI_Erase_t BlockSize)
{
  int32_t ret = BSP_ERROR_NONE;
	/* Check if Flash busy ? */
	if (W25Q64JVS_SoftPollingCheckLoMask(&(pqspi.hqspi), pqspi.InterfaceMode, W25Q64JVS_SR_BUSY, W25Q64JVS_BUSY_CHECK_TIME, W25Q64JVS_BUSY_TIMEOUT) != W25Q64JVS_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }/* Enable write operations */
    else if(W25Q64JVS_WriteEnable(&(pqspi.hqspi), pqspi.InterfaceMode) != W25Q64JVS_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      /* Issue Block Erase command */
      if(W25Q64JVS_BlockErase(&(pqspi.hqspi), pqspi.InterfaceMode, BlockAddress, BlockSize) != W25Q64JVS_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
    }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Erases the entire QSPI memory.
  * @retval BSP status
  */
int32_t BSP_QSPI_EraseChip(void)
{
  int32_t ret = BSP_ERROR_NONE;

	/* Check if Flash busy ? */
	if (W25Q64JVS_SoftPollingCheckLoMask(&(pqspi.hqspi), pqspi.InterfaceMode, W25Q64JVS_SR_BUSY, W25Q64JVS_BUSY_CHECK_TIME, W25Q64JVS_BUSY_TIMEOUT) != W25Q64JVS_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }/* Enable write operations */
    else if (W25Q64JVS_WriteEnable(&(pqspi.hqspi), pqspi.InterfaceMode) != W25Q64JVS_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      /* Issue Chip erase command */
      if(W25Q64JVS_ChipErase(&(pqspi.hqspi), pqspi.InterfaceMode) != W25Q64JVS_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
    }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Reads current status reg1,reg2,reg3 of the QSPI memory.
  * @param  reg_statue - point to array status reg
  *         If WIP != 0 then return busy.
  * @retval QSPI memory status: whether busy or not
  */
int32_t BSP_QSPI_GetAllStatus(uint8_t *reg_statue)
{
	int32_t ret = BSP_ERROR_NONE;

	if (W25Q64JVS_ReadStatusAllRegister(&(pqspi.hqspi), pqspi.InterfaceMode, reg_statue) != W25Q64JVS_OK)
	{
		ret = BSP_ERROR_COMPONENT_FAILURE;
	}
	else
	{
		/* Check the value of the register */
		if ((reg_statue[0] & (uint8_t)W25Q64JVS_SR1_BUSY) != 0U)
		{
			ret = BSP_ERROR_BUSY;
		}
	}
	/* Return BSP status */
	return ret;
}

/**
  * @brief  Reads current status of the QSPI memory.
  *         If WIP != 0 then return busy.
  * @retval QSPI memory status: whether busy or not
  */
int32_t BSP_QSPI_GetStatus(void)
{
  int32_t ret = BSP_ERROR_NONE;
  uint8_t reg;

    if(W25Q64JVS_ReadStatusRegister(&(pqspi.hqspi), pqspi.InterfaceMode, &reg) != W25Q64JVS_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      /* Check the value of the register */
	    if ((reg & (uint8_t)W25Q64JVS_SR1_BUSY) != 0U)
      {
        ret = BSP_ERROR_BUSY;
      }
    }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Return the configuration of the QSPI memory.
  * @param  pInfo     pointer on the configuration structure
  * @retval BSP status
  */
int32_t BSP_QSPI_GetInfo(BSP_QSPI_Info_t *pInfo)
{
  int32_t ret = BSP_ERROR_NONE;

    (void)W25Q64JVS_GetFlashInfo(pInfo);

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Configure the QSPI in memory-mapped mode
  *         Only 1 Instance can running MMP mode. And it will lock system at this mode.
  * @retval BSP status
  */
int32_t BSP_QSPI_EnableMemoryMappedMode(void)
{
  int32_t ret = BSP_ERROR_NONE;

      if(W25Q64JVS_EnableMemoryMappedMode(&(pqspi.hqspi), pqspi.InterfaceMode) != W25Q64JVS_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else /* Update QSPI context if all operations are well done */
      {
        pqspi.IsInitialized = QSPI_ACCESS_MMP;
      }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Exit form memory-mapped mode
  *         Only 1 Instance can running MMP mode. And it will lock system at this mode.
  * @retval BSP status
  */
int32_t BSP_QSPI_DisableMemoryMappedMode(void)
{
	uint8_t Dummy;
	int32_t ret = BSP_ERROR_NONE;

	/* Check if the instance is supported */
	if (pqspi.IsInitialized != QSPI_ACCESS_MMP)
	{
		ret = BSP_ERROR_QSPI_MMP_UNLOCK_FAILURE;
	}/* Abort MMP back to indirect mode */
	else if(HAL_QSPI_Abort(&(pqspi.hqspi)) != HAL_OK)
	{
		ret = BSP_ERROR_PERIPH_FAILURE;
	}
	else
	{
		/* Force QSPI interface Sampling Shift to half cycle */
		pqspi.hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;

		if (HAL_QSPI_Init(&(pqspi.hqspi)) != HAL_OK)
		{
			ret = BSP_ERROR_PERIPH_FAILURE;
		}
		/* Dummy read for exit from Performance Enhance mode */
		else if(W25Q64JVS_DMARead(&(pqspi.hqspi), pqspi.InterfaceMode, &Dummy, 0, 1) != W25Q64JVS_OK)
		{
			ret = BSP_ERROR_COMPONENT_FAILURE;
		}
		else /* Update QSPI context if all operations are well done */
		{
			pqspi.IsInitialized = QSPI_ACCESS_INDIRECT;
		}
	}
	/* Return BSP status */
	return ret;
}

/**
  * @brief  Get flash ID, 3 Byte
  *         Manufacturer ID, Memory type, Memory density
  * @param  Id QSPI Identifier
  * @retval BSP status
  */
int32_t BSP_QSPI_ReadID(uint8_t *Id)
{
  int32_t ret = BSP_ERROR_NONE;

    if(W25Q64JVS_ReadID(&(pqspi.hqspi), pqspi.InterfaceMode, Id) != W25Q64JVS_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set Flash to desired Interface mode. And this instance becomes current instance.
  *         If current instance running at MMP mode then this function isn't work.
  *         Indirect -> Indirect
  * @param  Mode      QSPI mode
  * @retval BSP status
  */
int32_t BSP_QSPI_ConfigFlash(BSP_QSPI_Interface_t Mode)
{
  int32_t ret = BSP_ERROR_NONE;

    /* Check if MMP mode locked ************************************************/
    if(pqspi.IsInitialized == QSPI_ACCESS_MMP)
    {
      ret = BSP_ERROR_QSPI_MMP_LOCK_FAILURE;
    }
    else
    {
      /* Setup MCU transfer rate setting ***************************************************/
	  pqspi.hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;

      if(HAL_QSPI_Init(&(pqspi.hqspi))!= HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
        /* Setup Flash interface ***************************************************/
        switch(pqspi.InterfaceMode)
        {
        case W25Q64JVS_QPI_MODE :               /* 4-4-4 commands */
          if(Mode != W25Q64JVS_QPI_MODE)
          {
            if(W25Q64JVS_ExitQPIMode(&(pqspi.hqspi)) != W25Q64JVS_OK)
            {
              ret = BSP_ERROR_COMPONENT_FAILURE;
            }
          }
          break;

        case BSP_QSPI_SPI_MODE :               /* 1-1-1 commands, Power on H/W default setting */
        default :
          if(Mode == W25Q64JVS_QPI_MODE)
          {
            if(W25Q64JVS_EnterQPIMode(&(pqspi.hqspi)) != W25Q64JVS_OK)
            {
              ret = BSP_ERROR_COMPONENT_FAILURE;
            }
          }
          break;
        }

        /* Update QSPI context if all operations are well done */
        if(ret == BSP_ERROR_NONE)
        {
          /* Update current status parameter *****************************************/
          pqspi.IsInitialized = QSPI_ACCESS_INDIRECT;
          pqspi.InterfaceMode = Mode;
        }
      }
    }
  /* Return BSP status */
  return ret;
}

/**
  * @brief QSPI MSP Initialization
  * @param hQspi : QSPI handle
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  *           - NVIC configuration for QSPI interrupt
  * @retval None
  */
static void QSPI_MspInit(QSPI_HandleTypeDef *hQspi)
{
  GPIO_InitTypeDef gpio_init_structure;
  static MDMA_HandleTypeDef hmdma;
	
  /* Prevent unused argument(s) compilation warning */
  //UNUSED(hQspi);

  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable the QuadSPI memory interface clock */
  QSPI_CLK_ENABLE();
  QSPI_MDMA_CLK_ENABLE(); 		
	
  /* Reset the QuadSPI memory interface */
  QSPI_FORCE_RESET();
  QSPI_RELEASE_RESET();
  /* Enable GPIO clocks */
  QSPI_CLK_GPIO_CLK_ENABLE();
  QSPI_BK1_CS_GPIO_CLK_ENABLE();
  QSPI_BK1_D0_GPIO_CLK_ENABLE();
  QSPI_BK1_D1_GPIO_CLK_ENABLE();
  QSPI_BK1_D2_GPIO_CLK_ENABLE();
  QSPI_BK1_D3_GPIO_CLK_ENABLE();
	
  /*##-2- Configure peripheral GPIO ##########################################*/
  /* QSPI CLK GPIO pin configuration  */
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Pull      = GPIO_NOPULL;
  gpio_init_structure.Pin       = QSPI_CLK_PIN;
  gpio_init_structure.Alternate = QSPI_CLK_GPIO_ALTERNATE;
  HAL_GPIO_Init(QSPI_CLK_GPIO_PORT, &gpio_init_structure);

  /* QSPI CS GPIO pin configuration  */
  gpio_init_structure.Pin       = QSPI_BK1_CS_PIN;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Alternate = QSPI_BK1_CS_GPIO_ALTERNATE;
  HAL_GPIO_Init(QSPI_BK1_CS_GPIO_PORT, &gpio_init_structure);

  /* QSPI D0 GPIO pin configuration  */
  gpio_init_structure.Pin       = QSPI_BK1_D0_PIN;
  gpio_init_structure.Pull      = GPIO_NOPULL;
  gpio_init_structure.Alternate = QSPI_BK1_D0_GPIO_ALTERNATE;
  HAL_GPIO_Init(QSPI_BK1_D0_GPIO_PORT, &gpio_init_structure);

  /* QSPI D1 GPIO pin configuration  */
  gpio_init_structure.Pin       = QSPI_BK1_D1_PIN;
  gpio_init_structure.Alternate = QSPI_BK1_D1_GPIO_ALTERNATE;
  HAL_GPIO_Init(QSPI_BK1_D1_GPIO_PORT, &gpio_init_structure);

  /* QSPI D2 GPIO pin configuration  */
  gpio_init_structure.Pin       = QSPI_BK1_D2_PIN;
  gpio_init_structure.Alternate = QSPI_BK1_D2_GPIO_ALTERNATE;
  HAL_GPIO_Init(QSPI_BK1_D2_GPIO_PORT, &gpio_init_structure);

  /* QSPI D3 GPIO pin configuration  */
  gpio_init_structure.Pin       = QSPI_BK1_D3_PIN;
  gpio_init_structure.Alternate = QSPI_BK1_D3_GPIO_ALTERNATE;	
  HAL_GPIO_Init(QSPI_BK1_D3_GPIO_PORT, &gpio_init_structure);

  /*##-3- Configure the NVIC for QSPI #########################################*/
  /* NVIC configuration for QSPI interrupt */
  HAL_NVIC_SetPriority(QUADSPI_IRQn, 0x06, 0);
  HAL_NVIC_EnableIRQ(QUADSPI_IRQn);
	
  /*##-4- Configure the DMA channel ###########################################*/
  /* Enable MDMA clock */
  /* Input MDMA */
  /* Set the parameters to be configured */ 
  hmdma.Init.Request = MDMA_REQUEST_QUADSPI_FIFO_TH;
  hmdma.Init.TransferTriggerMode = MDMA_BUFFER_TRANSFER;
  hmdma.Init.Priority = MDMA_PRIORITY_HIGH;
  hmdma.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
  
  hmdma.Init.SourceInc = MDMA_SRC_INC_BYTE;
  hmdma.Init.DestinationInc = MDMA_DEST_INC_DISABLE;
  hmdma.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
  hmdma.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
  hmdma.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
  hmdma.Init.BufferTransferLength = 4;
  hmdma.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
  hmdma.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
  
  hmdma.Init.SourceBlockAddressOffset = 0;
  hmdma.Init.DestBlockAddressOffset = 0;
  
  hmdma.Instance = MDMA_Channel1;

	/* Associate the DMA handle */
  __HAL_LINKDMA(hQspi, hmdma, hmdma);
  
  /* DeInitialize the MDMA Stream */
  HAL_MDMA_DeInit(&hmdma);      
  /* Initialize the MDMA stream */
  HAL_MDMA_Init(&hmdma);	
		
  /* Enable and set QuadSPI interrupt to the lowest priority */
  HAL_NVIC_SetPriority(MDMA_IRQn, 0x06, 0);
  HAL_NVIC_EnableIRQ(MDMA_IRQn);	
}

/**
  * @brief QSPI MSP De-Initialization
  * @param hQspi  QSPI handle
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO and NVIC configuration to their default state
  * @retval None
  */
static void QSPI_MspDeInit(QSPI_HandleTypeDef *hQspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hQspi);

  /*##-2- Disable peripherals and GPIO Clocks ################################*/
  /* De-Configure QSPI pins */
  HAL_GPIO_DeInit(QSPI_CLK_GPIO_PORT, QSPI_CLK_PIN);
  HAL_GPIO_DeInit(QSPI_BK1_CS_GPIO_PORT, QSPI_BK1_CS_PIN);
  HAL_GPIO_DeInit(QSPI_BK1_D0_GPIO_PORT, QSPI_BK1_D0_PIN);
  HAL_GPIO_DeInit(QSPI_BK1_D1_GPIO_PORT, QSPI_BK1_D1_PIN);
  HAL_GPIO_DeInit(QSPI_BK1_D2_GPIO_PORT, QSPI_BK1_D2_PIN);
  HAL_GPIO_DeInit(QSPI_BK1_D3_GPIO_PORT, QSPI_BK1_D3_PIN);

  /*##-3- Reset peripherals ##################################################*/
  /* Reset the QuadSPI memory interface */
  QSPI_FORCE_RESET();
  QSPI_RELEASE_RESET();

  /* Disable the QuadSPI memory interface clock */
  QSPI_CLK_DISABLE();
}

/**
  * @brief  This function reset the QSPI Flash memory.
  *         Fore QPI+SPI reset to avoid system come from unknown status.
  *         Flash accept 1-1-1, 1-1-2, 1-2-2 commands after reset.
  * @param  none
  * @retval BSP status
  */
static int32_t QSPI_ResetMemory(void)
{
	int32_t ret = BSP_ERROR_NONE;

	/* Send RESET ENABLE command in QPI mode (QUAD I/Os, 4-4-4) */
	if (W25Q64JVS_ResetEnable(&(pqspi.hqspi), W25Q64JVS_QPI_MODE) != W25Q64JVS_OK)
	{
		ret = BSP_ERROR_COMPONENT_FAILURE;
	}/* Send RESET memory command in QPI mode (QUAD I/Os, 4-4-4) */
	else if(W25Q64JVS_ResetMemory(&(pqspi.hqspi), W25Q64JVS_QPI_MODE) != W25Q64JVS_OK)
	{
		ret = BSP_ERROR_COMPONENT_FAILURE;
	}/* Wait Flash ready */
	else if(W25Q64JVS_SoftPollingCheckLoMask(&(pqspi.hqspi), pqspi.InterfaceMode, W25Q64JVS_SR_BUSY, W25Q64JVS_BUSY_CHECK_TIME, W25Q64JVS_BUSY_TIMEOUT) != W25Q64JVS_OK)
	{
		ret = BSP_ERROR_COMPONENT_FAILURE;
	}/* Send RESET ENABLE command in SPI mode (1-1-1) */
	else if(W25Q64JVS_ResetEnable(&(pqspi.hqspi), BSP_QSPI_SPI_MODE) != W25Q64JVS_OK)
	{
		ret = BSP_ERROR_COMPONENT_FAILURE;
	}/* Send RESET memory command in SPI mode (1-1-1) */
	else if(W25Q64JVS_ResetMemory(&(pqspi.hqspi), BSP_QSPI_SPI_MODE) != W25Q64JVS_OK)
	{
		ret = BSP_ERROR_COMPONENT_FAILURE;
	}
	else
	{
		pqspi.IsInitialized = QSPI_ACCESS_INDIRECT; /* After reset S/W setting to indirect access   */
		pqspi.InterfaceMode = BSP_QSPI_SPI_MODE; /* After reset H/W back to SPI mode by default  */
	}

	/* Return BSP status */
	return ret;
}

/**
  * @brief  This function handles MDMA interrupt request.
  * @param  None
  * @retval None
  */

void MDMA_IRQHandler(void)
{
	/* Check the interrupt and clear flag */
	HAL_MDMA_IRQHandler(pqspi.hqspi.hmdma);
}

/**
  * @brief  This function handles QUADSPI interrupt request.
  * @param  None
  * @retval None
  */
void QUADSPI_IRQHandler(void)
{
	HAL_QSPI_IRQHandler(&(pqspi.hqspi));
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

