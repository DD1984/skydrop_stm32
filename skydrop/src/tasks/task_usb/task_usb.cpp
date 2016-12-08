#include "task_usb.h"
#include "../../gui/gui.h"

#ifndef STM32
SleepLock usb_lock;
extern Usart sd_spi_usart;
#else

//#include "stm32f1xx_hal.h"
#include "usbd_desc.h"
#include "usbd_core.h"
#include "usbd_msc.h"

#include "../../drivers/storage/usbd_storage.h"

USBD_HandleTypeDef USBD_Device;

void USB_Init(void)
{
	  /* Init MSC Application */
	  USBD_Init(&USBD_Device, &MSC_Desc, 0);

	  /* Add Supported Class */
	  USBD_RegisterClass(&USBD_Device, USBD_MSC_CLASS);

	  /* Add Storage callbacks for MSC Class */
	  USBD_MSC_RegisterStorage(&USBD_Device, &USBD_DISK_fops);

	  /* Start Device Process */
	  USBD_Start(&USBD_Device);
}
#endif

void task_usb_init()
{
#ifndef STM32
	SD_EN_OFF;
	_delay_ms(200);

	USB_PWR_ON;
	SD_SPI_PWR_ON;
	SD_EN_ON;


	DEBUG("This is USB task\n");

	usb_lock.Lock();

	XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC2MHZ);
	XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC2MHZ, DFLL_REF_INT_RC32KHZ, 2000000ul);
	XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000ul, F_USB);

	DEBUG("SD card init in RAW mode ... ");
	if (SDCardManager_Init())
		DEBUG("OK\n");
	else
		DEBUG("Error\n");
#endif

	DEBUG("USB init\n");

	USB_Init();

	//init gui
	gui_init();
	gui_switch_task(GUI_USB);
}


void task_usb_stop()
{
#ifndef STM32
	usb_lock.Unlock();

	led_set(0, 0, 0);

	gui_stop();

	sd_spi_usart.Stop();

	USB_PWR_OFF;
	SD_SPI_PWR_OFF;
	SD_EN_OFF;
#endif
}


void task_usb_loop()
{
	gui_loop();

	for (uint8_t i=0; i < 128; i++)
	{
#ifndef STM32
		MassStorage_Loop();
#endif
		ewdt_reset();
	}
}


void task_usb_irqh(uint8_t type, uint8_t * buff)
{
	switch (type)
	{
	case(TASK_IRQ_USB):
		if (*buff == 0)
			task_set(TASK_ACTIVE);
	break;

	default:
		gui_irqh(type, buff);
	}
}
