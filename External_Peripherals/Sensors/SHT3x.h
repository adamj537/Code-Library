/******************************************************************************
  This is a library for the SHT31 Digital Humidity & Temp Sensor
*******************************************************************************/

#define SHT_ADDR_LOW		0x44		// I2C address (when ADDR pin is LOW)
#define SHT_ADDR_HIGH		0x45		// I2C address (when ADDR pin is HIGH)

bool     SHT31Init(uint8_t address);
bool     SHT3xReadTempHum(uint8_t address, double *temp, double *humidity);
uint16_t SHT3xReadStatus(uint8_t address);
void     SHT3xReset(uint8_t address);
void     SHT3xHeater(uint8_t address, bool enable);
