#include "audio.h"
#include "../../fc/fc.h"

#include "buzzer.h"
#include "sequencer.h"
#include "vario.h"

#ifndef STM32
Timer audio_timer;
#else
#include "audio_c.h"
TIM_HandleTypeDef audio_timer;
#endif

//demo
volatile bool audio_demo = false;
volatile float audio_demo_val = 0;

void audio_init()
{
#ifndef STM32
	AUDIO_TIMER_PWR_ON;
	audio_timer.Init(AUDIO_TIMER, timer_div1024);
	audio_timer.EnableInterrupts(timer_overflow);
#else
#define TIM3CLK 31250
	audio_timer.Instance = TIM3;
	audio_timer.Init.Prescaler = (uint32_t)(SystemCoreClock / TIM3CLK) - 1;;
	audio_timer.Init.ClockDivision = 0;
	audio_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
	audio_timer.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&audio_timer);
#endif
}

void audio_step()
{
	//sound effect is high priority
	if (seq_enabled)
	{
		seq_loop();
		return;
	}

	//vario demo sound suppress standard vario sound
	if (audio_demo)
	{
		audio_vario_step(audio_demo_val);
		return;
	}

	//barometer data are valid now
	if (fc.baro_valid)
	{
		//audio is suppressed due auto start
		if (config.autostart.flags & AUTOSTART_SUPRESS_AUDIO)
		{
			//vario in flight -> enable sound
			if (fc.flight_state == FLIGHT_FLIGHT)
				audio_vario_step(fc.vario);

			return;
		}

		//audio suppress is not used
		audio_vario_step(fc.vario);
	}
}

void audio_off()
{
#ifndef STM32
	//stop unused timer
	audio_timer.Stop();
#else
	if (audio_timer.Instance != NULL)
		HAL_TIM_Base_Stop_IT(&audio_timer);
#endif
	//reset state of audio vario
	audio_vario_reset();
	//silence!
	buzzer_set_vol(0);
}


#ifdef STM32
#include "audio_c.h"
void audio_step_c(void) {
	audio_step();
}
#endif
