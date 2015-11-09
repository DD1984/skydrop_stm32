#ifndef __ILI9163C_H__
#define __ILI9163C_H__

#include "common.h"

void TFT_ILI9163C_chipInit(void);
void clearScreen(uint16_t color);
void writedata16(uint16_t d);
void setAddr(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

#endif