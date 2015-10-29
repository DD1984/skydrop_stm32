#include "timers_irq_handlers.h"

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &audio_timer)
		audio_timer_ovf();

	if (htim == &fc_meas_timer)
		fc_meas_timer_ovf();
}
