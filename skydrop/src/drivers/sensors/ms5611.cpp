/*
 * ms5611.cc
 *
 *  Created on: 29.11.2013
 *      Author: horinek
 */

#include "ms5611.h"
#include "devices.h"

#ifndef STM32
void MS5611::Init(I2c * i2c, uint8_t address)
#else
void MS5611::Init(I2C_HandleTypeDef *i2c, uint8_t address)
#endif
{
	this->i2c = i2c;
	this->address = address;

	this->press_osr = MS5611_OSR_4096;
	this->temp_osr = MS5611_OSR_256;

	DEBUG("3.1\n");
	this->Reset();
	DEBUG("3.2\n");
	_delay_ms(10);
	DEBUG("3.3\n");
	this->ReadPROM();
}

uint16_t MS5611::Read16(uint8_t cmd)
{
#ifndef STM32
	this->i2c->Write(cmd);
	this->i2c->StartTransmittion(this->address, 2);
	this->i2c->Wait();

	uint8_t a = this->i2c->Read();
	uint8_t b = this->i2c->Read();

	return (a << 8) | b;
#else
	uint8_t buf[2];
	while (HAL_I2C_Master_Transmit(this->i2c, (uint16_t)this->address, &cmd, 1, 1000)!= HAL_OK);

	while (HAL_I2C_Master_Receive(this->i2c, (uint16_t)this->address, buf, 2, 1000) != HAL_OK);

	return (buf[0] << 8) | buf[1];
#endif
}

uint32_t MS5611::Read24(uint8_t cmd)
{
	union {uint32_t val; uint8_t raw[4];} res;

#ifndef STM32
	this->i2c->Write(cmd);
	this->i2c->StartTransmittion(this->address, 3);
	this->i2c->Wait();

	res.raw[2] = this->i2c->Read();
	res.raw[1] = this->i2c->Read();
	res.raw[0] = this->i2c->Read();
#else
	while (HAL_I2C_Master_Transmit(this->i2c, (uint16_t)this->address, &cmd, 1, 1000)!= HAL_OK);

	res.val = 0;
	while (HAL_I2C_Master_Receive(this->i2c, (uint16_t)this->address, (uint8_t *)&res.raw, 3, 1000) != HAL_OK);

	uint8_t temp = res.raw[0];
	res.raw[0] = res.raw[2];
	res.raw[2] = temp;

#endif

	return res.val;
}

bool MS5611::SelfTest()
{
#ifndef STM32
	this->i2c->StartTransmittion(this->address, 0);
	this->i2c->Wait();

	if (this->i2c->Error())
	{
		return false;
	}
#endif

	return true;
}

void MS5611::ReadPROM()
{
	this->calibration_C1 = this->Read16(MS5611_PROM + 0);
	this->calibration_C2 = this->Read16(MS5611_PROM + 2);
	this->calibration_C3 = this->Read16(MS5611_PROM + 4);
	this->calibration_C4 = this->Read16(MS5611_PROM + 6);
	this->calibration_C5 = this->Read16(MS5611_PROM + 8);
	this->calibration_C6 = this->Read16(MS5611_PROM + 10);

	DEBUG("ms5611 calibration data\n");
	DEBUG(" C1 %u\n", this->calibration_C1);
	DEBUG(" C2 %u\n", this->calibration_C2);
	DEBUG(" C3 %u\n", this->calibration_C3);
	DEBUG(" C4 %u\n", this->calibration_C4);
	DEBUG(" C5 %u\n", this->calibration_C5);
	DEBUG(" C6 %u\n", this->calibration_C6);
}

void MS5611::Write(uint8_t cmd)
{
#ifndef STM32
	this->i2c->Write(cmd);
	this->i2c->StartTransmittion(this->address, 0);
	this->i2c->Wait();
#else
	while (HAL_I2C_Master_Transmit(this->i2c, (uint16_t)this->address, &cmd, 1, 1000)!= HAL_OK);
#endif

};

void MS5611::Reset()
{
	this->Write(MS5611_RESET);
}

void MS5611::StartPressure()
{
	this->Write(MS5611_D1 | this->press_osr);

}

void MS5611::StartTemperature()
{
	this->Write(MS5611_D2 | this->temp_osr);

}

void MS5611::ReadPressure()
{
    this->raw_pressure = this->Read24(MS5611_READ);
}

void MS5611::ReadTemperature()
{
	this->raw_temperature = this->Read24(MS5611_READ);
}

void MS5611::CompensateTemperature()
{
	this->dT = this->raw_temperature - (this->calibration_C5 * (int32_t)256);
	this->temperature = (2000 + ((int64_t)this->dT * (int64_t)this->calibration_C6) / (int64_t)8388608) / 100.0;
}


void MS5611::CompensatePressure()
{
	int64_t off = (int64_t)this->calibration_C2 * (int64_t)65536 + ((int64_t)this->calibration_C4 * (int64_t)this->dT) / 128;
	int64_t sens = (int64_t)this->calibration_C1 * (int64_t)32768 + ((int64_t)this->calibration_C3 * (int64_t)this->dT) / 256;

	this->pressure = (float)((int32_t)this->raw_pressure * sens / (int32_t)2097152 - off) / 32768.0;
}

float MS5611::GetAltitude(float currentSeaLevelPressureInPa, float pressure)
{
    // Calculate altitude from sea level.
    float altitude = 44330.0 * (1.0 - pow(pressure / currentSeaLevelPressureInPa, 0.1902949571836346));
    return altitude;
}
