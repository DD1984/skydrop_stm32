#include "raw.h"
#include "logger.h"
#include "../../drivers/storage/storage.h"

bool raw_start(char * path)
{
	char filename[128];

	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t wday;
	uint8_t month;
	uint16_t year;

	datetime_from_epoch(time_get_utc(), &sec, &min, &hour, &day, &wday, &month, &year);

	sprintf_P(filename, PSTR("/%s/%02d-%02d%02d.RAW"), path, logger_flight_number, hour, min);
	DEBUG("RAW filename %s\n", filename);

	uint8_t res = f_open(&log_file, filename, FA_WRITE | FA_CREATE_ALWAYS);
	assert(res == FR_OK);

	//cannot create file
	if (res != FR_OK)
		return false;

	return LOGGER_ACTIVE;
}

void raw_step()
{
	uint8_t line[35];
	UINT wl;
	uint8_t l = 35;

	line[0] = 0xAA;

	uint32_t time = task_get_ms_tick();
	memcpy(line + 1, &time, 4);

#ifdef LSM303D_SUPPORT
	memcpy(line + 5, (void *)&fc.acc_data, 6);
	memcpy(line + 17, (void *)&fc.mag_data, 6);
#endif

#ifdef L3GD20_SUPPORT	
	memcpy(line + 11, (void *)&fc.gyro_data, 6);
#endif	

	memcpy(line + 23, (void *)&fc.altitude1, 4);

	if (fc.gps_data.valid)
	{
		memcpy(line + 27, (void *)&fc.gps_data.latitude, 4);
		memcpy(line + 31, (void *)&fc.gps_data.longtitude, 4);
	}
	else
	{
		memset(line + 27, 0xFF, 8);
	}

	assert(f_write(&log_file, line, l, &wl) == FR_OK);
	assert(wl == l);

#ifndef STM32
	//this is very slowly on spi flash
	assert(f_sync(&log_file) == FR_OK);
#endif
}

void raw_stop()
{
	assert(f_close(&log_file) == FR_OK);
}
