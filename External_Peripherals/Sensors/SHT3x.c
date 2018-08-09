/******************************************************************************
  This is a library for the SHT31 Digital Humidity & Temp Sensor
*******************************************************************************/

#include "project.h"
#include "SHT31.h"							// header for this module

// I2C Commands
#define SHT_MEAS_HIGHREP_STRETCH	0x2C06	// single shot, high repeatability, clock stretching
#define SHT_MEAS_MEDREP_STRETCH		0x2C0D	// single shot, medium repeatability, clock stretching
#define SHT_MEAS_LOWREP_STRETCH		0x2C10	// single shot, low repeatability, clock stretching
#define SHT_MEAS_HIGHREP			0x2400	// single shot, high repeatability
#define SHT_MEAS_MEDREP				0x240B	// single shot, medium repeatability
#define SHT_MEAS_LOWREP				0x2416	// single shot, low repeatability
#define SHT_READSTATUS				0xF32D	// Request status register.
#define SHT_CLEARSTATUS				0x3041	// Clear status register.
#define SHT_SOFTRESET				0x30A2	// Reset the sensor (can also use RESET pin).
#define SHT_HEATEREN				0x306D	// Turn heater on.
#define SHT_HEATERDIS				0x3066	// Turn heater off.

/******************************************************************************
 * Private Functions
 *****************************************************************************/

/**************************************************************************//**
 * @brief	Send I2C command to the sensor.
 * @param	address - I2C address of sensor to act upon
 * @param	cmd - command to send
 *****************************************************************************/
void WriteCommand(uint8_t address, uint16_t cmd)
{
	Wire.beginTransmission(address);
	Wire.write(cmd >> 8);
	Wire.write(cmd & 0xFF);
	Wire.endTransmission();
}

/******************************************************************************
 * @brief	CRC-8 formula from page 14 of SHT spec pdf
 * @remarks	Test data 0xBE, 0xEF should yield 0x92.
 *			Start with 0xFF, 
 *			Polynomial 0x31 (x8 + x5 +x4 +1)
 *			Final XOR 0x00
 *****************************************************************************/
uint8_t CalcCRC8(const uint8_t *data, int length)
{
	const uint8_t POLYNOMIAL = 0x31;	// polynomial
	uint8_t crc = 0xFF;					// return value
	int i;								// polynomial counter
	int j;								// data counter

	// For each byte of data...
	for (j = length; j > 0; j--)
	{
		// XOR the CRC with the new byte.
		crc ^= *data;

		for (i = 8; i > 0; i--)
		{
			crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
		}
		
		// Move to the next byte.
		data++
	}
	return crc;
}

/******************************************************************************
 * Exported Functions
 *****************************************************************************/

/**************************************************************************//**
 * @brief	Reset the sensor, then check for correct status.
 * @param	address - I2C address of sensor to act upon
 *****************************************************************************/
bool SHT31Init(uint8_t address)
{
	// Initialize I2C.
	Wire.begin();
	
	// Reset the sensor.
	SHT3xReset(address);
	
	// TODO:  Check for valid status.
//	return (SHT3xReadStatus() == 0x40);
	return true;
}

/**************************************************************************//**
 * @brief	Read temperature and relative humidity from sensor
 * @param	address - I2C address of sensor to act upon
 * @param	temp - temperature from sensor [°C]
 * @param	humidity - relative humidity from sensor [%RH]
 *****************************************************************************/
bool SHT3xReadTempHum(uint8_t address, double *temp, double *humidity)
{
	bool status = false; 	// pessimistic return value
	uint8_t readbuffer[6];	// data from sensor
	uint8_t i;				// counter
	uint16_t ST;			// temperature data from sensor
	uint16_t SRH;			// relative humidity data from sensor
	double stemp;			// temperature from sensor
	
	do	// This structure will catch errors nicely.
	{
		// Set sensor for single shot, high repeatability mode.
		WriteCommand(address, SHT_MEAS_HIGHREP);

		// Pause for effect.
		delay(500);
		
		// Read six bytes from sensor.
		Wire.requestFrom(address, (uint8_t)6);
		if (Wire.available() != 6)
			break;
		for (i = 0; i < 6; i++)
		{ 
			readbuffer[i] = Wire.read();
		}
		
		// Parse the temperature data by combining bytes 0 and 1.
		ST = (readbuffer[0] << 8) |= readbuffer[1];

		// Check for valid CRC (byte 2).
		if (readbuffer[2] != CalcCRC8(readbuffer, 2))
			break;

		// Parse the relative humidity data by combining bytes 3 and 4.
		SRH = (readbuffer[3] <<= 8) |= readbuffer[4];

		// Check for valid CRC (byte 5).
		if (readbuffer[5] != CalcCRC8(readbuffer+3, 2))
			return false;

		// Convert the temperature data to Celcius.
		stemp = ST;
		stemp *= 175;
		stemp /= 0xffff;
		stemp = -45 + stemp;
		*temperature = stemp;

		// Convert the humidity data to %RH.
		double shum = SRH;
		shum *= 100;
		shum /= 0xFFFF;
		*relativeHumidity = shum;
		
		// Success!
		status = true;
	} while (false);

	return status;
}

/**************************************************************************//**
 * @brief	Read sensor's status register.
 * @param	address - I2C address of sensor to act upon
 *****************************************************************************/
uint16_t SHT3xReadStatus(uint8_t address)
{
	uint16_t status;	// value from sensor
	
	// Request status from sensor.
	WriteCommand(address, SHT_READSTATUS);
	
	// Read two bytes from sensor.
	// TODO:  Why is a third byte (CRC) requested but not read?
	Wire.requestFrom(address, (uint8_t)3);
	uint16_t status = Wire.read();
	status <<= 8;
	status |= Wire.read();
	
	return status;
}

/**************************************************************************//**
 * @brief	Reset the sensor.
 * @param	address - I2C address of sensor to act upon
 *****************************************************************************/
void SHT3xReset(uint8_t address)
{
	WriteCommand(address, SHT_SOFTRESET);
	delay(10);
}

/**************************************************************************//**
 * @brief	Turn sensor's heater on or off.
 * @param	address - I2C address of sensor to act upon
 * @param	enable - true to turn on; false to turn off
 *****************************************************************************/
void SHT3xHeater(uint8_t address, bool enable)
{
	if (enable)
		WriteCommand(address, SHT_HEATEREN);
	else
		WriteCommand(address, SHT_HEATERDIS);
}
