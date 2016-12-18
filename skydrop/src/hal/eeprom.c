#include <stdlib.h>
#include "eeprom.h"

#define DEBUG printf

static FLASH_EraseInitTypeDef EraseInitStruct;
uint32_t PAGEError = 0;
extern uint32_t _eeprom_start;
extern uint32_t _eeprom_end;

void update_eeprom(void *addr, void *src, uint16_t size, char *src_func, uint32_t src_line)
{
	if ((uint32_t)addr < (uint32_t)&_eeprom_start || (uint32_t)addr > (uint32_t)&_eeprom_end) {
		DEBUG("eeprom: === WARNING ===\n");
		DEBUG("eeprom: start: 0x%08x, end: 0x%08x\n", (uint32_t)&_eeprom_start, (uint32_t)&_eeprom_end);
		DEBUG("eeprom: to_flash: 0x%08x from_ram: 0x%08x size: %d\n", (uint32_t)addr, (uint32_t)src, size);
		DEBUG("eeprom: source code - %s[%d]\n", src_func, src_line);
		DEBUG("eeprom: unknown section for update!!!\n");
		DEBUG("eeprom: ===============\n");
		return;
	}

	char *buf = (char *)malloc(FLASH_PAGE_SIZE);
	if (buf == NULL) {
		DEBUG("eeprom: can not allocate  memory for buf\n");
		return;
	}

	memcpy((void *)buf, (void *)&_eeprom_start, FLASH_PAGE_SIZE);

	uint32_t val_offset = (uint32_t)addr - (uint32_t)&_eeprom_start;
	memcpy((void *)(val_offset + (uint32_t)buf), src, size);

	if (memcmp((void *)((uint32_t)&_eeprom_start + val_offset), (void *)((uint32_t)buf + val_offset), size) == 0) {
		DEBUG("eeprom: NOT need update\n");
		free(buf);
		return;
	}

	DEBUG("eeprom: start update\n");

	HAL_FLASH_Unlock();

	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = (uint32_t)&_eeprom_start;
	EraseInitStruct.NbPages     = 1;

	HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);

	uint32_t faddr;
	uint32_t *data_ptr;

	faddr = (uint32_t)&_eeprom_start;
	data_ptr = (uint32_t *)buf;

	while (faddr < (uint32_t)&_eeprom_end) {
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, faddr, *data_ptr) == HAL_OK) {
			faddr += 4;
			data_ptr++;
		}
	}

	free(buf);

	HAL_FLASH_Lock();
	DEBUG("eeprom: update end\n");

}



