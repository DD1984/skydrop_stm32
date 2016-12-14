#include "uart.h"
#include "../fc/conf.h"

#define HW_UART_TX_SIZE	64
uint8_t hw_uart_tx_buffer[HW_UART_TX_SIZE];

#ifndef STM32
Usart uart(0, NULL, HW_UART_TX_SIZE, hw_uart_tx_buffer);

CreateStdIn(uart_in, uart.Read);
CreateStdOut(uart_out, uart.Write);
#else
UART_HandleTypeDef Uart;
#endif

void uart_send(uint16_t len, uint8_t * data)
{
#ifndef STM32
	for (uint16_t i = 0; i < len; i++)
		uart.Write(data[i]);
#endif
}

void uart_send(char * msg)
{
	char * ptr = msg;

	while (*ptr != 0)
	{
#ifndef STM32		
		uart.Write(*ptr);
#endif		
		ptr++;
	}
}

void uart_init()
{
#ifndef STM32
	//enable usart
	DEBUG_UART_PWR_ON;

	//init uart
	switch (config.connectivity.uart_function)
	{
		case(UART_FORWARD_DEBUG):
			uart.Init(DEBUG_UART, 921600ul);
		break;
		case(UART_FORWARD_OFF):
			DEBUG_UART_PWR_OFF;
			return;
		break;
		case(UART_FORWARD_9600):
			uart.Init(DEBUG_UART, 9600ul);
		break;
		case(UART_FORWARD_19200):
			uart.Init(DEBUG_UART, 19200ul);
		break;
		case(UART_FORWARD_38400):
			uart.Init(DEBUG_UART, 38400ul);
		break;
		case(UART_FORWARD_57600):
			uart.Init(DEBUG_UART, 57600ul);
		break;
		case(UART_FORWARD_115200):
			uart.Init(DEBUG_UART, 115200ul);
		break;
	}

	uart.SetInterruptPriority(HIGH);
//	uart.dbg = true;

	SetStdIO(uart_in, uart_out);
#else
	Uart.Instance        = USART3;

	Uart.Init.BaudRate   = 921600;
	Uart.Init.WordLength = UART_WORDLENGTH_8B;
	Uart.Init.StopBits   = UART_STOPBITS_1;
	Uart.Init.Parity     = UART_PARITY_NONE;
	Uart.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
	Uart.Init.Mode       = UART_MODE_TX_RX;

	HAL_UART_Init(&Uart);
	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
#endif
}

void uart_low_speed()
{
#ifndef STM32
	//enable usart
	DEBUG_UART_PWR_ON;

	//init uart
	uart.Init(DEBUG_UART, 9600);
	uart.SetInterruptPriority(HIGH);
//	uart.dbg = true;

	SetStdIO(uart_in, uart_out);
#endif
}

void uart_stop()
{
#ifndef STM32
	uart.Stop();

	//disable usart
	DEBUG_UART_PWR_OFF;
#endif
}

void DUMP_REG(uint8_t val)
{
	DEBUG("%02X - ", val);
	for (uint8_t q = 8; q > 0; q--)
	{
		DEBUG("%d", (val & (1 << (q - 1))) >> (q - 1));
		if (q == 5)
			DEBUG(" ");
	}
	DEBUG("\n");
}
