#ifndef __EEPROM_H__
#define __EEPROM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

#define EEMEM __attribute__((__section__(".eeprom")))

#define SHOW_SRC

#define eeprom_busy_wait() {}

#define eeprom_read_byte(addr) *(uint8_t *)addr
#define eeprom_read_block(dst, src, size) memcpy(dst, src, size)


void update_eeprom(void *addr, void *src, uint16_t size, char *src_func, uint32_t src_line);

#define _update_eeprom(addr, data, size) update_eeprom((void *)addr, (void *)data, size, (char *)__func__, __LINE__)

#define eeprom_update_byte(addr, val) {uint8_t tmp = val; _update_eeprom(addr, &tmp, sizeof(uint8_t));}
#define eeprom_update_word(addr, val) _update_eeprom(addr, &val, sizeof(uint16_t))
#define eeprom_update_float(addr, val) _update_eeprom(addr, &val, sizeof(float))
#define eeprom_update_block(data, addr, size) _update_eeprom(addr, data, size)

#define eeprom_write_float eeprom_update_float

#ifdef __cplusplus
}
#endif

#endif
