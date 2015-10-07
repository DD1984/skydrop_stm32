/*
 * psychosonda.h
 *
 *  Created on: 17.2.2014
 *      Author: horinek
 */

#ifndef SKYDROP_H_
#define SKYDROP_H_

#include "common.h"

#include "fc/conf.h"

#include "drivers/led.h"
#ifdef UART_SUPPORT
#include "drivers/uart.h"
#endif
#include "drivers/time.h"
#include "drivers/battery.h"
#include "drivers/buttons.h"

#include "drivers/audio/audio.h"
#include "drivers/audio/buzzer.h"

#ifdef STORAGE_SUPPORT
#include "drivers/storage/storage.h"
#endif

#ifdef BT_SUPPORT
#include "drivers/bluetooth/bt.h"
#endif

#include "drivers/sensors/devices.h"


#include "tasks/tasks.h"


void Setup();
void Post();

#endif /* PSYCHOSONDA_H_ */
