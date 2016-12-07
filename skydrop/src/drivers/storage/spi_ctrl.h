#ifndef __SPI_CTRL_H__
#define __SPI_CTRL_H__

#include <stdint.h>
#include "stm32f1xx_hal.h"

/* SPIx bus function */
HAL_StatusTypeDef  SPIx_Init(void);
uint8_t            SPIx_Write(uint8_t Value);
uint8_t            SPIx_Read(void);
void               SPIx_Error (void);
void               SPIx_MspInit(SPI_HandleTypeDef *hspi);

#endif
