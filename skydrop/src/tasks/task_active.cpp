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
	ewdt_reset();

	if (cfg_factory_passed())
	{
		gui_splash_set_mode(SPLASH_ON);
		gui_switch_task(GUI_SPLASH);
	}
	else
	{
		gui_switch_task(GUI_FTEST);
	}

	ewdt_reset();
	
#ifdef STORAGE_SUPPORT	
	if (storage_init())
	{
#ifndef STM32
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
#endif
	}
#endif

	//init flight computer
	ewdt_reset();
	fc_init();

#ifdef LED_SUPPORT
	led_notify_enable();
#endif	
}

void task_active_stop()
{
	StoreEEPROM();

#ifdef LED_SUPPORT
	led_notify_disable();
#endif

	fc_deinit();

	gui_stop();

	debug_end();

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
	debug_step();
}

void task_active_irqh(uint8_t type, uint8_t * buff)
{
	switch (type)
	{
#ifdef USB_SUPPORT		
	case(TASK_IRQ_USB):
		if (*buff && config.connectivity.usb_mode == USB_MODE_MASSSTORAGE)
			task_set(TASK_USB);
		break;
#endif		

	case(TASK_IRQ_BAT):
			fc_log_battery();
		break;

	default:
		gui_irqh(type, buff);
	}
}
