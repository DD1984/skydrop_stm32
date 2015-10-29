/*
 * BMP180.cc
 *
 *  Created on: 29.11.2013
 *      Author: horinek
 */

#include "bmp180.h"
#include "devices.h"

#ifndef STM32
void BMP180::Init(I2c * i2c, uint8_t dummy)
#else
#include "fc/kalman.h"
KalmanFilter * kalmanFilter_temp;
void BMP180::Init(I2C_HandleTypeDef *i2c, uint8_t dummy)
#endif
{
	this->i2c = i2c;
	this->press_osr = BMP180_OSR_ULTRA;

	kalmanFilter_temp = new KalmanFilter(1.0);
	//this->CheckID();

	this->ReadPROM();
}


bool BMP180::EOC(void)
{
	uint8_t data = BMP180_CONTROL;

	while (HAL_I2C_Master_Transmit(this->i2c, (uint16_t)BMP180_ADDRESS, &data, 1, 1000)!= HAL_OK);

	while (HAL_I2C_Master_Receive(this->i2c, (uint16_t)BMP180_ADDRESS, &data, 1, 1000) != HAL_OK);

	if (data & (1<<5))
		return false;
	else
		return true;
}

uint8_t BMP180::CheckID()
{
#ifndef STM32
	this->i2c->Write(BMP180_CHIP_ID_REG);
	this->i2c->StartTransmittion(BMP180_ADDRESS, 1);
	this->i2c->Wait();

	uint8_t data = this->i2c->Read();
#else
	uint8_t data = BMP180_CHIP_ID_REG;

	while (HAL_I2C_Master_Transmit(this->i2c, (uint16_t)BMP180_ADDRESS, &data, 1, 1000)!= HAL_OK);

	while (HAL_I2C_Master_Receive(this->i2c, (uint16_t)BMP180_ADDRESS, &data, 1, 1000) != HAL_OK);
#endif

	if (data != BMP180_CHIP_ID)
	{
		while(1)
		{
			printf("BMP180 ERROR %d\n", data);
			_delay_ms(500);
		}
	}
	else
		printf("BMP180 OK\n");

	return true;
}

uint16_t BMP180::Read16(uint8_t cmd)
{
#ifndef STM32
	this->i2c->Write(cmd);
	this->i2c->StartTransmittion(BMP180_ADDRESS, 2);
	this->i2c->Wait();

	uint8_t a = this->i2c->Read();
	uint8_t b = this->i2c->Read();

	return (a << 8) | b;
#else
	uint8_t buf[2];
	while (HAL_I2C_Master_Transmit(this->i2c, (uint16_t)BMP180_ADDRESS, &cmd, 1, 1000)!= HAL_OK);

	while (HAL_I2C_Master_Receive(this->i2c, (uint16_t)BMP180_ADDRESS, buf, 2, 1000) != HAL_OK);

	return (buf[0] << 8) | buf[1];
#endif
}

uint32_t BMP180::Read24(uint8_t cmd)
{
	union {uint32_t val; uint8_t raw[4];} res;

#ifndef STM32
	this->i2c->Write(cmd);
	this->i2c->StartTransmittion(BMP180_ADDRESS, 3);
	this->i2c->Wait();

	res.raw[2] = this->i2c->Read();
	res.raw[1] = this->i2c->Read();
	res.raw[0] = this->i2c->Read();

#else
	while (HAL_I2C_Master_Transmit(this->i2c, (uint16_t)BMP180_ADDRESS, &cmd, 1, 1000)!= HAL_OK);

	res.val = 0;
	while (HAL_I2C_Master_Receive(this->i2c, (uint16_t)BMP180_ADDRESS, (uint8_t *)&res.raw, 3, 1000) != HAL_OK);

	uint8_t temp = res.raw[0];
	res.raw[0] = res.raw[2];
	res.raw[2] = temp;

#endif
	return res.val;
}

void BMP180::ReadPROM()
{
//	mark();

    Calibration_AC1 = this->Read16(BMP180_PROM + 0);
    Calibration_AC2 = this->Read16(BMP180_PROM + 2);
    Calibration_AC3 = this->Read16(BMP180_PROM + 4);
    Calibration_AC4 = this->Read16(BMP180_PROM + 6);
    Calibration_AC5 = this->Read16(BMP180_PROM + 8);
    Calibration_AC6 = this->Read16(BMP180_PROM + 10);

    Calibration_B1 = this->Read16(BMP180_PROM + 12);
    Calibration_B2 = this->Read16(BMP180_PROM + 14);
    Calibration_MB = this->Read16(BMP180_PROM + 16);
    Calibration_MC = this->Read16(BMP180_PROM + 18);
    Calibration_MD = this->Read16(BMP180_PROM + 20);

	printf("AC1  %d\n", this->Calibration_AC1);
	printf("AC2  %d\n", this->Calibration_AC2);
	printf("AC3  %d\n", this->Calibration_AC3);
	printf("AC4  %u\n", this->Calibration_AC4);
	printf("AC5  %u\n", this->Calibration_AC5);
	printf("AC6  %u\n", this->Calibration_AC6);
	printf("B1   %d\n", this->Calibration_B1);
	printf("B2   %d\n", this->Calibration_B2);
	printf("MB   %d\n", this->Calibration_MB);
	printf("MC   %d\n", this->Calibration_MC);
	printf("MD   %d\n", this->Calibration_MD);
}

void BMP180::Write(uint8_t address, uint8_t data)
{
#ifndef STM32
	this->i2c->Write(address);
	this->i2c->Write(data);
	this->i2c->StartTransmittion(BMP180_ADDRESS, 0);
	this->i2c->Wait();
#else
	uint8_t buf[2];
	buf[0] = address;
	buf[1] =  data;
	while (HAL_I2C_Master_Transmit(this->i2c, (uint16_t)BMP180_ADDRESS, buf, 2, 1000)!= HAL_OK);
#endif
}

void BMP180::StartPressure()
{
	this->Write(BMP180_CONTROL, BMP180_PRESSURE_MEAS | this->press_osr << 6);
}

void BMP180::StartTemperature()
{
	this->Write(BMP180_CONTROL, BMP180_TEMPERATURE_MEAS);
}

void BMP180::ReadPressure()
{
    this->raw_pressure = this->Read24(BMP180_READ) >> (8 - this->press_osr);
}

void BMP180::ReadTemperature()
{
	this->raw_temperature = this->Read16(BMP180_READ);
}

void BMP180::CompensateTemperature()
{
	int32_t x1 = ((this->raw_temperature - Calibration_AC6) * Calibration_AC5) >> 15;
	int32_t x2 = ((int64_t)Calibration_MC << 11) / (x1 + Calibration_MD);

    this->param_b5 = x1 + x2;

    this->temperature = (int16_t)((this->param_b5 + 8) >> 4) / 10.0;
    this->itemperature = this->param_b5;
}

void BMP180::CompensatePressure()
{
    int32_t b6 = this->param_b5 - 4000;
    int32_t x1 = (this->Calibration_B2 * (b6 * b6 >> 12)) >> 11;
    int32_t x2 = this->Calibration_AC2 * b6 >> 11;
    int32_t x3 = x1 + x2;
    int32_t b3 = ((((int32_t)this->Calibration_AC1 * 4 + x3) << this->press_osr) + 2) / 4;
    x1 = this->Calibration_AC3 * b6 >> 13;
    x2 = (this->Calibration_B1 * (b6 * b6 >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    int32_t b4 = (int32_t)this->Calibration_AC4 * (x3 + 32768) >> 15;
    uint32_t b7 = (((this->raw_pressure - b3)) * (50000 >> this->press_osr));

    int32_t p;

    if (b7 < 0x80000000)
		p = ((b7 * 2) / b4);
    else
        p = ((b7 / b4) * 2);

    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;

    p = p + ((x1 + x2 + 3791) >> 4);

   //this->pressure = get_p(p);
    this->pressure = p;
}

float BMP180::GetAltitude(float currentSeaLevelPressureInPa, float pressure)
{
    // Calculate altitude from sea level.
    float altitude = 44330.0 * (1.0 - pow(pressure / currentSeaLevelPressureInPa, 0.1902949571836346));
    return altitude;
}
