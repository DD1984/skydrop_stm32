/*
 * vario.h
 *
 *  Created on: 3.9.2015
 *      Author: horinek
 */

#ifndef AUDIO_VARIO_H_
#define AUDIO_VARIO_H_

#include "../../common.h"

void audio_vario_step(float vario);
void audio_vario_reset();

#ifdef STM32
void audio_timer_ovf(void);
#endif

#endif /* VARIO_H_ */
