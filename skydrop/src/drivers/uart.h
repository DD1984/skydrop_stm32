/*
 * uart.h
 *
 *  Created on: 23.7.2014
 *      Author: horinek
 */

#ifndef UART_H_
#define UART_H_

#include "../common.h"
#ifndef STM32
#include <xlib/core/usart.h>
#else
#include <stdio.h>
#endif

//DEBUG
extern uint8_t debug_level;

#ifndef STM32
#define DEBUG(format, ...) \
	do \
	if (debug_level > 0) \
	{ \
		printf_P(PSTR(format), ##__VA_ARGS__); \
		uart.FlushTxBuffer(); \
	} \
	while(0)
#else
#define DEBUG(format, ...) {\
		printf("%s:%s[%d] - ", __FILE__, __func__,__LINE__);\
		printf(format, ##__VA_ARGS__);\
	}
#endif

//#define DEBUG1

#define DEBUG1(x...) \
	do \
	if (debug_level > 1) \
	{ \
		DEBUG("%S@%d: ", PSTR(__FILE__), __LINE__); \
		DEBUG(x); \
		DEBUG("\n"); \
		uart.FlushTxBuffer(); \
	} \
	while(0)


void uart_init_buffers();
void uart_init();
void uart_low_speed();
void uart_stop();


void DUMP_REG(uint8_t val);

#ifndef STM32
extern Usart uart;
#else
extern UART_HandleTypeDef Uart;
#ifdef __cplusplus
extern "C" {
#endif
int __io_putchar(int ch);
#ifdef __cplusplus
}
#endif
#endif

#endif /* UART_H_ */
