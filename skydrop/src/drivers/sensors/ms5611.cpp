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

	this->Reset();
	_delay_ms(10);
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

bool MS5611::CalcCRC(uint16_t *prom)
{
	int32_t i, j;
	uint32_t res = 0;
	uint8_t crc = prom[7] & 0xF;
	prom[7] &= 0xFF00;
	for (i = 0; i < 16; i++) {
		if (i & 1) {
			res ^= ((prom[i >> 1]) & 0x00FF);
		}
		else {
			res ^= (prom[i >> 1] >> 8);
		}
		for (j = 8; j > 0; j--) {
			if (res & 0x8000) {
				res ^= 0x1800;
			}
			res <<= 1;
		}
	}
	prom[7] |= crc;
	if (crc == ((res >> 12) & 0xF)) {
		return true;
	}
	else {
		return false;
	}
}

void MS5611::ReadPROM()
{
	DEBUG("ms5611 calibration data\n");
	uint8_t prom_addr = 0xA0;
	for (uint8_t i = 0; i < 8; i++) {
		uint16_t data = this->Read16(prom_addr);
		((uint16_t *)&calibration_C0)[i] = data;
		DEBUG(" C%d 0x%04X %u\n", i, data, data);
		prom_addr += 2;
	}

	DEBUG("CRC: ");
	if (CalcCRC(&calibration_C0) == true)
		DEBUG("OK\n");
	else
		DEBUG("FAIL!!!\n");
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
    assert(this->raw_pressure != 0);
}

void MS5611::ReadTemperature()
{
	this->raw_temperature = this->Read24(MS5611_READ);
    assert(this->raw_temperature != 0);
}

void MS5611::CompensateTemperature()
{
	this->dT = this->raw_temperature - (this->calibration_C5 * (int32_t)256);
	this->temperature = (2000ul + ((int64_t)this->dT * (int64_t)this->calibration_C6) / (int64_t)8388608);
}


void MS5611::CompensatePressure()
{
	int64_t off = (int64_t)this->calibration_C2 * (int64_t)65536 + ((int64_t)this->calibration_C4 * (int64_t)this->dT) / 128;
	int64_t sens = (int64_t)this->calibration_C1 * (int64_t)32768 + ((int64_t)this->calibration_C3 * (int64_t)this->dT) / 256;

	if (this->temperature < 2000)
	{
		//low temperature
		uint64_t t2 = (uint64_t)this->dT * (uint64_t)this->dT / 2147483648ul;
		uint64_t temp = (this->temperature - 2000) * (this->temperature - 2000);
		uint64_t off2 = 5 * temp / 2;
		uint64_t sens2 = off2 / 2;

		if (this->temperature < -1500)
		{
			//very low temperature
			temp = (this->temperature + 1500) * (this->temperature + 1500);
			off2 = off2 + 7 * temp;
			sens2 = sens2 + 11 * temp / 2;
		}

		this->temperature -= t2;
		off -= off2;
		sens -= sens2;
	}

	this->pressure = (float)((int32_t)this->raw_pressure * sens / (int32_t)2097152 - off) / 32768.0;
}

float MS5611::GetAltitude(float currentSeaLevelPressureInPa, float pressure)
{
    // Calculate altitude from sea level.
    float altitude = 44330.0 * (1.0 - pow(pressure / currentSeaLevelPressureInPa, 0.1902949571836346));
    return altitude;
}
