/*
 * devices.h
 *
 *  Created on: 30.7.2014
 *      Author: horinek
 */

#ifndef DEVICES_H_
#define DEVICES_H_

#include "../../skydrop.h"

#ifdef LSM303D_SUPPORT
#include "lsm303d.h"
#endif
#ifdef MS5611_SUPPORT
#include "ms5611.h"
#endif
#ifdef L3GD20_SUPPORT
#include "l3gd20.h"
#endif
#ifdef SHT21_SUPPORT
#include "sht21.h"
#endif
#ifdef GPS_L80_SUPPORT
#include "gps_l80.h"
#endif

#if defined(LSM303D_SUPPORT) || defined(MS5611_SUPPORT) || defined(L3GD20_SUPPORT)  || defined(SHT21_SUPPORT)  
extern I2c mems_i2c;
#endif

#ifdef LSM303D_SUPPORT
extern Lsm303d lsm303d;
#endif
#ifdef MS5611_SUPPORT
extern MS5611 ms5611;
#endif
#ifdef L3GD20_SUPPORT
extern L3gd20 l3gd20;
#endif
#ifdef SHT21_SUPPORT
extern SHT21 sht21;
#endif

int32_t to_dec_3(int64_t c);
int16_t to_dec_2(int32_t c);
int8_t to_dec_1(int8_t c);

bool mems_i2c_init();
bool mems_i2c_selftest();


#endif /* DEVICES_H_ */
