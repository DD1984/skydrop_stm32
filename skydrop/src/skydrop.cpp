#include "skydrop.h"

void Setup()
{
#ifdef UART_SUPPORT	
	debug_level = 2;
#endif	

#ifndef STM32
	//set clock to max for init 32MHz
	ClockSetSource(x32MHz);
	//disable 2MHZ osc
	OSC.CTRL = 0b00000010;
	//save power
	turnoff_subsystems();

	EnableInterrupts();
#else
	HAL_Init();
	SystemClock_Config();
#endif

	//init basic peripherals
#ifdef LED_SUPPORT
	led_init();
#endif	
#ifdef UART_SUPPORT	
	uart_init_buffers();
	uart_init();
#endif
#ifdef RTC_SUPPORT	
	time_init();
#endif
#ifdef AUDIO_SUPPORT
	buzzer_init();
#endif
	battery_init();
	buttons_init();

	//basic power control
	mems_power_init();
	io_init();
	
#ifndef STM32	
	SD_EN_INIT;
#endif

	//load configuration
	cfg_load();

	_delay_ms(100);
}

int free_ram_at_start;
uint8_t system_rst;

void Post()
{
	//buzzer_beep(_1sec, 0, 0, 0, 0);

	DEBUG("\n *** POST *** \n");

#ifndef STM32
	//Print reset reason
	DEBUG("Reset reason ... ");

	system_rst = RST.STATUS;

	if (RST.STATUS & 0b00100000)
		DEBUG("Software ");
	else
	if (RST.STATUS & 0b00010000)
		DEBUG("Programming ");
	else
	if (RST.STATUS & 0b00001000)
		DEBUG("Watchdog ");
	else
	if (RST.STATUS & 0b00000100)
		DEBUG("Brownout ");
	else
	if (RST.STATUS & 0b00000010)
		DEBUG("External ");
	else
	if (RST.STATUS & 0b00000001)
		DEBUG("Power On ");
	else
		DEBUG("Unknown: %02X", RST.STATUS);

	RST.STATUS = 0b00111111;
	DEBUG("\n");

#else
	DEBUG("RCC:\n")
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))
		DEBUG("\tPIN reset\n");
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
		DEBUG("\tSoftware Reset\n");
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
		DEBUG("\tIndependent Watchdog reset\n");
	 __HAL_RCC_CLEAR_RESET_FLAGS();

	DEBUG("PWR:\n")
	if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB)) {
		DEBUG("\tResumed from StandBy mode\n");
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
	}
	if (__HAL_PWR_GET_FLAG(PWR_FLAG_WU)) {
		DEBUG("\tWake Up event\n");
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
	}
	//
#endif	

	//App name
	print_fw_info();

#ifdef RTC_SUPPORT
	//Print actual time
	DEBUG("Time is ... \n");
	print_datetime();
#endif	

#ifndef STM32
	DEBUG("Free RAM at start ... %d\n", free_ram_at_start);
	test_memory();
#endif

	DEBUG("\n");
}

extern uint8_t actual_task;
extern uint8_t task_sleep_lock;

int main()
{
	free_ram_at_start = freeRam();
	Setup();

	Post();

	DEBUG(" *** STARTING TASKS ***\n");

	task_init();

	while (1)
	{
		//run main task
		task_loop();

		//run system tasks
		task_system_loop();

		//sleep now
		task_sleep();
	}
}
