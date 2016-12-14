/*
 * devices.cc
 *
 *  Created on: 30.7.2014
 *      Author: horinek
 */

#include "devices.h"
#include "../../fc/fc.h"

#ifndef STM32
I2c mems_i2c;
#else
I2C_HandleTypeDef mems_i2c;
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

#define MEMS_I2C_RX_SIZE	100
#define MEMS_I2C_TX_SIZE	8

uint8_t mems_i2c_rx_buffer[MEMS_I2C_RX_SIZE];
uint8_t mems_i2c_tx_buffer[MEMS_I2C_TX_SIZE];

RingBufferSmall mems_i2c_rx(MEMS_I2C_RX_SIZE, mems_i2c_rx_buffer);
RingBufferSmall mems_i2c_tx(MEMS_I2C_TX_SIZE, mems_i2c_tx_buffer);

bool mems_i2c_ok = false;

bool mems_i2c_init()
{
#ifndef STM32
	//Enable I2C peripheral
	MEMS_I2C_PWR_ON;

	//stabilize power
	_delay_ms(10);

	mems_i2c.InitMaster(MEMS_I2C, 800000ul, &mems_i2c_rx, &mems_i2c_tx);
	mems_i2c.StartTransmittion(0,0);
	_delay_ms(1);
	if (mems_i2c.Status() != i2c_idle)
		return false;

	mems_i2c_ok = true;
#else
	mems_i2c.Instance = I2C1;
	mems_i2c.Init.ClockSpeed = 400000;
	mems_i2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
	mems_i2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	mems_i2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
	mems_i2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
	mems_i2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;

	mems_i2c.Init.OwnAddress1     = 0xFF;
	mems_i2c.Init.OwnAddress2     = 0xFF;

	HAL_I2C_Init(&mems_i2c);

	//hack for reset i2c bysu flag - manually make stop condition on bus
    if (__HAL_I2C_GET_FLAG(&mems_i2c, I2C_FLAG_BUSY) == SET)
    {
    	GPIO_InitTypeDef  GPIO_InitStruct;

    	GPIO_InitStruct.Pin       = GPIO_PIN_6 //scl
    							  | GPIO_PIN_7; //sda
    	GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_OD;
    	GPIO_InitStruct.Pull      = GPIO_NOPULL;
    	GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
    	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
    	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);

    	_delay_ms(10);

    	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
    	_delay_ms(10);
    	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);

    	GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
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
