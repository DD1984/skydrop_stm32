#include "spi_flash_io.h"
#include "spi_flash_cmd.h"
#include "spi_ctrl.h"

/******************************** LINK FLASH SPI ********************************/
/**
  * @brief  Initializes the FLASH SPI and put it into StandBy State (Ready for
  *         data transfer).
  * @retval None
  */
HAL_StatusTypeDef FLASH_SPI_IO_Init(void)
{
  HAL_StatusTypeDef Status = HAL_OK;

  GPIO_InitTypeDef  gpioinitstruct = {0};

  /* EEPROM_CS_GPIO Periph clock enable */
  FLASH_SPI_CS_GPIO_CLK_ENABLE();

  /* Configure EEPROM_CS_PIN pin: EEPROM SPI CS pin */
  gpioinitstruct.Pin    = FLASH_SPI_CS_PIN;
  gpioinitstruct.Mode   = GPIO_MODE_OUTPUT_PP;
  gpioinitstruct.Pull   = GPIO_PULLUP;
  gpioinitstruct.Speed  = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(FLASH_SPI_CS_GPIO_PORT, &gpioinitstruct);

  /* SPI FLASH Config */
  Status = SPIx_Init();

  /* EEPROM chip select high */
  FLASH_SPI_CS_HIGH();

  return Status;
}

/**
  * @brief  Write a byte on the FLASH SPI.
  * @param  Data: byte to send.
  * @retval None
  */
uint8_t FLASH_SPI_IO_WriteByte(uint8_t Data)
{
  /* Send the byte */
  return (SPIx_Write(Data));
}

/**
  * @brief  Read a byte from the FLASH SPI.
  * @retval uint8_t (The received byte).
  */
uint8_t FLASH_SPI_IO_ReadByte(void)
{
  uint8_t data = 0;

  /* Get the received data */
  data = SPIx_Read();

  /* Return the shifted data */
  return data;
}

/**
  * @brief  Read data from FLASH SPI driver
  * @param  MemAddress: Internal memory address
  * @param  pBuffer: Pointer to data buffer
  * @param  BufferSize: Amount of data to be read
  * @retval HAL_StatusTypeDef HAL Status
  */
HAL_StatusTypeDef FLASH_SPI_IO_ReadData(uint32_t MemAddress, uint8_t* pBuffer, uint32_t BufferSize)
{
  /*!< Select the FLASH: Chip Select low */
  FLASH_SPI_CS_LOW();

  /*!< Send "Read from Memory " instruction */
  SPIx_Write(FLASH_SPI_CMD_READ);

  /*!< Send ReadAddr high nibble address byte to read from */
  SPIx_Write((MemAddress & 0xFF0000) >> 16);
  /*!< Send ReadAddr medium nibble address byte to read from */
  SPIx_Write((MemAddress& 0xFF00) >> 8);
  /*!< Send ReadAddr low nibble address byte to read from */
  SPIx_Write(MemAddress & 0xFF);

  while (BufferSize--) /*!< while there is data to be read */
  {
    /*!< Read a byte from the FLASH */
    *pBuffer = SPIx_Write(FLASH_SPI_DUMMY_BYTE);
    /*!< Point to the next location where the byte read will be saved */
    pBuffer++;
  }

  /*!< Deselect the FLASH: Chip Select high */
  FLASH_SPI_CS_HIGH();

  return HAL_OK;
}

/**
  * @brief  Select the FLASH SPI and send "Write Enable" instruction
  * @retval None
  */
void FLASH_SPI_IO_WriteEnable(void)
{
  /*!< Select the FLASH: Chip Select low */
  FLASH_SPI_CS_LOW();

  /*!< Send "Write Enable" instruction */
  SPIx_Write(FLASH_SPI_CMD_WREN);

  /*!< Select the FLASH: Chip Select low */
  FLASH_SPI_CS_HIGH();

    /*!< Select the FLASH: Chip Select low */
  FLASH_SPI_CS_LOW();
}

/**
  * @brief  Wait response from the FLASH SPI and Deselect the device
  * @retval HAL_StatusTypeDef HAL Status
  */
HAL_StatusTypeDef FLASH_SPI_IO_WaitForWriteEnd(void)
{
  /*!< Select the FLASH: Chip Select low */
  FLASH_SPI_CS_HIGH();

  /*!< Select the FLASH: Chip Select low */
  FLASH_SPI_CS_LOW();

  uint8_t flashstatus = 0;

  /*!< Send "Read Status Register" instruction */
  SPIx_Write(FLASH_SPI_CMD_RDSR);

  /*!< Loop as long as the memory is busy with a write cycle */
  do
  {
    /*!< Send a dummy byte to generate the clock needed by the FLASH
    and put the value of the status register in FLASH_Status variable */
    flashstatus = SPIx_Write(FLASH_SPI_DUMMY_BYTE);

  }
  while ((flashstatus & FLASH_SPI_WIP_FLAG) == SET); /* Write in progress */

  /*!< Deselect the FLASH: Chip Select high */
  FLASH_SPI_CS_HIGH();

  return HAL_OK;
}

/**
  * @brief  Reads FLASH SPI identification.
  * @retval FLASH identification
  */
uint32_t FLASH_SPI_IO_ReadID(void)
{
  uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

  /*!< Select the FLASH: Chip Select low */
  FLASH_SPI_CS_LOW();

  /*!< Send "RDID " instruction */
  SPIx_Write(0x9F);

  /*!< Read a byte from the FLASH */
  Temp0 = SPIx_Write(FLASH_SPI_DUMMY_BYTE);

  /*!< Read a byte from the FLASH */
  Temp1 = SPIx_Write(FLASH_SPI_DUMMY_BYTE);

  /*!< Read a byte from the FLASH */
  Temp2 = SPIx_Write(FLASH_SPI_DUMMY_BYTE);

  /*!< Deselect the FLASH: Chip Select high */
  FLASH_SPI_CS_HIGH();

  Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;

  return Temp;
}

void FLASH_SPI_IO_DP(void)
{
  /*!< Select the FLASH: Chip Select low */
  FLASH_SPI_CS_LOW();

  SPIx_Write(FLASH_SPI_CMD_DP);

  /*!< Deselect the FLASH: Chip Select high */
  FLASH_SPI_CS_HIGH();
}

void FLASH_SPI_IO_RDP(void)
{
  /*!< Select the FLASH: Chip Select low */
  FLASH_SPI_CS_LOW();

  SPIx_Write(FLASH_SPI_CMD_RDP);

  /*!< Deselect the FLASH: Chip Select high */
  FLASH_SPI_CS_HIGH();

  HAL_Delay(1);
}
