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

void uart_init();
void uart_low_speed();
void uart_stop();

void uart_send(char * msg);
void uart_send(uint16_t len, uint8_t * data);


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
