/*
 * devices.cc
 *
 *  Created on: 30.7.2014
 *      Author: horinek
 */

#include "devices.h"
#include "../../fc/fc.h"

#if defined(LSM303D_SUPPORT) || defined(MS5611_SUPPORT) || defined(L3GD20_SUPPORT)  || defined(SHT21_SUPPORT)
I2c mems_i2c;
#endif

#ifdef LSM303D_SUPPORT
Lsm303d lsm303d;
#endif
#ifdef MS5611_SUPPORT
MS5611 ms5611;
#endif
#ifdef L3GD20_SUPPORT
L3gd20 l3gd20;
#endif
#ifdef SHT21_SUPPORT
SHT21 sht21;
#endif

bool mems_i2c_ok = false;

bool mems_i2c_init()
{
#ifndef STM32
	//Enable I2C peripheral
	MEMS_I2C_PWR_ON;

	//stabilize power
	_delay_ms(10);

	mems_i2c.InitMaster(MEMS_I2C, 800000ul, 100, 8);
	mems_i2c.StartTransmittion(0,0);
	_delay_ms(1);
	if (mems_i2c.Status() != i2c_idle)
		return false;

	mems_i2c_ok = true;
#endif
	return true;
}

bool mems_i2c_selftest()
{
	return mems_i2c_ok;
}

int32_t to_dec_3(int64_t c)
{
	if (c < (int64_t)0x800000)
		return c;
	return (int64_t)c - (int64_t)0x1000000;
}

int16_t to_dec_2(int32_t c)
{
	if (c < (int32_t)0xFF)
		return c;
	return (int32_t)c - (int32_t)0x100;
}

int8_t to_dec_1(int8_t c)
{
	if (c<32) return c;
	return c - 64;
}

//ISR(AM_INT1_INT)
//{
////	fc_mag_irq();
//}
//
//ISR(AM_INT2_INT)
//{
////	fc_acc_irq();
//}
