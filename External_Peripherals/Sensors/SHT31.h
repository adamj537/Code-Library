/******************************************************************************
  This is a library for the SHT31 Digital Humidity & Temp Sensor
*******************************************************************************/

#define SHT31_ADDR_LOW		0x44		// I2C address (when ADDR pin is LOW)
#define SHT31_ADDR_HIGH		0x45		// I2C address (when ADDR pin is HIGH)

bool     SHT31Init(uint8_t i2caddr = SHT31_DEFAULT_ADDR);
float    SHT3xReadTemperature(void);
float    SHT3xReadHumidity(void);
uint16_t SHT3xReadStatus(void);
void     SHT3xReset(void);
void     SHT3xHeater(bool enable);
