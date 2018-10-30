/**
 * @brief	Driver for the Adafruit SPI FRAM breakout.
 * @remarks	Based on Adafruit driver.
 * @license	Original Adafruit driver had BSD license.
*/

#ifndef MB85RS64_H
#define MB85RS64_H

bool FramCheck(void);
void FramSetAddressSize(uint8_t bytes);
void FramWriteEnable(bool enable);
void FramWriteByte(uint32_t addr, uint8_t value);
void FramWrite(uint32_t addr, const uint8_t *values, size_t count);
uint8_t FramReadByte(uint32_t addr);
void FramRead(uint32_t addr, uint8_t *values, size_t count);
void FramGetDeviceID(uint8_t *manufacturerID, uint16_t *productID);
uint8_t FramGetStatusRegister(void);
void FramSetStatusRegister(uint8_t value);

#endif
