#ifndef DEBUG_COMMON_H_
#define DEBUG_COMMON_H_

#include "common.h"

#ifdef STM32
#include "debug_c.h"
#endif

#define DEBUG_FILE	"debug.log"

#define DEBUG_LOG_BUFFER_CHUNK	512
#define DEBUG_LOG_BUFFER_SIZE	2048

void debug_log(char * msg);

#ifndef STM32
//DEBUG
#define DEBUG(format, ...) \
	do { \
		if (debug_disabled())\
			break;\
		char msg_buff[256];\
		const char * msg PROGMEM = PSTR(format);\
		sprintf_P(msg_buff, msg, ##__VA_ARGS__); \
		debug_uart_send(msg_buff);\
		debug_log(msg_buff);\
	} while(0)
#endif

//assert
#define assert(cond) \
	do{ \
	if (!(cond)) \
		DEBUG("Assertion failed %s:%s[%d]!\n", PSTR(__FILE__), __func__, __LINE__); \
	} while(0); \

extern uint32_t debug_last_pc;
extern volatile uint16_t debug_min_stack_pointer;
extern volatile uint16_t debug_max_heap_pointer;

void debug_uart_send(char * msg);
void debug_log(char * msg);
void debug_timer_init();
void debug_last_dump();
void debug_step();
void debug_end();
bool debug_disabled();

void ewdt_init();
void ewdt_reset();

#endif
