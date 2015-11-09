#include "fc.h"

#include "../drivers/sensors/devices.h"
#ifdef UART_SUPPORT
#include "../drivers/uart.h"
#endif
#include "../drivers/audio/audio.h"

#include "kalman.h"
#include "vario.h"

#include "protocols/protocol.h"

volatile flight_data_t fc;

#ifdef FC_TIMER
Timer fc_meas_timer;
#else

TIM_HandleTypeDef fc_meas_timer;
#endif

#define TIM4CLK 125000 //125 == 1ms

#define MEAS_TEMP_TIME ((TIM4CLK / 1000) * 2) //typical time
#define MEAS_PRES_TIME ((TIM4CLK / 1000) * 16) //typical time
#define MEAS_POLL_TIME ((TIM4CLK / 1000) * 1) //polling time for detect EOC

enum {FCT_MEAS_TEMP, FCT_MEAS_PRES, FCT_MEAS_END};

uint8_t fc_meas_timer_state = FCT_MEAS_TEMP;

extern KalmanFilter * kalmanFilter;

void fc_init()
{
	DEBUG(" *** Flight computer init ***\n");

	//start values
	active_page = config.gui.last_page;
	if (active_page >= config.gui.number_of_pages)
		active_page = 0;

#ifdef RTC_SUPPORT
	fc.epoch_flight_timer = time_get_actual();
#endif
	fc.autostart_state = AUTOSTART_WAIT;

	fc.temp_step = 0;


	//init calculators
	vario_init();
#ifdef AUDIO_SUPPORT	
	audio_init();
#endif	

#ifdef GPS_SUPPORT
	gps_init();
	if (config.system.use_gps)
		gps_start();
#endif		

#ifdef BT_SUPPORT
	bt_init();
	if (config.system.use_bt)
		bt_module_init();
#endif

	//VCC to baro, acc/mag gyro + i2c pull-ups
	mems_power_on();

	//init and test i2c
	if (!mems_i2c_init())
	{
		DEBUG("ERROR I2C\n");
#ifdef LED_SUPPORT
		led_set(0xFF, 0, 0);
#endif
#ifndef STM32
		if (hw_revision == HW_REW_UNKNOWN)
		{
			hw_revision = HW_REW_1504;
			eeprom_busy_wait();
			eeprom_update_byte(&config_ro.hw_revision, hw_revision);
			eeprom_busy_wait();

			mems_power_init();
			io_init();
			mems_power_on();
			mems_i2c_init();
		}
#endif
	}
	else
	{
#ifndef STM32
		if (hw_revision == HW_REW_UNKNOWN)
		{
			hw_revision = HW_REW_1506;
			eeprom_busy_wait();
			eeprom_update_byte(&config_ro.hw_revision, hw_revision);
			eeprom_busy_wait();

			mems_power_init();
			io_init();
			mems_power_on();
			mems_i2c_init();
		}
#endif
	}

#ifdef MS5611_SUPPORT
	//Barometer
	ms5611.Init(&mems_i2c, MS5611_ADDRESS_CSB_LO);
#endif

#if defined(BMP180_SUPPORT) && defined(STM32)
	//Barometer
	bmp180.Init(&mems_i2c, 0);
#endif

#ifdef LSM303D_SUPPORT
	//Magnetometer + Accelerometer
	lsm303d_settings lsm_cfg;

	lsm_cfg.enabled = true;
	lsm_cfg.accOdr = lsm_acc_1600Hz;
	lsm_cfg.accScale = lsm_acc_16g;

	lsm_cfg.magOdr = lsm_mag_100Hz;
	lsm_cfg.magScale = lsm_mag_4g;
	lsm_cfg.magHiRes = true;

	lsm_cfg.tempEnable = false;
#endif	

#ifdef L3GD20_SUPPORT
	//Gyro
	l3gd20_settings l3g_cfg;
	l3g_cfg.enabled = true;
	l3g_cfg.bw = l3g_50Hz;
	l3g_cfg.odr = l3g_760Hz;
	l3g_cfg.scale = l3g_2000dps;
#endif	

#ifdef SHT21_SUPPORT
	sht21_settings sht_cfg;
	sht_cfg.rh_enabled = true;
	sht_cfg.temp_enabled = true;
#endif	

#ifdef LSM303D_SUPPORT
	//XXX: do self-test?
	lsm303d.Init(&mems_i2c, lsm_cfg);
	lsm303d.Start();
#endif

#ifdef L3GD20_SUPPORT
	l3gd20.Init(&mems_i2c, l3g_cfg);
	l3gd20.Start();
#endif	

#ifdef SHT21_SUPPORT
	sht21.Init(&mems_i2c, sht_cfg);
#endif	

#ifdef FC_TIMER
	//Measurement timer
	FC_MEAS_TIMER_PWR_ON;

	fc_meas_timer.Init(FC_MEAS_TIMER, timer_div256); //125 == 1ms
	fc_meas_timer.SetInterruptPriority(MEDIUM);
	fc_meas_timer.EnableInterrupts(timer_overflow | timer_compareA | timer_compareB | timer_compareC);
	fc_meas_timer.SetTop(125 * 10); // == 10ms
	fc_meas_timer.SetCompare(timer_A, 100); // == 0.64ms
	fc_meas_timer.SetCompare(timer_B, 430); // == 2.7ms
	fc_meas_timer.SetCompare(timer_C, 555); // == 3.7ms
	fc_meas_timer.Start();
#else
	fc_meas_timer.Instance = TIM4;
	fc_meas_timer.Init.Prescaler = (uint32_t)(SystemCoreClock / TIM4CLK) - 1;;
	fc_meas_timer.Init.ClockDivision = 0;
	fc_meas_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
	fc_meas_timer.Init.RepetitionCounter = 0;
	fc_meas_timer.Init.Period = MEAS_POLL_TIME;
	HAL_TIM_Base_Init(&fc_meas_timer);
	HAL_TIM_Base_Start_IT(&fc_meas_timer);
#endif

	DEBUG(" *** FC init done ***\n");

}

void fc_deinit()
{
	eeprom_busy_wait();
	//store altimeter info
	eeprom_update_float(&config_ee.altitude.QNH1, config.altitude.QNH1);
	eeprom_update_float(&config_ee.altitude.QNH2, config.altitude.QNH2);


	for (uint8_t i=0; i<NUMBER_OF_ALTIMETERS; i++)
	{
		eeprom_update_word((uint16_t *)&config_ee.altitude.altimeter[i].delta, config.altitude.altimeter[i].delta);
	}

	mems_power_off();
}

void fc_pause()
{
#ifdef FC_TIMER	
	fc_meas_timer.Stop();
#endif	
}

void fc_continue()
{
#ifdef FC_TIMER
	fc_meas_timer.Start();
#endif	
}

#ifdef STM32
static BMP180 *bmp180_ptr = &bmp180;
void fc_meas_timer_ovf(void)
{
#if 1
	if (bmp180_ptr->EOC() == false) {
		fc_meas_timer.Instance->CNT = 0;
		fc_meas_timer.Instance->ARR = MEAS_POLL_TIME;
		return;
	}
#endif
	if (fc_meas_timer_state == FCT_MEAS_TEMP) {
		bmp180_ptr->ReadTemperature();
		bmp180_ptr->StartPressure();

		fc_meas_timer.Instance->CNT = 0;
		fc_meas_timer.Instance->ARR = MEAS_PRES_TIME;

		fc_meas_timer_state = FCT_MEAS_PRES;


		vario_calc(bmp180_ptr->pressure);

		//audio loop
		audio_step();

		bmp180_ptr->CompensateTemperature();
		return;
	}
	if (fc_meas_timer_state == FCT_MEAS_PRES) {
		bmp180_ptr->ReadPressure();

		bmp180_ptr->StartTemperature();

		fc_meas_timer.Instance->CNT = 0;
		fc_meas_timer.Instance->ARR = MEAS_TEMP_TIME;

		fc_meas_timer_state = FCT_MEAS_TEMP;

		bmp180_ptr->CompensatePressure();
		return;
	}
}
#endif

#ifdef FC_TIMER
//First fc meas period
// * Read pressure from ms5611
// * Start temperature conversion ms5611
// * Init lsm303d i2c readout for magnetometer (block the i2c bus)
// * Compensate pressure from ms5611
ISR(FC_MEAS_TIMER_OVF)
{
	BT_SUPRESS_TX
	io_write(1, HIGH);

#ifdef MS5611_SUPPORT
	ms5611.ReadPressure();
	ms5611.StartTemperature();
#endif
#ifdef BMP180_SUPPORT
	bmp180.ReadPressure();
	bmp180.StartTemperature();
#endif	

#ifdef LSM303D_SUPPORT
	lsm303d.StartReadMag(); //it takes 152us to transfer
#endif	

#ifdef MS5611_SUPPORT
	ms5611.CompensatePressure();
#endif
#ifdef BMP180_SUPPORT
	bmp180.CompensatePressure();
#endif
	io_write(1, LOW);
	BT_ALLOW_TX
}

//Second fc meas period
// * Load lsm303d magnetometer data from buffer (free the i2c bus)
// * Read temperature form ms5611
// * Start pressure conversion ms5611
// * Init lsm303d i2c readout for accelerometer (block the i2c bus)
// * Calculate time sensitive values
// * Setup the Buzzer
// * Compensate temperature
ISR(FC_MEAS_TIMER_CMPA)
{
	BT_SUPRESS_TX
	io_write(1, HIGH);

#ifdef LSM303D_SUPPORT
	lsm303d.ReadMag(&fc.mag_data.x, &fc.mag_data.y, &fc.mag_data.z);
#endif

#ifdef MS5611_SUPPORT	
	ms5611.ReadTemperature();
	ms5611.StartPressure();
#endif
#ifdef BMP180_SUPPORT
	bmp180.ReadTemperature();
	bmp180.StartPressure();
#endif

#ifdef LSM303D_SUPPORT	
	lsm303d.StartReadAccStream(16); //it take 1600us to transfer
#endif	

#ifdef MS5611_SUPPORT
	vario_calc(ms5611.pressure);
#endif
#ifdef BMP180_SUPPORT
	vario_calc(ms5611.pressure);
#endif

	//audio loop
	audio_step();

#ifdef MS5611_SUPPORT
	ms5611.CompensateTemperature();
#endif	
#ifdef BMP180_SUPPORT
	bmp180.CompensateTemperature();
#endif

	io_write(1, LOW);
	BT_ALLOW_TX
}

//Third fc meas period
// * Load lsm303d accelerometer data from buffer (free the i2c bus)
// * Init l3gd20 i2c readout for gyroscope (block the i2c bus)
ISR(FC_MEAS_TIMER_CMPB)
{
	BT_SUPRESS_TX
	io_write(1, HIGH);

#ifdef LSM303D_SUPPORT
	lsm303d.ReadAccStreamAvg(&fc.acc_data.x, &fc.acc_data.y, &fc.acc_data.z, 16);
#endif
#ifdef L3GD20_SUPPORT	
	l3gd20.StartReadGyroStream(7); //it take 1000us to transfer
#endif
	io_write(1, LOW);
	BT_ALLOW_TX
}

//Final fc meas period
// * Load l3gd20 gyroscope data from buffer (free the i2c bus)
// * Handle slow sht21 conversions
ISR(FC_MEAS_TIMER_CMPC)
{
	BT_SUPRESS_TX
	io_write(1, HIGH);

#ifdef L3GD20_SUPPORT
	l3gd20.ReadGyroStreamAvg(&fc.gyro_data.x, &fc.gyro_data.y, &fc.gyro_data.z, 7); //it take 1000us to transfer
#endif	

	if (fc.temp_next < task_get_ms_tick())
	{
		switch (fc.temp_step)
		{
#ifdef SHT21_SUPPORT			
			case(0):
				sht21.StartHumidity();
			break;
			case(1):
				sht21.Read();
			break;
			case(2):
				sht21.CompensateHumidity();
				fc.humidity = sht21.humidity;
			break;
			case(3):
				sht21.StartTemperature();
			break;
			case(4):
				sht21.Read();
			break;
			case(5):
				sht21.CompensateTemperature();
				fc.temperature = sht21.temperature;
			break;
#endif			
		}
		fc.temp_next = task_get_ms_tick() + FC_TEMP_PERIOD;
		fc.temp_step = (fc.temp_step + 1) % 6;
	}

	io_write(1, LOW);

//	DEBUG("A;%d;%d;%d\n", fc.acc_data.x, fc.acc_data.y, fc.acc_data.z);
//	DEBUG("M;%d;%d;%d\n", fc.mag_data.x, fc.mag_data.y, fc.mag_data.z);
//	DEBUG("G;%d;%d;%d\n", fc.gyro_data.x, fc.gyro_data.y, fc.gyro_data.z);

	BT_ALLOW_TX
}
#endif

void fc_takeoff()
{
	gui_showmessage_P(PSTR("Take off"));
	fc.autostart_state = AUTOSTART_FLIGHT;
#ifdef RTC_SUPPORT
	fc.epoch_flight_timer = time_get_actual();
#endif

	//zero altimeters at take off
	for (uint8_t i = 0; i < NUMBER_OF_ALTIMETERS; i++)
	{
		if (config.altitude.altimeter[i].flags & ALT_AUTO_ZERO)
			fc_zero_alt(i + 1);
	}
}

void fc_landing()
{
	gui_showmessage_P(PSTR("Landing"));
	fc.autostart_state = AUTOSTART_LAND;
#ifdef RTC_SUPPORT	
	fc.epoch_flight_timer = time_get_actual() - fc.epoch_flight_timer;
#endif	

	audio_off();
}

#ifdef GPS_SUPPORT
void fc_sync_gps_time()
{
	uint32_t diff = 0;

	//do not change flight time during update
	if (fc.autostart_state == AUTOSTART_FLIGHT)
		diff = time_get_actual() - fc.epoch_flight_timer;

	time_set_actual(fc.gps_data.utc_time + (config.system.time_zone * 3600ul) / 2);

	//do not change flight time during update
	if (fc.autostart_state == AUTOSTART_FLIGHT)
		fc.epoch_flight_timer = time_get_actual() - diff;

	gui_showmessage_P(PSTR("GPS Time set"));
}
#endif

void fc_step()
{
#ifdef GPS_SUPPORT	
	gps_step();
#endif	

#ifdef BT_SUPPORT
	bt_step();
#endif	

#ifdef EXT_CONNECTION_SUPPORT
	protocol_step();
#endif	

	//auto start
	if (fc.baro_valid && fc.autostart_state == AUTOSTART_WAIT)
	{
		if (abs(fc.altitude1 - fc.start_altitude) > config.autostart.sensititvity)
		{
			fc_takeoff();
		}
		else
		{
#ifdef RTC_SUPPORT			
			//reset wait timer
			if (time_get_actual() - fc.epoch_flight_timer > FC_AUTOSTART_RESET)
			{
				fc.epoch_flight_timer = time_get_actual();
				fc.start_altitude = fc.altitude1;
			}
#endif			
		}
	}

#ifdef GPS_SUPPORT
	//gps time sync
	if ((config.system.time_flags & TIME_SYNC) && fc.gps_data.fix_cnt == GPS_FIX_TIME_SYNC)
	{
		fc_sync_gps_time();
	}

	//glide ratio
	//when you hav GPS, baro and speed is higher than 2km/h and you are sinking
	if (fc.gps_data.valid && fc.baro_valid && fc.gps_data.groud_speed > FC_GLIDE_MIN_KNOTS && fc.avg_vario < 0.0)
	{
		float spd_m = fc.gps_data.groud_speed * FC_KNOTS_TO_MPS;
		fc.glide_ratio = spd_m / abs(fc.avg_vario);

		fc.glide_ratio_valid = true;
	}
	else
	{
		fc.glide_ratio_valid = false;
	}
#endif
}

float fc_alt_to_qnh(float alt, float pressure)
{
	return pressure / pow(1.0 - (alt / 44330.0), 5.255);
}

float fc_press_to_alt(float pressure, float qnh)
{
	return 44330.0 * (1 - pow((pressure / qnh), 0.190295));
}

float fc_alt_to_press(float alt, float qnh)
{
	return qnh * pow(1.0 - (alt / 44330.0), 5.255);
}

void fc_zero_alt(uint8_t index)
{
	index -= 1;

	if (config.altitude.altimeter[index].flags & ALT_DIFF)
		{
			uint8_t a_index = config.altitude.altimeter[index].flags & 0x0F;

			if (a_index == 0)
			{
				config.altitude.altimeter[index].delta = -fc.altitude1;
			}
			else
				config.altitude.altimeter[index].delta = -fc.altitudes[a_index];

		}
}

void fc_manual_alt0_change(float val)
{
    kalmanFilter->setXAbs(val);
    fc.start_altitude = val;
}

