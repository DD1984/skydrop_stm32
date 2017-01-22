#ifndef __DEBUG_C_H__
#define __DEBUG_C_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

extern void debug_log_c(char * msg);

//DEBUG
#define DEBUG(format, ...) \
	do { \
		char msg_buff[256];\
		sprintf(msg_buff, format, ##__VA_ARGS__); \
		printf("%s", msg_buff);\
		debug_log_c(msg_buff);\
	} while(0)

#ifdef __cplusplus
}
#endif

#endif
