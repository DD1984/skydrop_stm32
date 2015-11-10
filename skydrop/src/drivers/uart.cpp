#include "uart.h"

#ifndef STM32
Usart uart;
#else
UART_HandleTypeDef Uart;
#endif

#ifndef STM32
CreateStdIn(uart_in, uart.Read);
CreateStdOut(uart_out, uart.Write);
#endif

void uart_init_buffers()
{
#ifndef STM32
	uart.InitBuffers(0, BUFFER_SIZE);
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
	uart.Init(DEBUG_UART, 921600ul);
	uart.SetInterruptPriority(HIGH);
	uart.dbg = true;

	SetStdIO(uart_in, uart_out);
#else
	Uart.Instance        = USART1;

	Uart.Init.BaudRate   = 921600;
	Uart.Init.WordLength = UART_WORDLENGTH_8B;
	Uart.Init.StopBits   = UART_STOPBITS_1;
	Uart.Init.Parity     = UART_PARITY_NONE;
	Uart.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
	Uart.Init.Mode       = UART_MODE_TX_RX;

	HAL_UART_Init(&Uart);
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
