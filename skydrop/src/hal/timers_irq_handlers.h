#ifndef _TIMERS_IRQ_HANDLERS_H_
#define _TIMERS_IRQ_HANDLERS_H_

#include "stm32f1xx_hal.h"
#include "drivers/audio/vario.h"
#include "drivers/audio/audio.h"
#include "fc/fc.h"

extern TIM_HandleTypeDef audio_timer;
extern TIM_HandleTypeDef fc_meas_timer;

#endif