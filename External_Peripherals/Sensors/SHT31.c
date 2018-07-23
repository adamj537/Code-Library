/******************************************************************************
  This is a library for the SHT31 Digital Humidity & Temp Sensor
*******************************************************************************/

#include "SHT31.h"

#define SHT31_MEAS_HIGHREP_STRETCH	0x2C06
#define SHT31_MEAS_MEDREP_STRETCH	0x2C0D
#define SHT31_MEAS_LOWREP_STRETCH	0x2C10
#define SHT31_MEAS_HIGHREP			0x2400
#define SHT31_MEAS_MEDREP			0x240B
#define SHT31_MEAS_LOWREP			0x2416
#define SHT31_READSTATUS			0xF32D
#define SHT31_CLEARSTATUS			0x3041
#define SHT31_SOFTRESET				0x30A2
#define SHT31_HEATEREN				0x306D
#define SHT31_HEATERDIS				0x3066

// Global variables
uint8_t _i2caddr;
float humidity;
float temp;

/******************************************************************************
 * Private Functions
 *****************************************************************************/

bool ReadTempHum(void)
{
	uint8_t readbuffer[6];
	uint8_t i;				// counter
	
	WriteCommand(SHT31_MEAS_HIGHREP);

	delay(500);
	Wire.requestFrom(_i2caddr, (uint8_t)6);
	if (Wire.available() != 6)
		return false;
	
	for (i=0; i<6; i++)
	{
		readbuffer[i] = Wire.read();
//		Serial.print("0x"); Serial.println(readbuffer[i], HEX);
	}
	uint16_t ST;
	uint16_t SRH;
	ST = readbuffer[0];
	ST <<= 8;
	ST |= readbuffer[1];

	if (readbuffer[2] != CalcCRC8(readbuffer, 2))
		return false;

	SRH = readbuffer[3];
	SRH <<= 8;
	SRH |= readbuffer[4];

	if (readbuffer[5] != CalcCRC8(readbuffer+3, 2))
		return false;

	// Serial.print("ST = "); Serial.println(ST);
	double stemp = ST;
	stemp *= 175;
	stemp /= 0xffff;
	stemp = -45 + stemp;
	temp = stemp;

	//  Serial.print("SRH = "); Serial.println(SRH);
	double shum = SRH;
	shum *= 100;
	shum /= 0xFFFF;

	humidity = shum;

	return true;
}

void WriteCommand(uint16_t cmd)
{
	Wire.beginTransmission(_i2caddr);
	Wire.write(cmd >> 8);
	Wire.write(cmd & 0xFF);
	Wire.endTransmission();
}

/**
 * CRC-8 formula from page 14 of SHT spec pdf
 *
 * Test data 0xBE, 0xEF should yield 0x92
 *
 * Initialization data 0xFF
 * Polynomial 0x31 (x8 + x5 +x4 +1)
 * Final XOR 0x00
 */
uint8_t CalcCRC8(const uint8_t *data, int len)
{
	const uint8_t POLYNOMIAL(0x31);
	uint8_t crc(0xFF);

	for (int j = len; j; --j)
	{
		crc ^= *data++;

		for ( int i = 8; i; --i )
		{
			crc = ( crc & 0x80 )
			? (crc << 1) ^ POLYNOMIAL : (crc << 1);
		}
	}
	return crc;
}

/******************************************************************************
 * Exported Functions
 *****************************************************************************/

bool SHT31Init(uint8_t i2caddr)
{
	// Initialize I2C.
	Wire.begin();
	
	// Remember the sensor's I2C address.
	_i2caddr = i2caddr;
	
	// Reset the sensor.
	SHT3xReset();
	
//	return (SHT3xReadStatus() == 0x40);
	return true;
}

uint16_t SHT3xReadStatus(void)
{
	WriteCommand(SHT31_READSTATUS);
	Wire.requestFrom(_i2caddr, (uint8_t)3);
	uint16_t stat = Wire.read();
	stat <<= 8;
	stat |= Wire.read();
	//Serial.println(stat, HEX);
	return stat;
}

void SHT3xReset(void)
{
	WriteCommand(SHT31_SOFTRESET);
	delay(10);
}

void SHT3xHeater(bool enable)
{
	if (enable)
		WriteCommand(SHT31_HEATEREN);
	else
		WriteCommand(SHT31_HEATERDIS);
}

float SHT3xReadTemperature(void)
{
	if (! ReadTempHum())
	{
		return NAN;
	}
	return temp;
}

float SHT3xReadHumidity(void)
{
	if (! ReadTempHum())
	{
		return NAN;
	}

	return humidity;
}

/*********************************************************************/
