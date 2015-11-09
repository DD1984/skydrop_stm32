#include "task_active.h"
#include "../gui/gui.h"
#include "../fc/fc.h"
#include "../gui/splash.h"

void task_active_init()
{
	DEBUG(" *** THIS IS TASK ACTIVE ***\n");

	//init gui
	gui_init();
	gui_trigger_backlight();
#ifdef WDT_SUPPORT
	wdt_reset();
#endif

	if (cfg_factory_passed())
	{
		gui_splash_set_mode(SPLASH_ON);
		gui_switch_task(GUI_SPLASH);
		gui_force_loop();
	}
	else
	{
		gui_switch_task(GUI_FTEST);
	}

#ifdef WDT_SUPPORT
	wdt_reset();
#endif
	
#ifdef STORAGE_SUPPORT	
	if (storage_init())
	{
		//Handle update files

		FILINFO fno;

		//new way to update FW if SKYDROP.FW file found
		if (f_stat("SKYDROP.FW", &fno) == FR_OK)
		{
			task_set(TASK_UPDATE);
			return;
		}

		//Reset factory test if RST_FT file found
		if (f_stat("RST_FT", &fno) == FR_OK)
		{
			f_unlink("RST_FT");
			cfg_reset_factory_test();
		}

		//Load eeprom update
		if (LoadEEPROM())
		{
			cfg_load();
			gui_load_eeprom();
		}

		//preserve EE and FW file if NO_WIPE file found (factory programming)
		if (f_stat("NO_WIPE", &fno) != FR_OK)
		{
			//remove applied update files
			f_unlink("UPDATE.EE");
			f_unlink("UPDATE.FW");
		}
	}
#endif

	//init flight computer
#ifdef WDT_SUPPORT	
	wdt_reset();
#endif	
	fc_init();
}

void task_active_stop()
{
	StoreEEPROM();

	fc_deinit();
	gui_stop();
#ifdef STORAGE_SUPPORT
	storage_deinit();
#endif	
}

void task_active_loop()
{
	fc_step();

	gui_loop();
#ifdef STORAGE_SUPPORT
	storage_step();
#endif	
}

void task_active_irqh(uint8_t type, uint8_t * buff)
{
	switch (type)
	{
#ifdef USB_SUPPORT		
	case(TASK_IRQ_USB):
		if (*buff && config.system.usb_mode == USB_MODE_MASSSTORAGE)
			task_set(TASK_USB);
		break;
#endif		

	default:
		gui_irqh(type, buff);
	}
}
