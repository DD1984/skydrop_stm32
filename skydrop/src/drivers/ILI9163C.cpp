#include "ILI9163C.h"

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


extern void LCD_PinSet(uint16_t pin, uint8_t level);
extern void SpiSendRaw(uint8_t data);

#ifdef LCD_RST
#undef LCD_RST
#endif

#ifdef LCD_CE
#undef LCD_CE
#endif

#ifdef LCD_DC
#undef LCD_DC
#endif

#define LCD_RST					GPIO_PIN_4
#define LCD_CE					GPIO_PIN_3
#define LCD_DC					GPIO_PIN_2


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
