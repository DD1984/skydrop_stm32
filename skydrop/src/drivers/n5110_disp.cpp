#include "n5110_disp.h"
#include "uart.h"

#ifdef STM32
#include "stm32f1xx.h"

#define TFT_ILI9163C
void TFT_ILI9163C_chipInit(void);
void clearScreen(uint16_t color);

SPI_HandleTypeDef Spi;

void SpiInitMaster(void)
{
	Spi.Instance               = SPI1;
	Spi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	Spi.Init.Direction         = SPI_DIRECTION_2LINES;
	Spi.Init.CLKPhase          = SPI_PHASE_1EDGE;
	Spi.Init.CLKPolarity       = SPI_POLARITY_LOW;
	Spi.Init.DataSize          = SPI_DATASIZE_8BIT;
	Spi.Init.FirstBit          = SPI_FIRSTBIT_MSB;
	Spi.Init.TIMode            = SPI_TIMODE_DISABLE;
	Spi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
	Spi.Init.CRCPolynomial     = 7;
	Spi.Init.NSS               = SPI_NSS_SOFT;
	Spi.Init.Mode = SPI_MODE_MASTER;

	HAL_SPI_Init(&Spi);
	__HAL_SPI_ENABLE(&Spi);
}

void SpiSendRaw(uint8_t data)
{
	HAL_SPI_Transmit(&Spi, &data, 1, 1);
}

#define LCD_RST					GPIO_PIN_4
#define LCD_CE					GPIO_PIN_3
#define LCD_DC					GPIO_PIN_2

void LCD_InitPins(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitStruct.Pin       = LCD_RST | LCD_CE | LCD_DC;
	GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull      = GPIO_NOPULL;
	GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void LCD_PinSet(uint16_t pin, uint8_t level)
{
	HAL_GPIO_WritePin(GPIOA, pin, (GPIO_PinState)level);
}
#endif

void n5110display::SetDrawLayer(uint8_t layer)
{
	this->active_buffer = this->layers[layer];
}

/**
 * Set on screen position for next character
 *
 */
void n5110display::GotoXY(uint8_t x, uint8_t y)
{
	text_x = x;
	text_y = y;
}

/**
 * Write ASCII character on screen
 *
 */
void n5110display::Write(uint8_t ascii=0)
{
	if (ascii < font_begin || ascii > font_end)
	{
		text_x += font_spacing;
	}
	else
	{
		uint16_t adr = 6 + (ascii - font_begin) * 2;

#ifndef STM32
		uint16_t start = pgm_read_word(&this->font_data[adr]);
		uint16_t width = pgm_read_word(&this->font_data[adr + 2]) - start;
#else
		uint16_t start = *(uint16_t *)((uint8_t *)font_data + adr);
		uint16_t width = *(uint16_t *)((uint8_t *)font_data + adr + 2) - start;
#endif

		adr = this->font_adr_start + start * this->font_lines;

		for (uint8_t x = 0; x < width; x++)
		{
			uint16_t index = adr + x * font_lines;
			for (uint8_t n = 0; n < font_lines; n++)
			{
#ifndef STM32
				uint8_t data = pgm_read_byte(&this->font_data[index + n]);
#else
				uint8_t data = this->font_data[index + n];
#endif
				for (uint8_t a = 0; a < 8; a++)
				{
					if (data & (1 << a))
						this->PutPixel(text_x, text_y + a + n * 8, 1);
				}
			}

			text_x++;
			if (text_x >= n5110_width)
			{
				text_x = 0;
				text_y += this->font_height;
			}
		}
	}

	text_x += font_spacing;
	if (text_x >= n5110_width)
	{
		text_x = 0;
		text_y += this->font_height;
	}
}

uint8_t n5110display::GetTextWidth(char * text)
{
	uint8_t ret = 0;

	while (*text != 0)
	{
		if (*text < font_begin || *text > font_end)
		{
			ret += font_spacing;
		}
		else
		{
			uint16_t adr = 6 + (*text - font_begin) * 2;

#ifndef STM32
			uint16_t start = pgm_read_word(&this->font_data[adr]);
			uint8_t width = pgm_read_word(&this->font_data[adr + 2]) - start;
#else
			uint16_t start = this->font_data[adr];
			uint8_t width = this->font_data[adr + 2] - start;
#endif

			ret += font_spacing + width;
		}
		text++;
	}

	return ret;
}

uint8_t n5110display::GetTextHeight()
{
	return this->font_height;
}

uint8_t n5110display::GetAHeight()
{
	return this->font_A_height;
}


void n5110display::LoadFont(const uint8_t * font)
{
	this->font_data = font;
#ifndef STM32
	this->font_height = pgm_read_byte(&font[0]);
	this->font_A_height = pgm_read_byte(&font[1]);
	this->font_spacing = pgm_read_byte(&font[2]);
	this->font_lines = pgm_read_byte(&font[3]);
	this->font_begin = pgm_read_byte(&font[4]);
	this->font_end = pgm_read_byte(&font[5]);
#else
	this->font_height = font[0];
	this->font_A_height = font[1];
	this->font_spacing = font[2];
	this->font_lines = font[3];
	this->font_begin = font[4];
	this->font_end = font[5];
#endif

	this->font_adr_start = 6; //header
	this->font_adr_start += (this->font_end - this->font_begin + 2) * 2; //char adr table
}


void n5110display::sendcommand(unsigned char cmd)
{
#ifndef STM32
	GpioWrite(LCD_DC, LOW);

	this->spi->SetSlave(LCD_CE);
	this->spi->SendRaw(cmd);
	this->spi->UnsetSlave();
#else
	LCD_PinSet(LCD_DC, LOW);

	LCD_PinSet(LCD_CE, LOW);
	SpiSendRaw(cmd);
	LCD_PinSet(LCD_CE, HIGH);
#endif
}

/**
 * Set display to active mode
 *
 * \param i2c Pointer to i2c object
 */
void n5110display::Init()
{
#ifndef STM32
	LCD_SPI_PWR_ON;

	this->spi = new Spi;

	this->spi->InitMaster(LCD_SPI);
	this->spi->SetDivider(spi_div_64);
	this->spi->SetDataOrder(MSB);
#else
	SpiInitMaster();
	LCD_InitPins();
#endif

	CreateSinTable();

#ifndef STM32
	GpioSetDirection(LCD_RST, OUTPUT);
	GpioSetDirection(LCD_DC, OUTPUT);
	GpioSetDirection(LCD_CE, OUTPUT);
	GpioSetDirection(LCD_VCC, OUTPUT);

	GpioWrite(LCD_VCC, HIGH);

	GpioWrite(LCD_RST, LOW);
	_delay_ms(10);
	GpioWrite(LCD_RST, HIGH);
#else
	LCD_PinSet(LCD_RST, LOW);
	_delay_ms(10);
	LCD_PinSet(LCD_RST, HIGH);

#ifdef TFT_ILI9163C
	TFT_ILI9163C_chipInit();
	clearScreen(0);
#endif
#endif

#ifndef TFT_ILI9163C
	this->SetBias(0x13);

	this->SetContrast(72);

	sendcommand(0x20); //Basic
	this->SetInvert(false);
#endif

	for (uint8_t i = 0; i < DISP_LAYERS; i++)
	{
		SetDrawLayer(i);
		ClearBuffer();
	}

	SetDrawLayer(0);

	Write('A');
	Draw();
}

void n5110display::SetContrast(uint8_t val) //0-127
{
	sendcommand(0x21); //Extended
	sendcommand(0x80 | val);
	sendcommand(0x20); //Basic
}

void n5110display::SetInvert(uint8_t invert)
{
	 //Set display control, normal mode. 0x0D for inverse
	if (invert)
		sendcommand(0x0D);
	else
		sendcommand(0x0C);
}

void n5110display::SetFlip(bool flip)
{
	this->flip = flip;
}

void n5110display::SetBias(uint8_t bias) // 0x13 / 0x14
{
	sendcommand(0x21); //Extended
	sendcommand(0x04); //Set Temp coefficent
	sendcommand(bias); //LCD bias mode 1:48: Try 0x13 or 0x14
}

void n5110display::Stop()
{
#ifndef STM32
	this->spi->Stop();
	delete this->spi;

	GpioSetDirection(LCD_RST, INPUT);
	GpioSetDirection(LCD_DC, INPUT);
	GpioSetDirection(LCD_CE, INPUT);
	GpioSetDirection(LCD_VCC, INPUT);

	LCD_SPI_PWR_OFF;
#endif
}

/**
 * Draw line (works in any direction)
 *
 */
void n5110display::DrawLine(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t color=1){
	uint8_t deltax,deltay,x,y,xinc1,xinc2,yinc1,yinc2,den,num,numadd,numpixels,curpixel;
	deltax = abs(x2 - x1);		// The difference between the x's
	deltay = abs(y2 - y1);		// The difference between the y's
	x = x1;				   	// Start x off at the first pixel
	y = y1;				   	// Start y off at the first pixel

	if (x2 >= x1){			 	// The x-values are increasing
	  xinc1 = 1;
	  xinc2 = 1;
	}
	else{						  // The x-values are decreasing
	  xinc1 = -1;
	  xinc2 = -1;
	}

	if (y2 >= y1){			 	// The y-values are increasing
	  yinc1 = 1;
	  yinc2 = 1;
	}
	else{						  // The y-values are decreasing
	  yinc1 = -1;
	  yinc2 = -1;
	}

	if (deltax >= deltay){	 	// There is at least one x-value for every y-value
	  xinc1 = 0;				  // Don't change the x when numerator >= denominator
	  yinc2 = 0;				  // Don't change the y for every iteration
	  den = deltax;
	  num = deltax / 2;
	  numadd = deltay;
	  numpixels = deltax;	 	// There are more x-values than y-values
	}
	else{						  // There is at least one y-value for every x-value
	  xinc2 = 0;				  // Don't change the x for every iteration
	  yinc1 = 0;				  // Don't change the y when numerator >= denominator
	  den = deltay;
	  num = deltay / 2;
	  numadd = deltax;
	  numpixels = deltay;	 	// There are more y-values than x-values
	}

	for (curpixel = 0; curpixel <= numpixels; curpixel++){
	  PutPixel(x, y,color);		 	// Draw the current pixel
	  num += numadd;			  // Increase the numerator by the top of the fraction
	  if (num >= den){		 	// Check if numerator >= denominator
		num -= den;		   	// Calculate the new numerator value
		x += xinc1;		   	// Change the x as appropriate
		y += yinc1;		   	// Change the y as appropriate
	  }
	  x += xinc2;			 	// Change the x as appropriate
	  y += yinc2;			 	// Change the y as appropriate
	}
}

/**
 * Put pixel on screen
 *
 * /param val -
 */
void n5110display::PutPixel(uint8_t x ,uint8_t  y ,uint8_t color)
{
	if (x >= n5110_width || y >= n5110_height)
		return;

	uint16_t index = ((y / 8) * n5110_width) + (x % n5110_width);
	if (color == DISP_COLOR_WHITE)
		active_buffer[index] |= (1 << (y % 8));
	else {
		active_buffer[index] &= ~(1 << (y % 8));
	}
}

void n5110display::InvertPixel(uint8_t x ,uint8_t  y)
{
	if (x >= n5110_width || y >= n5110_height)
		return;

	uint16_t index = ((y / 8) * n5110_width) + (x % n5110_width);
	active_buffer[index] ^= (1 << (y % 8));
}

void n5110display::DrawImage(const uint8_t *data,uint8_t x,uint8_t y)
{
	uint8_t cbuf;
#ifndef STM32
	cbuf = pgm_read_byte(&data[0]);
#else
	cbuf = data[0];
#endif

	uint8_t imgwidth = (cbuf+x < n5110_width)?cbuf:n5110_width-x;
	int16_t xCutOff  = (cbuf+x < n5110_width)?0:(cbuf+x-n5110_width);
	uint8_t yOffset  = (y/8 < 1)?0:y/8;

#ifndef STM32
	cbuf = pgm_read_byte(&data[1]);
#else
	cbuf = data[1];
#endif

	uint8_t imgheight = (cbuf/8);
	uint8_t _x = x;
	uint8_t _y = 0;
	uint16_t index = 2;

	if (y >= n5110_height || x >= n5110_width) return;

	for (_y=0;_y < imgheight; _y++){
		for(_x=0;_x < imgwidth; _x++){

#ifndef STM32
			cbuf = pgm_read_byte(&data[index]);
#else
			cbuf = data[index];
#endif

			if (y % 8 != 0) {
				uint16_t tmp = 0;

				uint8_t  tmpdat = cbuf;
				tmp = (tmpdat << (y%8));

				active_buffer[((_y+yOffset)*n5110_width)+_x+x] |= (tmp & 255);
				if (_y+yOffset+1 < n5110_height)
				active_buffer[((_y+yOffset+1)*n5110_width)+_x+x] |= ((tmp >> 8) & 255);
			}
			else
				active_buffer[((_y+yOffset)*n5110_width)+_x+x] |= cbuf;
			index++;
		}
		index += xCutOff;
	}
}

void n5110display::DrawRectangle(int8_t x1,int8_t y1,int8_t x2,int8_t y2,uint8_t color=1,uint8_t fill=0){
	int8_t xdir;
	int8_t xref = (x1 <= x2) ? x2:x1;
	int8_t ydir = (y1 <= y2) ? y2:y1;
	int8_t yref = (y1 <= y2) ? y1:y2;

	for(; ydir >= yref; ydir--){
		PutPixel(x1,ydir,color);
		PutPixel(x2,ydir,color);
		if (fill==0 && (ydir != y1 && ydir != y2)) continue;
		for(xdir = (x1 <= x2) ? (x1+1):(x2+1); xdir <= (xref-1); xdir++){
			PutPixel(xdir,ydir,color);
		}
	}
}

void n5110display::Invert(int8_t x1,int8_t y1,int8_t x2,int8_t y2){
	int8_t xdir;
	int8_t xref = (x1 <= x2) ? x2:x1;
	int8_t ydir = (y1 <= y2) ? y2:y1;
	int8_t yref = (y1 <= y2) ? y1:y2;

	for(; ydir >= yref; ydir--){
		InvertPixel(x1,ydir);
		InvertPixel(x2,ydir);
		for(xdir = (x1 <= x2) ? (x1+1):(x2+1); xdir <= (xref-1); xdir++){
			InvertPixel(xdir,ydir);
		}
	}
}

void n5110display::InvertPart(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2)
{
	uint8_t y, x;

  for(y = y1;y <= y2; y++)
  {
	  for (x = x1; x < x2; x++)
	  {
		  active_buffer[n5110_width * y + x] ^= 0xFF;
	  }
  }
}

void n5110display::DrawArc(uint8_t cx,uint8_t cy,uint8_t radius,int16_t start,int16_t end)
{
	int16_t angle = 0;
	int8_t x,y;

	for (angle=start;angle<=end;angle++)
	{
		x = radius * get_sin(angle);
		y = radius * get_sin(angle+180);
		PutPixel(cx+x,cy + y,1);
	}
}

void n5110display::DrawTriangle(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t x3,uint8_t y3,uint8_t color){
	 DrawLine(x1, y1, x2,y2,color);
	 DrawLine(x1,y1,x3,y3,color);
	 DrawLine(x2,y2,x3,y3,color);
}

void n5110display::DrawCircle(uint8_t cx, uint8_t cy, uint8_t radius,uint8_t color=1)
{
	int8_t error = -radius;
	uint8_t x = radius;
	uint8_t y = 0;

  while (x >= y)
  {
    plot8points(cx, cy, x, y, color);

    error += y;
    ++y;
    error += y;

    if (error >= 0)
    {
      error -= x;
      --x;
      error -= x;
    }
  }
}

void n5110display::plot8points(uint8_t cx, uint8_t cy, uint8_t x, uint8_t y,uint8_t color)
{
  plot4points(cx, cy, x, y, color);
  if (x != y) plot4points(cx, cy, y, x, color);
}

void n5110display::plot4points(uint8_t cx, uint8_t cy, uint8_t x, uint8_t y,uint8_t color)
{
  PutPixel(cx + x, cy + y,color);
  if (x != 0) PutPixel(cx - x, cy + y,color);
  if (y != 0) PutPixel(cx + x, cy - y,color);
  if (x != 0 && y != 0) PutPixel(cx - x, cy - y,color);
}

void n5110display::SetRowCol(unsigned char row,unsigned char col)
{
#ifndef	TFT_ILI9163C
	this->sendcommand(0x80 | col);
	this->sendcommand(0x40 | row);
#else
#endif
}

#ifndef STM32
void n5110display::Draw()
{
	if (this->flip)
	{
		for (uint8_t j=0;j<6;j++)
		{
			SetRowCol(5 - j, 0);

#ifndef STM32
			GpioWrite(LCD_DC, HIGH);
			this->spi->SetSlave(LCD_CE);
#else
			//LCD_PinSet(LCD_DC, HIGH);
			//LCD_PinSet(LCD_CE, LOW);
#endif
			for (uint8_t a=0; a < n5110_width; a++)
			{
#ifndef STM32
				this->spi->SendRaw(fast_flip(active_buffer[n5110_width - 1 - a + (j * n5110_width)]));
#else
				//SpiSendRaw(fast_flip(active_buffer[n5110_width - 1 - a + (j * n5110_width)]));
#endif
			}
#ifndef STM32
			this->spi->UnsetSlave();
#else
			//LCD_PinSet(LCD_CE, HIGH);
#endif
		}
	}
	else
	{
		SetRowCol(0, 0);

		for (uint8_t j=0;j<6;j++)
		{
#ifndef STM32
			GpioWrite(LCD_DC, HIGH);
			this->spi->SetSlave(LCD_CE);
#else
			//LCD_PinSet(LCD_DC, HIGH);
			//LCD_PinSet(LCD_CE, LOW);
#endif
			for (uint8_t a=0; a < n5110_width; a++)
			{
#ifndef STM32
				this->spi->SendRaw(active_buffer[a+(j * n5110_width)]);
#else
				//SpiSendRaw(active_buffer[a+(j * n5110_width)]);
#endif
			}
#ifndef STM32
			this->spi->UnsetSlave();
#else
			//LCD_PinSet(LCD_CE, HIGH);
#endif
		}
	}
}
#else
void writedata16(uint16_t d);
void setAddr(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void n5110display::Draw()
{
	setAddr(0x00 + 20, 0x00 + 20, n5110_width + 20 - 1, n5110_height + 20);//go home

	for (uint8_t j = 0;j < 6;j++) {
		for (uint8_t b = 0; b < 8; b++)
			for (uint8_t a = 0; a < n5110_width; a++) {
				if ((active_buffer[a + (j * n5110_width)]) & (1<<b))
					writedata16(0xffff);
				else
					writedata16(0);
		}
	}
	printf("!\n");
}

#endif

void n5110display::CopyToLayerX(uint8_t dst, int8_t x)
{
	uint8_t start_x, end_x, col_x;

	if (x < 0)
	{
		start_x = abs(x);
		end_x = n5110_width;
		col_x = 0;
	}
	else
	{
		start_x = 0;
		end_x = n5110_width - x;
		col_x = x;
	}

	for (uint8_t j=0;j<6;j++)
	{
		uint16_t index = j * n5110_width + col_x;
		uint8_t cnt = 0;

		for (uint8_t a = start_x; a < end_x; a++)
		{
			this->layers[dst][index + cnt] = this->active_buffer[a + (j * n5110_width)];
			cnt++;
		}
	}
}

void n5110display::CopyToLayer(uint8_t dst)
{
	memcpy(this->layers[dst], this->active_buffer, (n5110_height / 8) * n5110_width);
}

void n5110display::CopyToLayerPart(uint8_t dst, uint8_t row1, uint8_t col1, uint8_t row2, uint8_t col2)
{
	for (uint8_t j=row1;j<row2;j++)
	{
		uint16_t start_i = j * n5110_width;

		for (uint8_t a = col1; a < col2; a++)
		{
			uint16_t index = start_i + a;
			this->layers[dst][index] = this->active_buffer[index];
		}
	}
}

void n5110display::ClearBuffer(void){
  unsigned char i,k;
  for(k=0;k<6;k++)
  {
	  for(i=0;i<n5110_width;i++)     //clear all COL
	  {
		active_buffer[i+(k*n5110_width)] = 0;
	  }
  }
}

void n5110display::ClearPart(uint8_t row1, uint8_t col1, uint8_t row2, uint8_t col2)
{
	for (uint8_t j = row1; j < row2; j++)
	{
		uint16_t index = j * n5110_width;
		uint8_t cnt = 0;

		for (uint8_t a = col1; a < col2; a++)
		{
			this->active_buffer[index + cnt] = 0;
			cnt++;
		}
	}
}

void  n5110display::CreateSinTable(){
	  for (int16_t i=0; i < 91; i++)
		  sin_table[i] = sin(((float)i/180.0)*3.142);
}

float n5110display::get_sin(uint16_t angle)
{
	angle = angle % 360;

	if (angle < 90)
		return this->sin_table[angle];
	else if (angle < 180)
		return this->sin_table[90 - (angle - 90)];
	else if (angle < 270)
		return -this->sin_table[angle - 180];
	else return -this->sin_table[90 - (angle - 270)];
}

#define _GRAMWIDTH 128
#define _GRAMHEIGH 160
#define _GRAMSIZE	_GRAMWIDTH * _GRAMHEIGH
#define __OFFSET		32//*see note 2

//ILI9163C registers-----------------------
#define CMD_NOP     	0x00//Non operation
#define CMD_SWRESET 	0x01//Soft Reset
#define CMD_SLPIN   	0x10//Sleep ON
#define CMD_SLPOUT  	0x11//Sleep OFF
#define CMD_PTLON   	0x12//Partial Mode ON
#define CMD_NORML   	0x13//Normal Display ON
#define CMD_DINVOF  	0x20//Display Inversion OFF
#define CMD_DINVON   	0x21//Display Inversion ON
#define CMD_GAMMASET 	0x26//Gamma Set (0x01[1],0x02[2],0x04[3],0x08[4])
#define CMD_DISPOFF 	0x28//Display OFF
#define CMD_DISPON  	0x29//Display ON
#define CMD_IDLEON  	0x39//Idle Mode ON
#define CMD_IDLEOF  	0x38//Idle Mode OFF
#define CMD_CLMADRS   	0x2A//Column Address Set
#define CMD_PGEADRS   	0x2B//Page Address Set

#define CMD_RAMWR   	0x2C//Memory Write
#define CMD_RAMRD   	0x2E//Memory Read
#define CMD_CLRSPACE   	0x2D//Color Space : 4K/65K/262K
#define CMD_PARTAREA	0x30//Partial Area
#define CMD_VSCLLDEF	0x33//Vertical Scroll Definition
#define CMD_TEFXLON		0x35//Tearing Effect Line ON
#define CMD_TEFXLOF		0x34//Tearing Effect Line OFF
#define CMD_MADCTL  	0x36//Memory Access Contro
#define CMD_VSSTADRS	0x37//Vertical Scrolling Start address
#define CMD_PIXFMT  	0x3A//Interface Pixel Format
#define CMD_FRMCTR1 	0xB1//Frame Rate Control (In normal mode/Full colors)
#define CMD_FRMCTR2 	0xB2//Frame Rate Control(In Idle mode/8-colors)
#define CMD_FRMCTR3 	0xB3//Frame Rate Control(In Partial mode/full colors)
#define CMD_DINVCTR		0xB4//Display Inversion Control
#define CMD_RGBBLK		0xB5//RGB Interface Blanking Porch setting
#define CMD_DFUNCTR 	0xB6//Display Fuction set 5
#define CMD_SDRVDIR 	0xB7//Source Driver Direction Control
#define CMD_GDRVDIR 	0xB8//Gate Driver Direction Control

#define CMD_PWCTR1  	0xC0//Power_Control1
#define CMD_PWCTR2  	0xC1//Power_Control2
#define CMD_PWCTR3  	0xC2//Power_Control3
#define CMD_PWCTR4  	0xC3//Power_Control4
#define CMD_PWCTR5  	0xC4//Power_Control5
#define CMD_VCOMCTR1  	0xC5//VCOM_Control 1
#define CMD_VCOMCTR2  	0xC6//VCOM_Control 2
#define CMD_VCOMOFFS  	0xC7//VCOM Offset Control
#define CMD_PGAMMAC		0xE0//Positive Gamma Correction Setting
#define CMD_NGAMMAC		0xE1//Negative Gamma Correction Setting
#define CMD_GAMRSEL		0xF2//GAM_R_SEL

void writecommand(uint8_t c)
{
	LCD_PinSet(LCD_DC, LOW);
	LCD_PinSet(LCD_CE, LOW);
	SpiSendRaw(c);
	LCD_PinSet(LCD_CE, HIGH);
}

void writedata(uint8_t c)
{
	LCD_PinSet(LCD_DC, HIGH);
	LCD_PinSet(LCD_CE, LOW);
	SpiSendRaw(c);
	LCD_PinSet(LCD_CE, HIGH);
}

void writedata16(uint16_t d)
{
	LCD_PinSet(LCD_DC, HIGH);
	LCD_PinSet(LCD_CE, LOW);
	SpiSendRaw(d >> 8);
	SpiSendRaw(d);
	LCD_PinSet(LCD_CE, HIGH);
}

void setRotation(uint8_t m)
{
	writecommand(CMD_MADCTL);
	writedata(0b00001000);
}

void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	writecommand(CMD_CLMADRS); // Column
	writedata16(x0);
	writedata16(x1);

	writecommand(CMD_PGEADRS); // Page

	writedata16(y0 + __OFFSET);
	writedata16(y1 + __OFFSET);
	writecommand(CMD_RAMWR); //Into RAM
}

void setAddr(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	setAddrWindow(x0,y0,x1,y1);
}

void clearScreen(uint16_t color)
{
	int px;

	//writecommand(CMD_RAMWR);
	setAddr(0x00,0x00,_GRAMWIDTH,_GRAMHEIGH);//go home
	for (px = 0; px < _GRAMSIZE; px++)
		writedata16(color);
}

void TFT_ILI9163C_chipInit(void)
{
	uint8_t i;
#if defined(__GAMMASET1)
	const uint8_t pGammaSet[15]= {0x36,0x29,0x12,0x22,0x1C,0x15,0x42,0xB7,0x2F,0x13,0x12,0x0A,0x11,0x0B,0x06};
	const uint8_t nGammaSet[15]= {0x09,0x16,0x2D,0x0D,0x13,0x15,0x40,0x48,0x53,0x0C,0x1D,0x25,0x2E,0x34,0x39};
#elif defined(__GAMMASET2)
	const uint8_t pGammaSet[15]= {0x3F,0x21,0x12,0x22,0x1C,0x15,0x42,0xB7,0x2F,0x13,0x02,0x0A,0x01,0x00,0x00};
	const uint8_t nGammaSet[15]= {0x09,0x18,0x2D,0x0D,0x13,0x15,0x40,0x48,0x53,0x0C,0x1D,0x25,0x2E,0x24,0x29};
#elif defined(__GAMMASET3)
	const uint8_t pGammaSet[15]= {0x3F,0x26,0x23,0x30,0x28,0x10,0x55,0xB7,0x40,0x19,0x10,0x1E,0x02,0x01,0x00};
	//&const uint8_t nGammaSet[15]= {0x00,0x19,0x1C,0x0F,0x14,0x0F,0x2A,0x48,0x3F,0x06,0x1D,0x21,0x3D,0x3F,0x3F};
	const uint8_t nGammaSet[15]= {0x09,0x18,0x2D,0x0D,0x13,0x15,0x40,0x48,0x53,0x0C,0x1D,0x25,0x2E,0x24,0x29};
#else
	const uint8_t pGammaSet[15]= {0x3F,0x25,0x1C,0x1E,0x20,0x12,0x2A,0x90,0x24,0x11,0x00,0x00,0x00,0x00,0x00};
	const uint8_t nGammaSet[15]= {0x20,0x20,0x20,0x20,0x05,0x15,0x00,0xA7,0x3D,0x18,0x25,0x2A,0x2B,0x2B,0x3A};
#endif

	writecommand(CMD_SWRESET);//software reset
	_delay_ms(500);
	writecommand(CMD_SLPOUT);//exit sleep
	_delay_ms(5);
	writecommand(CMD_PIXFMT);//Set Color Format 16bit
	writedata(0x05);
	_delay_ms(5);
	writecommand(CMD_GAMMASET);//default gamma curve 3
	writedata(0x04);//0x04
	_delay_ms(1);
	writecommand(CMD_GAMRSEL);//Enable Gamma adj
	writedata(0x01);
	_delay_ms(1);
	writecommand(CMD_NORML);

	writecommand(CMD_DFUNCTR);
	writedata(0b11111111);//
	writedata(0b00000110);//

	writecommand(CMD_PGAMMAC);//Positive Gamma Correction Setting
	for (i=0;i<15;i++){
		writedata(pGammaSet[i]);
	}
	writecommand(CMD_NGAMMAC);//Negative Gamma Correction Setting
	for (i=0;i<15;i++){
		writedata(nGammaSet[i]);
	}

	writecommand(CMD_FRMCTR1);//Frame Rate Control (In normal mode/Full colors)
	writedata(0x08);//0x0C//0x08
	writedata(0x02);//0x14//0x08
	_delay_ms(1);
	writecommand(CMD_DINVCTR);//display inversion
	writedata(0x07);
	_delay_ms(1);
	writecommand(CMD_PWCTR1);//Set VRH1[4:0] & VC[2:0] for VCI1 & GVDD
	writedata(0x0A);//4.30 - 0x0A
	writedata(0x02);//0x05
	_delay_ms(1);
	writecommand(CMD_PWCTR2);//Set BT[2:0] for AVDD & VCL & VGH & VGL
	writedata(0x02);
	_delay_ms(1);
	writecommand(CMD_VCOMCTR1);//Set VMH[6:0] & VML[6:0] for VOMH & VCOML
	writedata(0x50);//0x50
	writedata(99);//0x5b
	_delay_ms(1);
	writecommand(CMD_VCOMOFFS);
	writedata(0);//0x40
	_delay_ms(1);

	writecommand(CMD_CLMADRS);//Set Column Address
	writedata16(0x00);
	writedata16(_GRAMWIDTH);

	writecommand(CMD_PGEADRS);//Set Page Address
	writedata16(0X00);
	writedata16(_GRAMHEIGH);
	// set scroll area (thanks Masuda)
	writecommand(CMD_VSCLLDEF);
	writedata16(__OFFSET);
	writedata16(_GRAMHEIGH - __OFFSET);
	writedata16(0);
	setRotation(0);
	writecommand(CMD_DISPON);//display ON
	_delay_ms(1);
	writecommand(CMD_RAMWR);//Memory Write

	_delay_ms(1);

	//writecommand(CMD_DINVON);
}



