#include "task_active.h"
#ifdef DISPLAY_SUPPORT
#include "../gui/gui.h"
#endif
#include "../fc/fc.h"
#ifdef DISPLAY_SUPPORT
#include "../gui/splash.h"
#endif

void task_active_init()
{
	DEBUG(" *** THIS IS TASK ACTIVE ***\n");

#ifdef DISPLAY_SUPPORT
	//init gui
	gui_init();
	gui_trigger_backlight();
#endif
#ifdef WDT_SUPPORT
	wdt_reset();
#endif

#ifdef DISPLAY_SUPPORT
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
#endif		

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
#ifdef DISPLAY_SUPPORT			
			gui_load_eeprom();
#endif			
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
#ifdef DISPLAY_SUPPORT
	gui_stop();
#endif	
#ifdef STORAGE_SUPPORT
	storage_deinit();
#endif	
}

void task_active_loop()
{
	fc_step();

#ifdef DISPLAY_SUPPORT
	gui_loop();
#endif	
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
#ifdef DISPLAY_SUPPORT
		gui_irqh(type, buff);
#else
		;
#endif		
	}
}
