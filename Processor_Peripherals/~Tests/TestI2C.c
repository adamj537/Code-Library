/******************************************************************************
 *
 *	Filename:		Test.c
 *
 *	Author:			Adam Johnson
 *
 *	Description:	This file contains tests for a code module.  Having this
 *					ensures that as the module is ported or upgraded, is still
 *					has the same usage and performs correctly.
 *
 *****************************************************************************/

#ifdef INCLUDE_TEST

#include "I2C.h"
#include "TestI2C.h"

#define I2C_ADDR	0x50	// I2C address of EEPROM 24C02 (see datasheet)
#define I2C_SPEED	100000	// 10 kHz

i2cResult_t TestI2C(i2cChannel_t channel, uint8_t repeatFlag)
{
	i2cResult_t result = I2C_RESULT_OK;	// an optimistic return value :)
	i2cConfig_t config;					// module configuration data
	volatile uint8_t response[16];		// bytes received from EEPROM

	do	// I'm afraid of goto statements, so I'll use this instead ;}
	{
		// Setup the I2C configuration.
		config.mode = I2C_MODE_MASTER;
		config.speed = I2C_SPEED;
		config.address = I2C_ADDR;
		config.generalCallEnable = FALSE;

		// Setup the I2C driver with the setup configuration.
		result = I2CInit(channel, &config);
		if (result != I2C_RESULT_OK)
			break;

		// Enable the I2C peripheral.
		result = I2CEnable(channel);
		if (result != I2C_RESULT_OK)
			break;

		do	// This part can (optionally) be repeated.
		{
			// Try a read.
			result = I2CRead(channel, (uint8_t *)response, sizeof(response));
			if (result != I2C_RESULT_OK)
				break;

			// Wait until the device is not busy from the previous operation.
			while (I2CIsBusy(channel));

			// Try a write.
			result = I2CWrite(channel, (uint8_t *)writeMsg, sizeof(writeMsg));
			if (result != I2C_RESULT_OK)
				break;

			// Wait until the device is not busy from the previous operation.
			while (I2CIsBusy(channel));

			// Wait > 5 ms for the EEPROM to finish the write cycle (see
			// datasheet).  MCLK = 1048576 Hz (by default), so:
			// MCLK (1/s) * 0.005 s = 5242.88.  Round up to 5250, and wait that
			// many cycles.
//			__delay_cycles(5250);

			// Try a write-then-read.
			result = I2CWriteThenRead(channel, (uint8_t *)readMsg,
				sizeof(readMsg), (uint8_t *)response, sizeof(response));
			if (result != I2C_RESULT_OK)
				break;

			// Wait until the device is not busy from the previous operation.
			while (I2CIsBusy(channel));

		} while (repeatFlag);			// Repeat if requested.
	} while (FALSE);					// Don't repeat the initialization.

	return result;
}

#define EE_ADDR		0x05	// memory location in EEPROM to store data

/******************************************************************************
 *
 *	Function:		TestI2CLowLevel
 *
 *	Description:	This example shows how the I2C library can be used to
 *					access a serial EEPROM (24C02).  Based on Atmel Application
 *					Note AVR300, adapted to AVR-GCC C interface.  Uses a port
 *					of eight active-low LEDs to show result.
 *
 *					The test writes 0x75 to EEPROM address 0x05, then reads it
 *					back.  Then it writes data to EEPROM address 0x00 to 0x03,
 *					then reads it back.  Shows data on LEDs.
 *
 *****************************************************************************/
 
uint8_t TestI2CLowLevel(void)
{
	uint8_t data;						// data from EEPROM
	uint8_t error = 0;					// an optimistic return value :)

	DDRB  = 0xFF;						// Set all port B pins to output.
	PORTB = 0xFF;						// Turn off all (active low) LED's.

	do	// I'm afraid of goto statements, so I'll use this instead ;}
	{
		I2CInit();								// Initialize I2C clock.
		
		// Write 0x75 to EEPROM address 0x05.
		error = I2CStart(I2C_ADDR + I2C_WRITE);	// Send START and write address.
		if (error) break;						// Check for ACK.
		error = I2CWrite(EE_ADDR);				// Send EEPROM data address.
		if (error) break;						// Check for ACK.
		error = I2CWrite(0x75);					// Send data.
		if (error) break;						// Check for ACK.
		I2CStop();								// Send STOP, release the bus.
		
		// Read value back from EEPROM address 0x05. Wait until the device is
		// no longer busy from the previous write operation.
		I2CStartWait(I2C_ADDR + I2C_WRITE);		// Send START & write address; wait.
		error = I2CWrite(EE_ADDR);				// Send EEPROM data address.
		if (error) break;						// Check for ACK.
		error = I2CStart(I2C_ADDR + I2C_READ);	// Send START and read address.
		if (error) break;						// Check for ACK.
		data = I2CRead(0);						// Read one byte; send NACK.
		I2CStop();								// Send STOP, release the bus.

		PORTB = ~data;							// Output received byte on LEDs.
		
		// Write 0x70,0x71,072,073 to EEPROM address 0x00..0x03. Wait until the
		// device is no longer busy from the previous write operation.
		I2CStartWait(I2C_ADDR + I2C_WRITE);		// Send START & write address; wait.
		error = I2CWrite(0x00);					// Send EEPROM data address 0.
		if (error) break;						// Check for ACK.
		error = I2CWrite(0x70);					// Write data to address 0.
		if (error) break;						// Check for ACK.
		error = I2CWrite(0x71);					// Write data to address 1.
		if (error) break;						// Check for ACK.
		error = I2CWrite(0x72);					// Write data to address 2.
		if (error) break;						// Check for ACK.
		error = I2CWrite(0x73);					// Write data to address 3.
		if (error) break;						// Check for ACK.
		I2CStop();								// Send STOP, release the bus.
		
		// Read value back from EEPROM address 0..3. Wait until the device is
		// no longer busy from the previous write operation.
		I2CStartWait(I2C_ADDR + I2C_WRITE);		// Send START & write address; wait.
		error = I2CWrite(0x00);					// Send EEPROM data address 0.
		if (error) break;						// Check for ACK.
		error = I2CStart(I2C_ADDR + I2C_READ);	// Send START and read address.
		if (error) break;						// Check for ACK.
		data = I2CRead(ACK);					// Read one byte from address 0.
		data = I2CRead(ACK);					// Read one byte from address 1.
		data = I2CRead(ACK);					// Read one byte from address 2.
		data = I2CRead(NACK);					// Read one byte from address 3.
		I2CStop();								// Send STOP, release the bus.

		PORTB = ~data;							// Output received byte on LEDs.
	} while (0);
	
	// Handle errors by releasing the bus and lighting all LEDs.
	if (error)
	{
		I2CStop();						// Send STOP and release the bus.
		PORTB = 0x00;					// Activate all LEDs to show error.
	}
	
	return error;
}

#endif