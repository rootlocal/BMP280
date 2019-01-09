/*
 * BMP280.c
 *
 *  Created on: Jan 4, 2019
 *      Author: Tomasz Masłoń
 */

#include "BMP280.h"

BMP280_configData *BMP280;
BMP280_data *BMP280_readings;


static uint8_t BMP280_read8b(uint8_t address)
{
	uint8_t tmp;

	HAL_I2C_Mem_Read(BMP280->handler, BMP280_IC2ADDRESS, address, 1, &tmp, 1, 10);

	return tmp;
}

static uint16_t BMP280_read16b(uint8_t address)
{
	uint8_t tmp[2];
	// tmp without dereferece bc tmp is a table
	HAL_I2C_Mem_Read(BMP280->handler, BMP280_IC2ADDRESS, address, 1, tmp, 2, 10);

	return (tmp[0]<<8 | tmp[1]); // combines two 8b into one 16b
}

static uint32_t BMP280_read24b(uint8_t address)
{
	uint8_t tmp[3];
	// tmp without dereferece bc tmp is a table
	HAL_I2C_Mem_Read(BMP280->handler, BMP280_IC2ADDRESS, address, 1, tmp, 3, 10);

	return (tmp[0]<<16 | tmp[1]<<8 | tmp[2]); // combines three 8b into one 24b
}

static void BMP280_write8b(uint8_t address, uint8_t data)
{
	HAL_I2C_Mem_Write(BMP280->handler, BMP280_IC2ADDRESS, address, 1, &data, 1, 10);
}

static void BMP280_setConfig(uint8_t standbyTime, uint8_t filter)
{
	// & 0x7 bc only 3 bits are valid(in one data), & OxFC bc only 6 bits are valid(in whole register, 2x data)
	BMP280_write8b(BMP280_CONFIG,((standbyTime & 0x7) << 5  | (filter & 0x7) << 2 ) & 0xFC);
}


void BMP280_init(I2C_HandleTypeDef *i2c_hndlr, uint8_t pOversamp, uint8_t tempRes, uint8_t mode)
{
	BMP280->handler=i2c_hndlr;
	BMP280->powerMode=mode;
	if(BMP280->powerMode == BMP280_MODE_STANDARD)
	{
		BMP280_setConfig(BMP280_STNDBY_500,BMP280_FILTER_2);
	}
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

	BMP280_write8b(BMP280_CTRL_MEASUREMENTS,(((BMP280->temperatureResolution & 0x7)<<5) |
			((BMP280->pressureOversampling & 0x7)<<2) | mode) & 0x3);
}


static void BMP280_getTemperatureData()
{
	int32_t var1, var2;
	if(BMP280->powerMode==BMP280_MODE_FORCED)
	 {
		  uint8_t powerMode;
		  uint8_t ctrl = BMP280_read8b(BMP280_CTRL_MEASUREMENTS);
		  ctrl &= ~(0x03); 	/*we only want to change power mode*/
		  ctrl |= BMP280_MODE_FORCED;
		  BMP280_write8b(BMP280_CTRL_MEASUREMENTS, ctrl);

		  powerMode = BMP280_read8b(BMP280_CTRL_MEASUREMENTS); 	// Read written mode
		  powerMode &= 0x03;							// Do not work without it...

		  if(powerMode == BMP280_MODE_FORCED)
		  {
			  while(1) // Wait for end of conversion
			  {
				  powerMode = BMP280_read8b(BMP280_CTRL_MEASUREMENTS);
				  powerMode &= 0x03;
				  if(powerMode == BMP280_MODE_FORCED)
					  break;
			  }

			  int32_t adc_T = BMP280_read24b(BMP280_TEMPERATURE_DATA);
			  adc_T >>= 4;

			  var1  = ((((adc_T>>3) - ((int32_t)BMP280->t1 <<1))) *
					  ((int32_t)BMP280->t2)) >> 11;

			  var2  = (((((adc_T>>4) - ((int32_t)BMP280->t1)) *
					  ((adc_T>>4) - ((int32_t)BMP280->t1))) >> 12) *
					  ((int32_t)BMP280->t3)) >> 14;

			  BMP280->t_fine = var1 + var2;

			  float T  = (BMP280->t_fine * 5 + 128) >> 8;
			  BMP280_readings->temperature=T/100;
		  }
	}
}


static void BMP280_getPressureData()
{
	  int64_t var1, var2, p;

	  // Must be done first to get the t_fine variable set up
	  BMP280_getTemperatureData();

	  int32_t adc_P = BMP280_read24b(BMP280_PRESSURE_DATA);
	  adc_P >>= 4;

	  var1 = ((int64_t)BMP280->t_fine) - 128000;
	  var2 = var1 * var1 * (int64_t)BMP280->p6;
	  var2 = var2 + ((var1*(int64_t)BMP280->p5)<<17);
	  var2 = var2 + (((int64_t)BMP280->p4)<<35);
	  var1 = ((var1 * var1 * (int64_t)BMP280->p3)>>8) +
	    ((var1 * (int64_t)BMP280->p2)<<12);
	  var1 = (((((int64_t)1)<<47)+var1))*((int64_t)BMP280->p1)>>33;

	  if (var1 == 0) {
	    return ;  // avoid exception caused by division by zero
	  }
	  p = 1048576 - adc_P;
	  p = (((p<<31) - var2)*3125) / var1;
	  var1 = (((int64_t)BMP280->p9) * (p>>13) * (p>>13)) >> 25;
	  var2 = (((int64_t)BMP280->p8) * p) >> 19;

	  p = ((p + var1 + var2) >> 8) + (((int64_t)BMP280->p7)<<4);
	  BMP280_readings->pressure=(int32_t)p/256;
}


