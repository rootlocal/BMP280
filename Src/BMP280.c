/*
 * BMP280.c
 *
 *  Created on: Jan 4, 2019
 *      Author: Tomasz Masłoń
 */

#include "BMP280.h"

BMP280_configData *BMP280;

static uint8_t BMP280_read8b(uint8_t address)
{
	uint8_t tmp;

	HAL_I2C_Mem_Read(i2c_handler, BMP280_IC2ADDRESS, address, 1, &tmp, 1, 10);

	return tmp;
}

static uint16_t BMP280_read16b(uint8_t address)
{
	uint8_t tmp[2];
	// tmp without dereferece bc tmp is a table
	HAL_I2C_Mem_Read(i2c_handler, BMP280_IC2ADDRESS, address, 1, tmp, 2, 10);

	return (tmp[0]<<8 | tmp[1]); // combines two 8b into one 16b
}

static uint32_t BMP280_read24b(uint8_t address)
{
	uint8_t tmp[3];
	// tmp without dereferece bc tmp is a table
	HAL_I2C_Mem_Read(i2c_handler, BMP280_IC2ADDRESS, address, 1, tmp, 3, 10);

	return (tmp[0]<<16 | tmp[1]<<8 | tmp[2]); // combines three 8b into one 24b
}

static void BMP280_write8b(uint8_t address, uint8_t data)
{
	HAL_I2C_Mem_Write(i2c_handler, BMP280_IC2ADDRESS, address, 1, &data, 1, 10);
}

void BMP280_setConfig(uint8_t standbyTime, uint8_t filter)
{
	// & 0x7 bc only 3 bits are valid(in one data), & OxFC bc only 6 bits are valid(in whole register, 2x data)
	BMP280_write8b(BMP280_CONFIG,((standbyTime & 0x7) << 5  | (filter & 0x7) << 2 ) & 0xFC, 1, 10);
}


void BMP280_init(I2C_HandleTypeDef* i2c_hndlr, uint8_t pOversamp, uint8_t tempRes, uint8_t mode)
{
	BMP280->handler=i2c_hndlr;
	BMP280->powerMode=mode;
	BMP280->temperatureResolution=tempRes;
	BMP280->pressureOversampling=pOversamp;


	/* read calibration data */

	BMP280->t1=BMP280_read16b(BMP280_DIG_T1);
	BMP280->t2=BMP280_read16b(BMP280_DIG_T2);
	BMP280->t3=BMP280_read16b(BMP280_DIG_T3);

	BMP280->p1=BMP280_read16b(BMP280_DIG_P1);
	BMP280->p2=BMP280_read16b(BMP280_DIG_P2);
	BMP280->p3=BMP280_read16b(BMP280_DIG_P3);
	BMP280->p4=BMP280_read16b(BMP280_DIG_P4);
	BMP280->p5=BMP280_read16b(BMP280_DIG_P5);
	BMP280->p6=BMP280_read16b(BMP280_DIG_P6);
	BMP280->p7=BMP280_read16b(BMP280_DIG_P7);
	BMP280->p9=BMP280_read16b(BMP280_DIG_P8);
	BMP280->p9=BMP280_read16b(BMP280_DIG_P9);

}









