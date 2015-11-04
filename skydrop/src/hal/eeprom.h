#ifndef __EEPROM_H__
#define __EEPROM_H__

#include "stm32f1xx_hal.h"

#define EEMEM __attribute__((__section__(".eeprom")))

#define eeprom_busy_wait() {}

#define eeprom_read_byte(addr) *(uint8_t *)addr
#define eeprom_read_block(dst, src, size) memcpy(dst, src, size)

#define eeprom_update_block(addr, data, size) {}

#define eeprom_write_float eeprom_update_float

void eeprom_update_byte(void *addr, uint8_t val);
void eeprom_update_word(void *addr, uint16_t val);
void eeprom_update_float(void *addr, float val);

#endif
