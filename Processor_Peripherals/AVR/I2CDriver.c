/******************************************************************************
 *
 *	Filename:		I2Cmaster.c
 *
 *	Author:			Adam Johnson (sort of)
 *
 *	Source:			Peter Fleury <pfleury@gmx.ch>  http://jump.to/fleury
 *
 *	Terms of Use:	MIT License
 *
 *	Description:	I2C master library using hardware TWI interface; written
 *					for any AVR device with hardware TWI, and compilable with
 *					AVR-GCC and avr-libc.
 *
 *	TODO:	- add support for multiple channels (I2C_0, I2C_1, etc.)
 *	TODO:	- add support for multiple modes (master, slave, multi-master, etc.)
 *
 *	Notes:
 *
 *		I2C is wired like this.  1k, 4k, 10k ohm are common values for pullups.
 *
 *		      +5V
 *		     |   |    __________     __________     __________
 *		   Rp[] []Rp |          |   |          |   |          |
 *		     |   |   | Device 1 |   | Device 2 |   | Device 3 |
 *		     |   |   |__________|   |__________|   |__________|
 *		     |   |        | |            | |            | |
 *		SCL--+---}--------+-}------------+-}------------+-}--------
 *		SDA------+----------+--------------+--------------+--------
 *
 *		There are two special sequences defined for the I2C bus.  They are the
 *		only places where the SDA (data line) is allowed to change while the
 *		SCL (clock line) is high.  The start and stop sequences mark the
 *		beginning and end of a transaction with the slave device.
 *
 *				Start sequence:			Stop sequence:
 *
 *				SDA---+        			           +---
 *				      |        			           |
 *				      +--------			 SDA-------+
 *
 *				SCL-------+    			       +--------
 *				          |    			       |
 *				          +----			 SCL---+
 *
 *****************************************************************************/

#include <inttypes.h>					// compiler-specific data types
#include <compat/twi.h>
#include <avr/io.h>
#include "I2Cmaster.h"					// header for this module

#if (__GNUC__ * 100 + __GNUC_MINOR__) < 304
#error "This library requires AVR-GCC 3.4 or later, update to newer AVR-GCC!"
#endif

#ifndef F_CPU
#define F_CPU		4000000UL			// CPU frequency in Hz
#endif

#define SCL_CLOCK	100000L				// I2C clock in Hz

/*************************************************************************
 Initialization of the I2C bus interface. Needs to be called only once.
*************************************************************************/
void I2CInit(void)
{
	// Set prescaler = 1.
	TWSR = 0;
	
	// Initialize TWI clock; must be > 10 for stable operation.
	TWBR = ((F_CPU/SCL_CLOCK)-16) / 2;
}/* I2CInit */


/*************************************************************************
  Issues a start condition and sends address and transfer direction.
  return 0 = device accessible, 1= failed to access device
*************************************************************************/
uint8_t I2CStart(uint8_t address)
{
	uint8_t status;

	// Send START condition.
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

	// Wait until transmission completed.
	while (!(TWCR & (1<<TWINT)));

	// Check value of TWI status register (but mask prescaler bits).
	status = TW_STATUS & 0xF8;
	if ((status != TW_START) && (status != TW_REP_START))
		return 1;

	// Send device address.
	TWDR = address;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// Wait until transmission completed and ACK/NACK has been received.
	while (!(TWCR & (1<<TWINT)));

	// Check value of TWI status register (but mask prescaler bits).
	status = TW_STATUS & 0xF8;
	if ((status != TW_MT_SLA_ACK) && (status != TW_MR_SLA_ACK))
		return 1;

	return 0;

}/* I2CStart */

/*************************************************************************
 Issues a start condition and sends address and transfer direction.
 If device is busy, use ack polling to wait until device is ready
 
 Input:   address and transfer direction of I2C device
*************************************************************************/
void I2CStartWait(uint8_t address)
{
	uint8_t status;

	while (1)
	{
		// Send START condition.
		TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

		// Wait until transmission completed.
		while(!(TWCR & (1<<TWINT)));

		// Check value of TWI status register (but mask prescaler bits).
		status = TW_STATUS & 0xF8;
		if ((status != TW_START) && (status != TW_REP_START))
			continue;

		// Send device address.
		TWDR = address;
		TWCR = (1<<TWINT) | (1<<TWEN);

		// Wait until transmission completed.
		while(!(TWCR & (1<<TWINT)));

		// Check value of TWI status register (but mask prescaler bits).
		status = TW_STATUS & 0xF8;
		if ((status == TW_MT_SLA_NACK ) || (status == TW_MR_DATA_NACK))
		{
			// Device busy, send stop condition to terminate write operation.
			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
			
			// Wait until stop condition is executed and bus released.
			while(TWCR & (1<<TWSTO));
			
			continue;
		}
		// if( status != TW_MT_SLA_ACK) return 1;
		break;
	 }

}/* I2CStartWait */

/*************************************************************************
 Terminates the data transfer and releases the I2C bus
*************************************************************************/
void I2CStop(void)
{
	// Send stop condition.
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	
	// Wait until stop condition is executed and bus released.
	while (TWCR & (1<<TWSTO));

}/* I2CStop */


/*************************************************************************
  Send one byte to I2C device
  
  Input:    byte to be transferred
  Return:   0 write successful 
            1 write failed
*************************************************************************/
uint8_t I2CWrite(uint8_t data)
{	
	uint8_t status;
    
	// Send data to the previously addressed device.
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// Wait until transmission completed.
	while (!(TWCR & (1<<TWINT)));

	// Check value of TWI status register (but mask prescaler bits).
	status = TW_STATUS & 0xF8;
	if (status != TW_MT_DATA_ACK)
	{
		return 1;
	}
	return 0;

}/* i2c_write */


/*************************************************************************
 Read one byte from the I2C device.  Then either request more data from the
 device, or send a stop condition (depending on the ack parameter).
 
 Return:  byte read from I2C device
*************************************************************************/
uint8_t I2CRead(i2cAck_t response);
{
	if (response == ACK)
	{
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	}
	else
	{
		TWCR = (1<<TWINT) | (1<<TWEN);
	}
	while (!(TWCR & (1<<TWINT)));    

    return TWDR;

}/* I2CRead */
