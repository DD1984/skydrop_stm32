#ifndef DEBUG_COMMON_H_
#define DEBUG_COMMON_H_

#include "common.h"

#define DEBUG_FILE	"debug.log"

void debug_log(char * msg);

//DEBUG
#ifndef STM32
#define DEBUG(format, ...) \
	do { \
		char msg_buff[256];\
		const char * msg PROGMEM = PSTR(format);\
		sprintf_P(msg_buff, msg, ##__VA_ARGS__); \
		uart_send(msg_buff);\
		debug_log(msg_buff);\
		uart.FlushTxBuffer(); \
	} while(0)
#else

#define DEBUG(format, ...) \
	do { \
		char msg_buff[256];\
		const char * msg PROGMEM = PSTR(format);\
		sprintf_P(msg_buff, msg, ##__VA_ARGS__); \
		printf(msg_buff);\
		printf("\n");\
	} while(0)
#endif	



//assert
#define assert(cond) \
	do{ \
	if (!cond) \
		DEBUG("Assertion failed %S@%d!\n", PSTR(__FILE__), __LINE__); \
	} while(0); \

extern uint32_t debug_last_pc;
extern volatile uint16_t debug_min_stack_pointer;
extern volatile uint16_t debug_max_heap_pointer;

void debug_log(char * msg);
void debug_timer_init();
void debug_last_dump();

void ewdt_init();
void ewdt_reset();

#endif
