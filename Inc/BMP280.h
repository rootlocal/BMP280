/*
 * BMP280.h
 *
 *  Created on: Jan 4, 2019
 *      Author: Tomasz Masłoń
 */

#include "stm32l4xx_hal.h"

/* BMP290 IC2 ADDRESS */

#define BMP280_IC2ADDRESS				0xEC

/* BMP280 POWER MODES */

#define BMP280_MODE_SLEEP				0
#define BMP280_MODE_FORCED				1
#define BMP280_MODE_STANDARD			3

/* BMP280 PRESSURE OVERSAMPLING/RESOLUTION */

#define BMP280_PRESSURE_DISABLED		0
#define BMP280_PRESSURE_ULTRALOW		1	/* 1X OVERSAMPLING */
#define BMP280_PRESSURE_LOWPOWER		2	/* 2X OVERSAMPLING */
#define BMP280_PRESSURE_STANDARD		3	/* 4X OVERSAMPLING */
#define BMP280_PRESSURE_HIGH			4	/* 8X OVERSAMPLING */
#define BMP280_PRESSURE_ULTRAHIGH		5	/* 16X OVERSAMPLING */

/* BMP280 TEMPERATURE OVERSAMPLING */

#define BMP280_TEMPERATURE_DISABLED		0
#define BMP280_TEMPERATURE_16B			1	/* 1X OVERSAMPLING */
#define BMP280_TEMPERATURE_17B			2	/* 2X OVERSAMPLING */
#define BMP280_TEMPERATURE_18B			3	/* 4X OVERSAMPLING */
#define BMP280_TEMPERATURE_19B			4	/* 8X OVERSAMPLING */
#define BMP280_TEMPERATURE_20B			5	/* 16X OVERSAMPLING */

/* BMP280 IIR FILTER COEFFICIENT */

#define BMP280_FILTER_0					0
#define BMP280_FILTER_2					1
#define BMP280_FILTER_4					2
#define BMP280_FILTER_8					3
#define BMP280_FILTER_16				4

/* BMP280 STANDBY TIME [MS] (STANDARD POWER MODE) */

#define BMP280_STNDBY_0_5				0
#define BMP280_STNDBY_62_5				1
#define BMP280_STNDBY_125				2
#define BMP280_STNDBY_250				3
#define BMP280_STNDBY_500				4
#define BMP280_STNDBY_1000				5
#define BMP280_STNDBY_2000				6
#define BMP280_STNDBY_4000				7

/* BMP280 COMPENSATION PARAMETERS REGISTERS */

#define	BMP280_DIG_T1					0x88
#define	BMP280_DIG_T2					0x8A
#define	BMP280_DIG_T3					0x8C

#define	BMP280_DIG_P1					0x8E
#define	BMP280_DIG_P2					0x90
#define	BMP280_DIG_P3					0x92
#define	BMP280_DIG_P4					0x94
#define	BMP280_DIG_P5					0x96
#define	BMP280_DIG_P6					0x98
#define	BMP280_DIG_P7					0x9A
#define	BMP280_DIG_P8					0x9C
#define	BMP280_DIG_P9					0x9E

/* BMP280 MEMORY MAP // REGISTERS */

#define BMP280_ID						0xD0
#define BMP280_SW_RESET					0xE0
#define BMP280_STATUS					0xF3
#define BMP280_CTRL_MEASUREMENTS		0xF4
#define BMP280_CONFIG					0xF5
#define BMP280_PRESSURE_DATA			0xF7
#define BMP280_TEMPERATURE_DATA			0xFA


//typedef enum
//{
//	BMP280_ULTRA_HIGH_RES   = 	5,
//	BMP280_HIGH_RES         = 	4,
//	BMP280_STANDARD         = 	3,
//	BMP280_LOW_POWER        = 	2,
//	BMP280_ULTRA_LOW_POWER  = 	1,
//	BMP280_OFF				=	0
//}BMP280_resolution;


//typedef enum
//{
//	BMP280_OK,
//	BMP280_Error,
//	BMP280_Disconnected,
//	BMP280_InvalidDevice
//}BMP280_result;


typedef struct
{
	uint32_t pressureRaw;
	uint32_t temperatureRaw;

	uint32_t pressure;
	int16_t temperature;

	float altitude;
}BMP280_data;


typedef struct
{
	I2C_HandleTypeDef *handler;
	uint8_t temperatureResolution;
	uint8_t pressureOversampling;
	uint8_t powerMode;

	uint8_t standbyTime;
	uint8_t iirFilter;

	/* calibration data */
	uint16_t t1;
	int16_t t2;
	int16_t t3;

	uint16_t p1;
	int16_t p2;
	int16_t p3;
	int16_t p4;
	int16_t p5;
	int16_t p6;
	int16_t p7;
	int16_t p8;
	int16_t p9;
}BMP280_configData;

/* Private functions */

static uint8_t BMP280_read8b(uint8_t);
static uint16_t BMP280_read16b(uint8_t);
static uint32_t BMP280_read24b(uint8_t);
static void BMP280_write8b(uint8_t, uint8_t);

void BMP280_init(I2C_HandleTypeDef*, uint8_t, uint8_t, uint8_t);
static void BMP280_setConfig(uint8_t, uint8_t);
static void BMP280_getCalibrationData();
static void BMP280_getRawPressureData();
static void BMP280_getRawTemperatureData();
void BMP280_readTemperature();
void BMP280_readPressure();
void BMP280_readAltitude();
