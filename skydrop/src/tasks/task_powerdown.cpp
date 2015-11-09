#include "task_powerdown.h"

extern SleepLock powerdown_lock;
#ifndef STM32
extern Usart uart;
#endif

volatile bool powerdown_loop_break = false;

void task_powerdown_init()
{
#ifndef STM32
	//Lower F_CPU
	ClockSetSource(x2MHz);
	//disable other oscilators
	OSC.CTRL = 0b00000001;
#endif
#ifdef UART_SUPPORT
	uart_stop();
#endif	
	_delay_ms(10);

	turnoff_subsystems();

#ifdef UART_SUPPORT
	uart_low_speed();
#endif	
	_delay_ms(10);

	DEBUG(" *** POWER DOWN INIT ***\n");

	test_memory();

	//we do not want to enter sleep
	powerdown_loop_break = false;
	powerdown_lock.Lock();

	task_timer_setup(false);

#ifndef STM32
	SD_EN_OFF;

	DEBUG("Using low speed uart\n");
#endif
}


void task_powerdown_stop()
{
	//Reinit all devices
	DEBUG("Restarting all devices\n");

#ifndef STM32
#ifdef UART_SUPPORT
	uart_stop();
#endif	
	Setup();
	task_timer_setup();
	DEBUG("Restoring full speed uart\n");
//	Post();
#endif
	powerdown_lock.Unlock();
}


extern bool time_rtc_irq;

void powerdown_sleep()
{
	_delay_ms(31);
#ifndef STM32
	do
	{
		task_timer_stop();

		//allow rtc irq handler but do not wake up
		time_rtc_irq = false;

		//rtc irq set time_rtc_irq to true if executed
		SystemPowerSave();

#ifdef WDT_SUPPORT
		if (time_rtc_irq)
			wdt_reset();
#endif

		//start task timer in low speed mode
		task_timer_setup(false);
	} while (time_rtc_irq == true);
#else
	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
	DEBUG("STM32 - enter in standby mode\n");
	HAL_PWR_EnterSTANDBYMode();
#endif
}

extern uint8_t task_sleep_lock;

void task_powerdown_loop()
{
	//do not go to the Power down when there is another lock pending (button, buzzer)
	if ((task_sleep_lock == 1 && powerdown_lock.Active()) && powerdown_loop_break == false)
	{
		DEBUG("PD sleep\n");
		

#ifdef UART_SUPPORT
		uart_stop();
#endif

		powerdown_sleep();

#ifdef UART_SUPPORT
		uart_low_speed();
#endif
	}
}

void task_powerdown_irqh(uint8_t type, uint8_t * buff)
{
	DEBUG("POWERDOWN IRQH %d %d\n\n", type, *buff);

	switch(type)
	{
	case(TASK_IRQ_BUTTON_M):
		if (*buff == BE_LONG)
		{
			powerdown_loop_break = true;
			task_set(TASK_ACTIVE);
		}
	break;

#ifdef USB_SUPPORT
	case(TASK_IRQ_USB):
		uint8_t state = *buff;

		if (state == 1)
		{
			powerdown_loop_break = true;
			task_set(TASK_USB);
		}
	break;
#endif
	}
}
