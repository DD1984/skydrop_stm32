#include "tasks.h"
#ifndef STM32
#include "../xlib/core/watchdog.h"
#endif
#include "../fc/conf.h"


void (* task_init_array[])() = {
	task_powerdown_init,
#ifdef USB_SUPPORT	
	task_usb_init,
#endif	
	task_active_init,
#ifdef UPDATE_SUPPORT
	task_update_init
#endif	
};


void (* task_stop_array[])() = {
	task_powerdown_stop,
#ifdef USB_SUPPORT	
	task_usb_stop,
#endif	
	task_active_stop,
#ifdef UPDATE_SUPPORT	
	task_update_stop
#endif	
};

void (* task_loop_array[])() = {
	task_powerdown_loop,
#ifdef USB_SUPPORT	
	task_usb_loop,
#endif	
	task_active_loop,
#ifdef UPDATE_SUPPORT	
	task_update_loop
#endif	
};

void (* task_irqh_array[])(uint8_t type, uint8_t * buff) = {
	task_powerdown_irqh,
#ifdef USB_SUPPORT	
	task_usb_irqh,
#endif	
	task_active_irqh,
#ifdef UPDATE_SUPPORT	
	task_update_irqh
#endif	
};

//task variables
#ifndef STM32
Timer task_timer;
#endif
volatile uint32_t task_timer_high;
volatile uint8_t task_sleep_lock = 0;

volatile uint32_t fine_timer_high;

volatile uint8_t actual_task = NO_TASK;
volatile uint8_t new_task = TASK_POWERDOWN;

#ifdef USB_SUPPORT
uint8_t usb_state;
#endif

SleepLock powerdown_lock;

SleepLock::SleepLock()
{
	this->active = false;
}

void SleepLock::Lock()
{
	if (this->active == false)
	{
		this->active = true;
		task_sleep_lock++;
	}
}

void SleepLock::Unlock()
{
	if (this->active == true)
	{
		this->active = false;
		task_sleep_lock--;
	}
}

bool SleepLock::Active()
{
	return this->active;
}

#ifndef STM32
ISR(TASK_TIMER_CMPA)
{
	if (SP < debug_min_stack_pointer)
		debug_min_stack_pointer = SP;

	if (debug_max_heap_pointer < freeRam())
		debug_max_heap_pointer = freeRam();

	task_timer_high += 512ul;
	fine_timer_high += 64000ul;

	uint16_t val = task_timer.GetValue();
	task_timer.SetValue(val - 63999);
}
#endif

#ifdef USB_SUPPORT
#ifndef STM32
ISR(USB_CONNECTED_IRQ)
{
	//dummy ISR
	//just wake up the device
	//usb_in is checked in main loop
}
#endif
#endif

uint32_t old_tick = 0;

uint32_t task_get_ms_tick_once()
{
#ifndef STM32
	uint32_t res;

	cli();
	res = (task_timer_high) + (uint32_t)(task_timer.GetValue() / 125);
	sei();

	if (res < old_tick)
	{
		assert(0);
		DEBUG(" res=%lu\n", res);
		DEBUG(" old=%lu\n", old_tick);
	}

	old_tick = res;

	return res;
#else
	return 0;
#endif
}

uint32_t task_get_ms_tick()
{
#ifndef STM32
	return old_tick;
#else
	return HAL_GetTick();
#endif
}


uint32_t fine_timer_get()
{
#ifndef STM32
	uint32_t res;

	cli();
	res = (fine_timer_high) + (uint32_t)(task_timer.GetValue());
	sei();

	return res;
#else
	return 0;
#endif
}

void task_timer_setup(bool full_speed)
{
#ifndef STM32	
	TASK_TIMER_PWR_ON;

	if (full_speed)
		task_timer.Init(TASK_TIMER, timer_div256);
	else
		task_timer.Init(TASK_TIMER, timer_div4);

	task_timer.Stop();
	task_timer.SetInterruptPriority(HIGH);
	task_timer.EnableInterrupts(timer_compareA);
	task_timer.SetValue(0);
	task_timer.SetCompare(timer_A, 63999); //125 == 1ms
	task_timer_high = 0;
	old_tick = 0;

	task_timer.Start();
#endif	
}

void task_timer_stop()
{
#ifndef	STM32
	task_timer.Stop();

	TASK_TIMER_PWR_OFF;
#endif	
}

void task_init()
{
	task_timer_setup();
#ifdef USB_SUPPORT
#ifndef STM32
	USB_CONNECTED_IRQ_ON;
#endif	
#endif

	powerdown_lock.Unlock();

	//if ftest is not done
	if (!cfg_factory_passed())
		task_set(TASK_ACTIVE);

#ifdef USB_SUPPORT
	//if is USB connected go directly to USB task
	if ((usb_state = USB_CONNECTED))
		task_set(TASK_USB);
#endif		

	ewdt_init();
}

void task_set(uint8_t task)
{
	new_task = task;
}

void task_loop()
{
	ewdt_reset();
	if (actual_task != NO_TASK)
		task_loop_array[actual_task]();
}

uint64_t loop_start = 0;

void task_system_loop()
{
	ewdt_reset();

	//task switching outside interrupt
	if (new_task != actual_task)
	{
		DEBUG("Switching task %d to %d\n", actual_task, new_task);
		if (actual_task != NO_TASK)
		{
			task_stop_array[actual_task]();

			//XXX: this will guarantee that task switched from the powerdown task will be vanilla
			if (new_task == TASK_POWERDOWN)
			{
#ifdef STM32
				HAL_RTCEx_BKUPWrite(&RtcHandle, 1, PD_MAGIC);
#endif
				SystemReset();
			}

#ifdef USB_SUPPORT
			//XXX: usb is bit unstable when it is switched from another task, this is hack
			if (new_task == TASK_USB && actual_task != NO_TASK)
			{
				SystemReset();
			}
#endif
		}
#ifdef STM32
		else {
			if (new_task == TASK_POWERDOWN) {
				if (HAL_RTCEx_BKUPRead(&RtcHandle, 1) != PD_MAGIC)
					new_task = TASK_ACTIVE;
			}
		}
#endif

		actual_task = new_task;

		task_init_array[actual_task]();
	}

#ifdef USB_SUPPORT
	//check USB and send IRQ
	if (usb_state != USB_CONNECTED)
	{
		usb_state = USB_CONNECTED;

		battery_force_update();
		task_irqh(TASK_IRQ_USB, &usb_state);
	}
#endif	

	buttons_step();
	if (powerdown_lock.Active() == false)
	{
		if (battery_step())
			task_irqh(TASK_IRQ_BAT, NULL);
	}
}

uint16_t wake_ups = 0;
uint32_t wake_next = 0;

uint32_t fine_last = 0;
uint32_t fine_acc = 0;

void task_sleep()
{
	fine_acc += fine_timer_get() - fine_last;

	if (task_sleep_lock == 0)
	{
		SystemPowerIdle();
	}

	task_get_ms_tick_once();
	fine_last = fine_timer_get();
	wake_ups++;

	if (wake_next < task_get_ms_tick())
	{
		uint8_t usage = fine_acc / 6250;
//		DEBUG("CPU: %3u%% (%u irq)\n", usage, wake_ups / 5);

		wake_ups = 0;
		fine_acc = 0;

		wake_next = task_get_ms_tick() + 5000;
	}
}

void task_irqh(uint8_t type, uint8_t * buff)
{
	if (actual_task != new_task)
	{
		DEBUG("IGNORING IRQ\n");
	}
	else
	{
		task_irqh_array[actual_task](type, buff);
	}
}

