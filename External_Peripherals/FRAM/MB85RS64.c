/**************************************************************************//**
 * @brief	Driver for the Adafruit SPI FRAM breakout.
 * @remarks	Based on Adafruit driver.
 * @license	Original Adafruit driver had BSD license.
 *****************************************************************************/

#include "st_spi.h"						// SPI driver
#include "MB85RS64.h"					// header for this module

typedef enum							// FRAM chip commands
{
	OPCODE_WREN   = 0x06,				// Write Enable Latch
	OPCODE_WRDI   = 0x04,				// Reset Write Enable Latch
	OPCODE_RDSR   = 0x05,				// Read Status Register
	OPCODE_WRSR   = 0x01,				// Write Status Register
	OPCODE_READ   = 0x03,				// Read Memory
	OPCODE_WRITE  = 0x02,				// Write Memory
	OPCODE_RDID   = 0x9F				// Read Device ID
} opcodes_t;

uint8_t _addressSize;					// byte width of FRAM chip's address

/**************************************************************************//**
 * @brief	Tell the FRAM chip what address we wish to act upon
 * @param	addr - address in memory to act upon
 *****************************************************************************/
static void WriteAddress(uint32_t addr)
{
	if (_addressSize > 3)
		SPItransfer((uint8_t)(addr >> 24));
	
	if (_addressSize > 2)
		SPItransfer((uint8_t)(addr >> 16));
	
	SPItransfer((uint8_t)(addr >> 8));
	SPItransfer((uint8_t)(addr & 0xFF));
}

/**************************************************************************//**
 * @brief	Remember how many bytes to send for the FRAM's address.
 * @remarks	Call this before calling anything else in this library.
 * 			The addressSize parameter is used because different parts might
 * 			have different address sizes, so the library can be used with them.
 * @param	bytes - width of FRAM chip's address [bytes]
 *****************************************************************************/
void FramSetAddressSize(uint8_t bytes)
{
	// Remember the address size.
	_addressSize = bytes;
}

/**************************************************************************//**
 * @brief	Test whether the FRAM chip is connected.
 * @param	addressSize - size of address [bytes]
 * @returns	true for success; false otherwise
 *****************************************************************************/
bool FramCheck(void)
{
	bool result = true;					//  return value
	uint8_t manufID;					// manufacturer ID
	uint16_t prodID;					// product ID
	
	// Fetch the device ID to see if we're actually connected.
	FramGetDeviceID(&manufID, &prodID);

	// If either ID is incorrect...
	if (manufID != 0x04 && manufID != 0x7f)
	{
		result = false;
	}
	if (prodID != 0x0302 && prodID != 0x7f7f)
	{
		result = false;
	}

	return result;
}

/**************************************************************************//**
 * @brief	Enable writing to SPI flash
 * @param	enable - true to enable; false to disable
 *****************************************************************************/
void FramWriteEnable(bool enable)
{
	digitalWrite(_cs, LOW);				// Enable chip select.
	
	if (enable)
	{
		SPItransfer(OPCODE_WREN);		// Send ENABLE WRITE command.
	}
	else
	{
		SPItransfer(OPCODE_WRDI);		// Send DISABLE WRITE command.
	}
	
	digitalWrite(_cs, HIGH);			// Disable chip select.
}

/**************************************************************************//**
 * @brief	Write a byte at the specific FRAM address
 * @param	addr - 32-bit address to write to in FRAM memory
 * @param	value - 8-bit value to write to memory
 *****************************************************************************/
void FramWriteByte(uint32_t addr, uint8_t value)
{
	digitalWrite(_cs, LOW);				// Enable chip select.
	
	SPItransfer(OPCODE_WRITE);			// Send WRITE command.
	WriteAddress(addr);					// Send the address.
	SPItransfer(value);					// Send the value.
	
	// CS on the rising edge commits the WRITE.
	digitalWrite(_cs, HIGH);			// Disable chip select.
}

/**************************************************************************//**
 * @brief	Write array of data to FRAM.
 * @param	addr - address to act upon
 * @param	values - array of data to write
 * @param	count - number of bytes to write
 *****************************************************************************/
void FramWrite(uint32_t addr, const uint8_t *values, size_t count)
{
	digitalWrite(_cs, LOW);				// Enable chip select.
	
	SPItransfer(OPCODE_WRITE);			// Send the WRITE command.
	WriteAddress(addr);					// Send the address.
	for (size_t i = 0; i < count; i++)	// For each byte...
	{
		SPItransfer(values[i]);			// Send the byte.
	}
	
	// CS on the rising edge commits the WRITE.
	digitalWrite(_cs, HIGH);			// Disable chip select.
}

/**************************************************************************//**
 * @brief	Read a byte from FRAM
 * @param	addr - address to act upon
 * @returns	value read from FRAM
 *****************************************************************************/
uint8_t FramReadByte(uint32_t addr)
{
	uint8_t value;						// return value
	
	digitalWrite(_cs, LOW);				// Enable chip select.

	SPItransfer(OPCODE_READ);			// Send READ command.
	WriteAddress(addr);					// Send address to read from.
	value = SPItransfer(0);				// Read a byte (send dummy data).

	digitalWrite(_cs, HIGH);			// Disable chip select.

	return value;
}

/**************************************************************************//**
 * @brief	Read array of bytes from FRAM
 * @param	addr - address to act upon
 * @param	values - pointer to array of data read from FRAM
 * @param	count - number of bytes to read
 *****************************************************************************/
void FramRead(uint32_t addr, uint8_t *values, size_t count)
{
	digitalWrite(_cs, LOW);				// Enable chip select.

	SPItransfer(OPCODE_READ);			// Send READ command.
	WriteAddress(addr);					// Send address to start reading from.
	for (size_t i = 0; i < count; i++)	// For each byte...
	{
		values[i] = SPItransfer(0);		// Read a byte.
	}

	digitalWrite(_cs, HIGH);			// Disable chip select.
}

/**************************************************************************//**
 * @brief	Read manufacturer ID and Product ID from FRAM
 * @param	manufacturerID - 8-bit manufacturer ID (Fujitsu = 0x04)
 * @param	productID - memory density (bits 15 - 8) and proprietary product ID
 * 				fields (bits 7 - 0); should be 0x0302 for MB85RS64VPNF-G-JNERE1.
 *****************************************************************************/
void FramGetDeviceID(uint8_t *manufacturerID, uint16_t *productID)
{
	uint8_t a[4] = { 0, 0, 0, 0 };

	digitalWrite(_cs, LOW);				// Enable chip select.
	
	SPItransfer(OPCODE_RDID);			// Send REQUEST DEVICE ID command.
	a[0] = SPItransfer(0);				// Read four bytes.
	a[1] = SPItransfer(0);
	a[2] = SPItransfer(0);
	a[3] = SPItransfer(0);
	
	digitalWrite(_cs, HIGH);			// Disable chip select.

	// Shift values to separate manufacturer and product IDs.
	// See datasheet page 10.
	*manufacturerID = (a[0]);
	*productID = (a[2] << 8) + a[3];
}

/**************************************************************************//**
 * @brief	Reads the status register
 * @returns	value of status register
 *****************************************************************************/
uint8_t FramGetStatusRegister(void)
{
	uint8_t reg = 0;					// return value

	digitalWrite(_cs, LOW);				// Enable chip select.

	SPItransfer(OPCODE_RDSR);			// Send READ STATUS REGISTER command.
	reg = SPItransfer(0);				// Read a byte.

	digitalWrite(_cs, HIGH);			// Disable chip select.

	return reg;
}

/**************************************************************************//**
 * @brief	Sets the status register
 *****************************************************************************/
void FramSetStatusRegister(uint8_t value)
{
	digitalWrite(_cs, LOW);				// Enable chip select.

	SPItransfer(OPCODE_WRSR);			// Send WRITE STATUS REGISTER command.
	SPItransfer(value);					// Write a byte.

	digitalWrite(_cs, HIGH);			// Disable chip select.
}
