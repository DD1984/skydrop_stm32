/*
 * gui.h
 *
 *  Created on: 22.1.2015
 *      Author: horinek
 */

#ifndef GUI_H_
#define GUI_H_

#include "../common.h"
#include "../drivers/n5110_disp.h"
#include "../drivers/led.h"

extern n5110display disp;
extern FILE * lcd_out;

#define GUI_DISP_WIDTH	n5110_width
#define GUI_DISP_HEIGHT	n5110_height

#define GUI_BLINK_TGL(PER) (task_get_ms_tick() % PER > (PER / 2))

#define GUI_CONTRAST_STEPS 	10

void gui_switch_task(uint8_t new_task);

void gui_init();
void gui_load_eeprom();

void gui_stop();
void gui_loop();
void gui_irqh(uint8_t type, uint8_t * buff);

void gui_force_loop();

void gui_trigger_backlight();
void gui_change_disp_cfg();
void gui_update_disp_cfg();
void gui_showmessage_P(const char * msg);
void gui_showmessage(char * msg);

void gui_dialog_P(const char * title);
void gui_dialog(char * title);

#define GUI_DIALOG_TOP 		13
#define GUI_DIALOG_LEFT 	2
#define GUI_DIALOG_RIGHT 	(GUI_DISP_WIDTH - 3)
#define GUI_DIALOG_BOTTOM 	(GUI_DISP_HEIGHT - 3)
#define GUI_DIALOG_WIDTH	(GUI_DIALOG_RIGHT - GUI_DIALOG_LEFT)
#define GUI_DIALOG_HEIGHT	(GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP)

void gui_raligh_text_P(const char * text, uint8_t x, uint8_t y);
void gui_raligh_text(char * text, uint8_t x, uint8_t y);

void gui_caligh_text_P(const char * text, uint8_t x, uint8_t y);
void gui_caligh_text(char * text, uint8_t x, uint8_t y);

void gui_statusbar();

#if 0
#define GUI_NONE			0xFF
#define GUI_PAGES			0
#define GUI_SETTINGS		1
#define GUI_SPLASH			2
#define GUI_SET_VARIO		3
#define GUI_SET_VAL			4
#define GUI_SET_AUDIO		5
#define GUI_SET_WIDGETS		6
#define GUI_LAYOUTS			7
#define GUI_SET_LAYOUT		8
#define GUI_SET_DISPLAY		9
#define GUI_USB				10
#define GUI_FTEST			11
#define GUI_SET_SYSTEM		12
#define GUI_SET_AUTOSTART	13
#define GUI_SET_GPS			14
#define GUI_SET_GPS_DETAIL	15
#define GUI_SET_DEBUG		16
#define GUI_SET_ALTIMETERS	17
#define GUI_SET_ALTIMETER	18
#define GUI_SET_TIME		19
#define GUI_SET_LOGGER		20
#define GUI_DIALOG			21
#define GUI_SET_BLUETOOTH	22
#define GUI_UPDATE			23
#define GUI_SET_WEEKLIFT	24
#define GUI_SET_AUDIO_MENU	25
#endif

enum {
	GUI_PAGES = 0,
	GUI_SETTINGS,
	GUI_SPLASH,
	GUI_SET_VARIO,
	GUI_SET_VAL,
#ifdef AUDIO_SUPPORT
	GUI_SET_AUDIO,
#endif
	GUI_SET_WIDGETS,
	GUI_LAYOUTS,
	GUI_SET_LAYOUT,
	GUI_SET_DISPLAY,
#ifdef USB_SUPPORT
	GUI_USB,
#endif
	GUI_FTEST,
	GUI_SET_SYSTEM,
	GUI_SET_AUTOSTART,
#ifdef GPS_SUPPORT
	GUI_SET_GPS,
	GUI_SET_GPS_DETAIL,
#endif
	GUI_SET_DEBUG,
	GUI_SET_ALTIMETERS,
	GUI_SET_ALTIMETER,
#ifdef RTC_SUPPORT
	GUI_SET_TIME,
#endif
	GUI_SET_LOGGER,
	GUI_DIALOG,
#ifdef BT_SUPPORT
	GUI_SET_BLUETOOTH,
#endif
#ifdef UPDATE_SUPPORT
	GUI_UPDATE,
#endif
	GUI_SET_WEEKLIFT,
	GUI_SET_AUDIO_MENU,
	//***
	GUI_END,
	//***
	GUI_NONE = 0xFF
};

#define NUMBER_OF_GUI_TASKS	GUI_END

#define GUI_LAST_TASK		GUI_NONE

extern uint8_t lcd_contrast_min;
extern uint8_t lcd_contrast_max;

extern volatile uint8_t gui_task;
extern volatile bool gui_buttons_override;

#endif /* GUI_H_ */
