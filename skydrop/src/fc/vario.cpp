#include "vario.h"

#include "fc.h"
#include "kalman.h"

KalmanFilter kalmanFilter(1.0);

void vario_init()
{
	fc.baro_valid = false;
	fc.avg_vario = 0;
	fc.digital_vario = 0;

	for (uint8_t i = 0; i < VARIO_HISTORY_SIZE; i++)
		fc.vario_history[i] = 0;

	vario_update_history_delay();
	fc.vario_history_pointer = 0;
	fc.vario_history_step = 0;
}

void vario_update_history_delay()
{
	fc.vario_history_delay = (mul_to_sec(config.vario.avg_vario_dampening) * 100) / VARIO_HISTORY_SIZE;
}

int16_t	vario_get_altitude(uint8_t flags, uint8_t index)
{
	switch (flags & 0b11000000)
	{
		case(ALT_ABS_QNH1):
			return fc_press_to_alt(fc.pressure, config.altitude.QNH1);
		case(ALT_ABS_QNH2):
			return fc_press_to_alt(fc.pressure, config.altitude.QNH2);
#ifdef GPS_SUPPORT
		case(ALT_ABS_GPS):
			if (fc.gps_data.valid)
				return fc.gps_data.altitude;
			else
				return 0;
#endif
		case(ALT_DIFF):
			uint8_t a_index = (flags & 0b00001111) + 1;
			if (a_index == 1)
				return fc.altitude1 + config.altitude.altimeter[index].delta;
			else
			{
				a_index -= 2;
				return fc.altitudes[a_index] + config.altitude.altimeter[index].delta;
			}
	}
	return 0;
}

//drop few first measurements (12s)
uint16_t vario_drop = 0;
#define VARIO_DROP	1200ul

void vario_calc(float pressure)
{
	//if pressure is NaN do not compute!
	if (isnan(pressure))
	{
		DEBUG("pressure is NaN\n");
		return;
	}

	float rawAltitude = fc_press_to_alt(pressure, config.altitude.QNH1);

	//if rawAltitude is NaN do not compute!
	if (isnan(rawAltitude))
	{
		DEBUG("rawAltitude is NaN\n");
		return;
	}

	kalmanFilter.update(rawAltitude);

	float vario = kalmanFilter.getXVel();
	float altitude = kalmanFilter.getXAbs();

//	DEBUG("%lu;%lu;%0.2f;%ld\n", ms5611.raw_pressure, ms5611.raw_temperature, ms5611.pressure, ms5611.temperature);

	fc.pressure = fc_alt_to_press(altitude, config.altitude.QNH1);

	fc.vario = vario;

	if (vario_drop < VARIO_DROP)
	{
		vario_drop++;
		if (vario_drop == VARIO_DROP)
		{
			fc.baro_valid = true;
			fc.autostart_altitude = altitude;
		}
		else
			return;
	}

	if (fc.vario_history_step % fc.vario_history_delay == 0)
	{
		float val = vario * VARIO_HISTORY_SCALE;
		if (val > 127) val = 127;
		if (val < -126) val = -126;
		fc.vario_history[fc.vario_history_pointer] = val;

		fc.vario_history_pointer = (fc.vario_history_pointer + 1) % VARIO_HISTORY_SIZE;
		fc.vario_history_step = 1;
	}
	else
		fc.vario_history_step++;

	//AVG vario and alt shoud start only on valid vario data
	fc.digital_vario += (vario - fc.digital_vario) * config.vario.digital_vario_dampening;
	fc.avg_vario += (vario - fc.avg_vario) * config.vario.avg_vario_dampening;

	fc.altitude1 = altitude;

	for (uint8_t i = 0; i < NUMBER_OF_ALTIMETERS; i++)
		fc.altitudes[i] = vario_get_altitude(config.altitude.altimeter[i].flags, i);
}
