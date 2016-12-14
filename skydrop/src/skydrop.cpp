#include "skydrop.h"

int free_ram_at_start;
uint8_t system_rst;

void Setup()
{
#ifndef STM32
	//set clock to max for init 32MHz
	ClockSetSource(x32MHz);
	//disable 2MHZ osc
	OSC.CTRL = 0b00000010;

	//get RAM info
	free_ram_at_start = freeRam();

	//get reset reason
	system_rst = RST.STATUS;
	RST.STATUS = 0b00111111;

	//save power - peripherals are turned on on demand by drivers

	turnoff_subsystems();

	EnableInterrupts();
#else
	HAL_Init();
	SystemClock_Config();
#endif

	//load device id
	GetID();

	//init basic peripherals
#ifdef LED_SUPPORT
	led_init();
#endif	
	time_init();
	buzzer_init();
	buttons_init();

	//basic power control
	mems_power_init();
	io_init();
	
#ifndef STM32	
	SD_EN_INIT;
#endif

	//load configuration from EE
	cfg_load();
	uart_init();
	battery_init();


	_delay_ms(100);
}

void Post()
{
	DEBUG("\n *** POST *** \n");

	//Reset reason
#ifndef STM32
	print_reset_reason();

#else
	DEBUG("RCC:\n");
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))
		DEBUG("\tPIN reset\n");
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
		DEBUG("\tSoftware Reset\n");
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
		DEBUG("\tIndependent Watchdog reset\n");
	 __HAL_RCC_CLEAR_RESET_FLAGS();

	DEBUG("PWR:\n");
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

	//Print actual time
	DEBUG("Time is ... \n");
	print_datetime(time_get_local());

#ifndef STM32
	DEBUG("Free RAM at start ... %d\n", free_ram_at_start);
	test_memory();
#endif

	char id[23];
	GetID_str(id);
	DEBUG("Device serial number ... %s\n", id);

	DEBUG("Board rev ... %u\n", (hw_revision == HW_REW_1504) ? 1504 : 1406);

	//debug info
	debug_last_dump();
}

int main()
{
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
