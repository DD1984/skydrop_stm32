#include "eeprom.h"
#include "fc/conf.h"

void update_eeprom(void *addr, void *src, uint16_t size);

void eeprom_update_byte(void *addr, uint8_t val)
{
	update_eeprom(addr, &val, sizeof(uint8_t));
}

void eeprom_update_word(void *addr, uint16_t val)
{
	update_eeprom(addr, &val, sizeof(uint16_t));
}

void eeprom_update_float(void *addr, float val)
{
	update_eeprom(addr, &val, sizeof(float));
}

static FLASH_EraseInitTypeDef EraseInitStruct;
uint32_t PAGEError = 0;
extern uint32_t _eeprom_start;
extern uint32_t _eeprom_end;

void update_eeprom(void *addr, void *src, uint16_t size)
{
	cfg_t temp_cfg;

	if (((uint32_t)addr - (uint32_t)&config_ee) > sizeof(cfg_t)) {
		DEBUG("eeprom: unknown seection for update\n");
		return;
	}

	memcpy((void *)&temp_cfg, (void *)&config_ee, sizeof(cfg_t));

	uint32_t val_offset = (uint32_t)addr - (uint32_t)&config_ee;
	memcpy((void *)(val_offset + (uint32_t)&temp_cfg), src, size);

	if (memcmp((void *)((uint32_t)&temp_cfg + val_offset), (void *)((uint32_t)&config_ee + val_offset), size) == 0) {
		DEBUG("eeprom: NOT need update\n");
		return;
	}

	DEBUG("eeprom: need update\n");

	HAL_FLASH_Unlock();

	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = (uint32_t)&_eeprom_start;
	EraseInitStruct.NbPages     = 1;

	HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);

	uint32_t faddr;
	uint32_t *data_ptr;

	faddr = (uint32_t)&_eeprom_start;
	data_ptr = (uint32_t *)&temp_cfg;

	while (faddr < (uint32_t)&_eeprom_end) {
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, faddr, *data_ptr) == HAL_OK) {
			faddr += 4;
			data_ptr++;
		}
	}

	HAL_FLASH_Lock();
	DEBUG("eeprom: update end\n");

}



