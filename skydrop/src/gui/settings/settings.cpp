#include "settings.h"
#include "../gui_list.h"

enum {
	SETTINGS_VARIO,
	SETTINGS_ALTIMETERS,
	SETTINGS_LOGGER,
#ifdef GPS_SUPPORT
	SETTINGS_GPS,
#endif
#ifdef BT_SUPPORT
	SETTINGS_BT,
#endif
	SETTINGS_SETTINGS,
	SETTINGS_DEBUG,
	//
	SETTINGS_END
};

void gui_settings_init()
{
	gui_list_set(gui_settings_item, gui_settings_action, SETTINGS_END, GUI_PAGES);
}

void gui_settings_stop()
{
}

void gui_settings_loop()
{
	gui_list_draw();
}

void gui_settings_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_settings_action(uint8_t index)
{
	switch(index)
	{
	case(SETTINGS_VARIO):
		gui_switch_task(GUI_SET_VARIO);
	break;

	case(SETTINGS_ALTIMETERS):
		gui_switch_task(GUI_SET_ALTIMETERS);
	break;

	case(SETTINGS_LOGGER):
		gui_switch_task(GUI_SET_LOGGER);
	break;

#ifdef GPS_SUPPORT
	case(SETTINGS_GPS):
		gui_switch_task(GUI_SET_GPS);
	break;
#endif

#ifdef BT_SUPPORT
	case(SETTINGS_BT):
		gui_switch_task(GUI_SET_BLUETOOTH);
	break;
#endif

	case(SETTINGS_SETTINGS):
		gui_switch_task(GUI_SET_SYSTEM);
	break;

	case(SETTINGS_DEBUG):
		gui_switch_task(GUI_SET_DEBUG);
	break;

	}
}

void gui_settings_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (SETTINGS_VARIO):
			sprintf_P(text, PSTR("Vario"));
			*flags |= GUI_LIST_FOLDER;
		break;
		case (SETTINGS_ALTIMETERS):
			sprintf_P(text, PSTR("Altimeters"));
			*flags |= GUI_LIST_FOLDER;
		break;
		case (SETTINGS_LOGGER):
			sprintf_P(text, PSTR("Logger"));
			*flags |= GUI_LIST_FOLDER;
		break;
#ifdef GPS_SUPPORT
		case (SETTINGS_GPS):
			sprintf_P(text, PSTR("GPS"));
			*flags |= GUI_LIST_FOLDER;
		break;
#endif
#ifdef BT_SUPPORT
		case (SETTINGS_BT):
			sprintf_P(text, PSTR("Bluetooth"));
			*flags |= GUI_LIST_FOLDER;
		break;
#endif
		case (SETTINGS_SETTINGS):
			sprintf_P(text, PSTR("Settings"));
			*flags |= GUI_LIST_FOLDER;
		break;
		case (SETTINGS_DEBUG):
			sprintf_P(text, PSTR("Debug"));
			*flags |= GUI_LIST_FOLDER;
		break;


	}
}

