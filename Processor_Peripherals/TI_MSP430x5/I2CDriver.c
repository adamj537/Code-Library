/******************************************************************************
 *
 *	Filename:		I2C.c (for TI's MSP430)
 *
 *	Author:			Adam Johnson
 *
 *	Description:	This is an I2C master transmitter module which uses
 *					interrupts.
 *
 *	Notes:		(1) If you choose to use USCIB0, the UCB0SDA and UCB0SCL lines
 *					are not assigned to a dedicated port and need to be
 *					assigned to PMAP ports.
 *
 *				(2) External pull-ups are required...the GPIO's internal
 *					pull-ups don't appear usable when in I2C mode.  Here's the
 *					schematic for the test function:
 *
 *					                     /|\  /|\
 *					     MSP430F665x     10k  10k       EEPROM
 *					       master         |    |         24C02
 *					  -----------------   |    |   -----------------
 *					-|XIN  P2.0/UCB0SDA|<-|----+->|SDA              |
 *					 |                 |  |       |                 |
 *					-|XOUT             |  |       |                 |
 *					 |     P2.1/UCB0SCL|<-+------>|SCL              |
 *					 |                 |          |                 |
 *
 *				(3) The acts of (1) reading from UCBxRXBUF, (2) setting UCTXSTT
 *					bit, and (2) setting UCTXSTP bit all cause new data to be
 *					clocked into UCBxRXBUF.  So before getting the last byte in
 *					a receive transaction, we disable the RX interrupt, do both
 *					actions, and then enable it again.  Otherwise we will lose
 *					the 2nd-to-last byte of data in the transaction.  We also
 *					need to do something similar when receiving only one byte.
 *
 *	Origin:			github.com/adamj537/Code-Library
 *
 *	Terms of Use:	MIT License
 *
 *****************************************************************************/

#include <msp430.h>						// required by development platform
#include "Datatypes.h"					// compiler-specific data types
#include "MSP430_I2C.h"					// header for this module

// USCI registers - the numbers are offsets from a "base address" defined in
// the handleToBaseAddress array below.
#define I2CREG_CTLW0		0x0000		// Control Word
#define I2CREG_CTL1			0x0000		// control 1
#define I2CREG_CTL0			0x0001		// Control 0
#define I2CREG_BRW			0x0006		// Baud Rate Control Word
#define I2CREG_BR0			0x0006		// Baud Rate Control 0
#define I2CREG_BR1			0x0007		// Baud Rate Control 1
#define I2CREG_STAT			0x000A		// Status Register
#define I2CREG_RXBUF		0x000C		// Receive Buffer
#define I2CREG_TXBUF		0x000E		// Transmit Buffer
#define I2CREG_OA			0x0010		// I2C Own Address
#define I2CREG_SA			0x0012		// I2C Slave Address
#define I2CREG_ICTL			0x001C		// Interrupt Enable Register
#define I2CREG_IE			0x001C		// USCI Bx Interrupt Enable Register
#define I2CREG_IFG			0x001D		// USCI Bx Interrupt Flags Register
#define I2CREG_IV			0x001E		// Interrupt Vector Register

// Macros to help with register access.
#define REG16(x)			(*((volatile uint16_t *)((uint16_t)x)))
#define REG8(x)				(*((volatile uint8_t *)((uint16_t)x)))

// This array holds the address of the lowest register in each I2C peripheral.
// If we know that address and the "offset" of each register (defined above),
// then we can access any register of any I2C peripheral.
static const uint16_t baseAddr[] = {
#if defined(__MSP430_HAS_USCI_B0__)		// USCI B0
	__MSP430_BASEADDRESS_USCI_B0__,
#endif

#if defined(__MSP430_HAS_USCI_B1__)		// USCI B1
	__MSP430_BASEADDRESS_USCI_B1__,
#endif

#if defined(__MSP430_HAS_USCI_B2__)		// USCI B2
	__MSP430_BASEADDRESS_USCI_B2__,
#endif

#ifdef __MSP430_HAS_USCI_B3__			// USCI B3
	__MSP430_BASEADDRESS_USCI_B3__,
#endif
};

// Figure out how many channels we have available.
#define I2C_NUM_CHANNELS	(sizeof(baseAddr)/sizeof(baseAddr[0])

volatile uint8_t *txDataPtr[I2C_NUM_CHANNELS];	// pointer to TX data
volatile uint32_t txByteCtr[I2C_NUM_CHANNELS];	// number of bytes to TX

volatile uint8_t *rxDataPtr[I2C_NUM_CHANNELS];	// pointer to RX data
volatile uint32_t rxByteCtr[I2C_NUM_CHANNELS];	// number of bytes to RX

/******************************************************************************
 *
 *	Function:		I2CInit
 *
 *	Description:	Initializes the specified I2C with the provided settings.
 *
 *	Parameters:		uint8_t channel - which I2C peripheral to act upon
 *					i2cConfig_t *configPtr - settings to apply to the channel
 *
 *	Return Value:	I2C_RESULT_OK for success; other values indicate failure
 *
 *****************************************************************************/

i2cResult_t I2CInit(uint8_t channel, i2cConfig_t *configPtr)
{
	i2cResult_t result = I2C_RESULT_OK;	// an optimistic return value ;)
	uint16_t base = baseAddr[channel];	// Convert channel to base address.
	uint16_t preScalarValue;

	// Reset global variables in case we were stuck somewhere.
	txByteCtr[channel] = 0;
	rxByteCtr[channel] = 0;

	// Check for invalid channel.
	if (channel >= I2C_NUM_CHANNELS)
	{
		result = I2C_RESULT_INVALID_SELECTION;
	}

	// Configure in slave mode if requested.
	else if ((configPtr->mode == I2C_MODE_SLAVE) ||
			(configPtr->mode == I2C_MODE_SLAVE_AND_GEN_CALL))
	{
		// Disable the USCI module and clear the other bits of control register.
		REG8(base + I2CREG_CTL1) = UCSWRST;

		// Clear USCI master mode.
		REG8(base + I2CREG_CTL0) &= ~UCMST;

		// Configure I2C as Slave and Synchronous mode.
		REG8(base + I2CREG_CTL0) = UCMODE_3 | UCSYNC;

		// Set up the slave address.
		REG16(base + I2CREG_OA) = configPtr->address;

		// Enable general call if requested.
		if (configPtr->mode == I2C_MODE_SLAVE_AND_GEN_CALL)
		{
			REG16(base + I2CREG_OA) |= UCGCEN;
		}
		else
		{
			REG16(base + I2CREG_OA) &= ~UCGCEN;
		}
	}

	// Configure in master (or multi-master) mode if requested.
	else
	{
		// Disable the USCI module.
		REG8(base + I2CREG_CTL1) |= UCSWRST;

		// Configure as I2C master mode.
		// UCMST = Master mode; UCMODE_3 = I2C mode; UCSYNC = Synchronous mode
		REG8(base + I2CREG_CTL0) = UCMST | UCMODE_3 | UCSYNC;

		// If multi-master mode is selected, enable it.
		if (config->mode == I2C_MODE_MULTI_MASTER)
		{
			REG8(base + I2CREG_CTL0) |= UCMM;
		}
		else
		{
			REG8(base + I2CREG_CTL0) &= ~UCMM;
		}

		// Configure I2C clock source.
		REG8(base + I2CREG_CTL1) = (I2C_CLOCK_SOURCE + UCSWRST);

		// Compute the clock divider to give close to the desired speed.
		preScalarValue = (uint16_t)(I2C_CLOCK_FREQ_HZ / config->speed);
		REG16(base + I2CREG_BRW) = preScalarValue;

		// Set the address we'll attempt to talk to.
//		REG8(base + I2CREG_SA) = config->address;
	}

	return result;
}

/******************************************************************************
 *
 *	Function:		I2CEnable
 *
 *	Description:	Turns on the specified I2C channel.  This is separate from
 *					the init function so that the channel can be enabled and
 *					disabled without reconfiguring it.
 *
 *	Parameters:		uint8_t channel - which I2C peripheral to act upon
 *
 *	Return Value:	I2C_RESULT_OK for success; other values indicate failure
 *
 *****************************************************************************/

i2cResult_t I2CEnable(uint8_t channel)
{
	i2cResult_t result = I2C_RESULT_OK;	// an optimistic return value :)
	uint16_t base = baseAddr[channel];	// Convert channel to base address.

	// Check for invalid channel.
	if (channel >= I2C_NUM_CHANNELS)
	{
		result = I2C_RESULT_INVALID_SELECTION;
	}

	else
	{
		// Clear the UCSWRST bit to enable the I2C Module.
		REG8(base + I2CREG_CTL1) &= ~(UCSWRST);

		// Enable transmit, receive, and not-acknowledge interrupts.
		REG8(base + I2CREG_IE) |= UCTXIE | UCRXIE | UCNACKIE;
	}

	return result;
}

/******************************************************************************
 *
 *	Function:		I2CDisable
 *
 *	Description:	Turns off the specified I2C channel.  The configuration
 *					specified in the init function will be preserved, so you
 *					don't need to redo the init function after calling this.
 *
 *	Parameters:		uint8_t channel - which I2C peripheral to act upon
 *
 *	Return Value:	I2C_RESULT_OK for success; other values indicate failure
 *
 *****************************************************************************/

i2cResult_t I2CDisable(uint8_t channel)
{
	i2cResult_t result = I2C_RESULT_OK;	// an optimistic return value :)
	uint16_t base = baseAddr[channel];	// Convert channel to base address.

	// Check for invalid channel.
	if (channel >= I2C_NUM_CHANNELS)
	{
		result = I2C_RESULT_INVALID_SELECTION;
	}

	else
	{
		// Set the UCSWRST bit to disable the I2C Module.
		REG8(base + I2CREG_CTL1) |= UCSWRST;

		// Disable transmit, receive, and not-acknowledge interrupts.
		REG8(base + I2CREG_IE) &= ~(UCTXIE | UCRXIE | UCNACKIE);
	}

	return result;
}

/******************************************************************************
 *
 *	Function:		I2CIsBusy
 *
 *	Description:	This function indicates whether or not the I2C bus is busy.
 *					It checks the status of the UCBBUSY bit in UCBxSTAT
 *					register.  If you call I2CRead or I2CWrite while the bus is
 *					busy, then both I2C transactions will be clobbered.
 *
 *	Parameters:		uint8_t channel - which I2C peripheral to act upon
 *
 *	Return Value:	TRUE if the I2C Master is busy; FALSE otherwise.
 *
 *****************************************************************************/

uint8_t I2CIsBusy (uint8_t channel)
{
	uint16_t base = baseAddr[channel];	// Convert channel to base address.

    // Check the bus busy bit of the STAT register.
	if ((REG8(base + I2CREG_STAT) & UCBBUSY)	||
		(REG8(base + I2CREG_STAT) & UCSCLLOW)	||
		(REG8(base + I2CREG_CTL1) & UCTXSTP)	||
		(REG8(base + I2CREG_CTL1) & UCTXSTT)	||
		(REG8(base + I2CREG_IFG) & UCTXIFG)		||
		(REG8(base + I2CREG_IFG) & UCRXIFG))
	{
		return 1;
	}
	return 0;
}

/******************************************************************************
 *
 *	Function:		I2CWrite
 *
 *	Description:	Writes to an I2C slave device.
 *
 *	Parameters:		uint8_t channel - which I2C peripheral to act upon
 *					uint8_t address - I2C device address to use
 *					uint8_t *data - array of bytes to write to slave device
 *					uint32_t count - size of data array
 *
 *	Return Value:	I2C_RESULT_OK for success; other values indicate failure
 *
 *****************************************************************************/

i2cResult_t I2CWrite(uint8_t channel, uint8_t address, uint8_t *data, uint32_t count)
{
	i2cResult_t result = I2C_RESULT_OK;	// an optimistic return value :)
	uint16_t base = baseAddr[channel];	// Convert channel to base address.

	// Check for invalid channel.
	if (channel >= I2C_NUM_CHANNELS)
	{
		result = I2C_RESULT_INVALID_SELECTION;
	}

	else
	{
		// Set TX array start address.
		txDataPtr[channel] = data;

		// Load TX byte counter.
		txByteCtr[channel] = count;

		// Indicate that we're not reading.
		rxByteCtr[channel] = 0;

		// Set the address we'll attempt to talk to.
		REG8(base + I2CREG_SA) = address;

		// Send START + address + WRITE.
		REG8(base + I2CREG_CTL1) |= UCTXSTT | UCTR;

		// The rest of the message will be sent via ISR.
	}

	return result;
}

/******************************************************************************
 *
 *	Function:		I2CRead
 *
 *	Description:	Reads from an I2C slave device.
 *
 *	Parameters:		uint8_t channel - which I2C peripheral to act upon
 *					uint8_t address - I2C device address to use
 *					uint8_t *data - array to put data read from the device
 *					uint32_t count - size of data array
 *
 *	Return Value:	I2C_RESULT_OK for success; other values indicate failure
 *
 *****************************************************************************/

i2cResult_t I2CRead(uint8_t channel, uint8_t address, uint8_t *data, uint32_t count)
{
	i2cResult_t result = I2C_RESULT_OK;	// an optimistic return value :)
	uint16_t base = baseAddr[channel];	// Convert channel to base address.

	// Check for invalid channel.
	if (channel >= I2C_NUM_CHANNELS)
	{
		result = I2C_RESULT_INVALID_SELECTION;
	}

	else
	{
		// Set the address we'll attempt to talk to.
		REG8(base + I2CREG_SA) = address;

		// Indicate that we're not writing.
		txByteCtr[channel] = 0;

		// Set RX array start address.
		rxDataPtr[channel] = data;

		// The case when we send only one byte is strange because of the way
		// the MSP430's I2C hardware state machine works...you have to send
		// START and STOP before you allow the interrupt to fire to get the
		// received byte.  This is similar to the case of the last byte of a
		// receive transaction (see Note 3 above).
		if (count == 1)
		{
			rxByteCtr[channel] = 0;

			// Disable receive interrupt.
			REG8(base + I2CREG_IE) &= ~UCRXIE;

			// Send START + address + READ.
			REG8(base + I2CREG_CTL1) &= ~UCTR;
			REG8(base + I2CREG_CTL1) |= UCTXSTT;

			// Wait for START to be sent.
			while (REG8(base + I2CREG_CTL1) & UCTXSTT);

			// Send STOP.
			REG8(base + I2CREG_CTL1) |= UCTXSTP;

			// Enable receive interrupt.
			REG8(base + I2CREG_IE) |= UCRXIE;
		}
		else if (count > 1)
		{
			// Set RX byte counter.  We subtract 1 because the I2C peripheral
			// reads the first byte after the START condition is sent, which is
			// before the ISR is called.  So by the time we get into the ISR
			// (which uses rxByteCtr), we've already got the first byte.  We
			// subtract another 1 because the ISR needs the rxByteCtr to be
			// 0-indexed.
			rxByteCtr[channel] = count - 2;

			// Send START + address + READ.
			REG8(base + I2CREG_CTL1) &= ~UCTR;
			REG8(base + I2CREG_CTL1) |= UCTXSTT;
		}
		else
		{
			return I2C_RESULT_INVALID_SELECTION;
		}

		// The rest of the message will be sent via ISR.
	}

	return result;
}

/******************************************************************************
 *
 *	Function:		I2CWriteThenRead
 *
 *	Description:	Writes to, and then reads from, an I2C devie.  This is a
 *					common practice, for example we may wish to read data from
 *					an I2C EEPROM, so we will write the location we wish to
 *					start reading from, and then read the data.
 *
 *	Parameters:		uint8_t channel - which I2C peripheral to act upon
 *					uint8_t address - I2C device address to use
 *					uint8_t *writeData - array of bytes to write to slave device
 *					uint32_t writeCount - size of data array
 *					uint8_t *readData - array to put data read from the device
 *					uint32_t readCount - size of data array
 *
 *	Return Value:	I2C_RESULT_OK for success; other values indicate failure
 *
 *****************************************************************************/

i2cResult_t I2CWriteThenRead(uint8_t channel, uint8_t address, uint8_t *writeData,
	uint32_t writeCount, uint8_t *readData, uint32_t readCount)
{
	i2cResult_t result = I2C_RESULT_OK;	// an optimistic return value :)
	uint16_t base = baseAddr[channel];	// Convert channel to base address.

	// Check for invalid channel.
	if (channel >= I2C_NUM_CHANNELS)
	{
		result = I2C_RESULT_INVALID_SELECTION;
	}

	else
	{
		// Set the address we'll attempt to talk to.
		REG8(base + I2CREG_SA) = address;

		// Set TX array start address.
		txDataPtr[channel] = writeData;

		// Load TX byte counter.
		txByteCtr[channel] = writeCount;

		// Set RX array start address.
		rxDataPtr[channel] = readData;

		// Set RX byte counter.  Unlike in I2CRead, we don't subtract anything,
		// because we won't read any bytes until the ISR, and we need this to
		// be 1-indexed (for now).  It's tricky...you might want to step
		// through this operation with a debugger.
		rxByteCtr[channel] = readCount;

		// Send START + address + WRITE.
		REG8(base + I2CREG_CTL1) |= UCTXSTT | UCTR;

		// The rest of the message will be sent via ISR.
	}

	return result;
}

/******************************************************************************
 *
 *	Function:		I2CReadThenWrite
 *
 *	Description:	Writes to, and then reads from, an I2C device.  This is not
 *					common, but the author thought "why not?".
 *
 *	Parameters:		uint8_t channel - which I2C peripheral to act upon
 *					uint8_t address - I2C device address to use
 *					uint8_t *readData - array to put data read from the device
 *					uint32_t readCount - size of data array
 *					uint8_t *writeData - array of bytes to write to slave device
 *					uint32_t writeCount - size of data array
 *
 *	Return Value:	I2C_RESULT_OK for success; other values indicate failure
 *
 *****************************************************************************/

i2cResult_t I2CReadThenWrite(uint8_t channel, uint8_t address, uint8_t *readData,
		uint32_t readCount, uint8_t *writeData, uint32_t writeCount)
{
	i2cResult_t result = I2C_RESULT_OK;	// an optimistic return value :)
	uint16_t base = baseAddr[channel];	// Convert channel to base address.

	// Check for invalid channel.
	if (channel >= I2C_NUM_CHANNELS)
	{
		result = I2C_RESULT_INVALID_SELECTION;
	}

	else
	{
		// Set the address we'll attempt to talk to.
		REG8(base + I2CREG_SA) = address;

		// Set TX array start address.
		txDataPtr[channel] = writeData;

		// Load TX byte counter.
		txByteCtr[channel] = writeCount;

		// Set RX array start address.
		rxDataPtr[channel] = readData;

		// Set RX byte counter.  We subtract 1 because the I2C peripheral reads
		// the first byte after the START condition is sent, which is before
		// the ISR is called.  So by the time we get into the ISR (which uses
		// rxByteCtr), we've already got the first byte.
		rxByteCtr[channel] = readCount - 1;

		// Send START + address + READ.
		REG8(base + I2CREG_CTL1) &= ~UCTR;
		REG8(base + I2CREG_CTL1) |= UCTXSTT;

		// The rest of the message will be sent via ISR.
	}
	return result;
}

/******************************************************************************
 *	Test Functions
 *****************************************************************************/

#ifdef INCLUDE_TESTS

#define I2C_ADDR	0x50

// First message:  Write to EEPROM.
const uint8_t writeMsg[] = {
	0x00,	// register to access
	0x00,	// value for register
	0x01,	// value for register
	0x02,	// value for register
	0x03,
	0x04,
	0x05,
};

// Second message:  Read from EEPROM.  This is for the "write-then-read" test.
const uint8_t readMsg[] = {
	0x00,	// EEPROM register to read from
};

uint8_t I2CTest(uint8_t channel, uint8_t repeatFlag)
{
	i2cResult_t result = I2C_RESULT_OK;	// an optimistic return value :)
	i2cConfig_t config;					// module config data
	volatile uint8_t response[16];		// bytes received from EEPROM

	do	// I'm afraid of goto statements, so I'll use this instead ;}
	{
		// Setup the I2C configuration.
		config.mode = I2C_MODE_MASTER;
		config.speed = 10000;
		config.address = I2C_ADDR;
		config.generalCallEnable = 0;

		// Initialize the I2C peripheral with the configuration.
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
			while (I2CIsBusBusy(channel));

			// Try a write.
			result = I2CWrite(channel, (uint8_t *)writeMsg, sizeof(writeMsg));
			if (result != I2C_RESULT_OK)
				break;

			// Wait until the device is not busy from the previous operation.
			while (I2CIsBusBusy(channel));
			
			// Wait > 5 ms for the EEPROM to finish the write cycle (see
			// datasheet).  MCLK = 1048576 Hz (by default), so:
			// MCLK (1/s) * 0.005 s = 5242.88.  Round up to 5250, and wait that
			// many cycles.
			__delay_cycles(5250);

			// Try a write-then-read.
			result = I2CWriteThenRead(channel, (uint8_t *)readMsg,
				sizeof(readMsg), (uint8_t *)response, sizeof(response));
			if (result != I2C_RESULT_OK)
				break;

			// Wait until the device is not busy from the previous operation.
			while (I2CIsBusBusy(channel));

		} while (repeatFlag);			// Repeat if requested.
	} while (0);						// Don't repeat the initialization.

	return 0;
}

#endif	/* INCLUDE_TEST */

/******************************************************************************
 *
 *	Function:		I2C_ISR
 *
 *	Description:	This function is called by the interrupt handlers for this
 *					module.  It is structured such that it can be used to
 *					transmit any number of bytes by pre-loading a byte counter
 *					with the byte count, and doing some pointer math.  There
 *					are tricky things about how this hardware works, so	read
 *					Note 3 (above) to learn more.  Also, this function is
 *					declared as "inline" to speed up code execution in the ISR.
 *
 *	Parameters:		uint8_t ch - which I2C peripheral to act upon
 *					volatile uint16_t reg - interrupt vector address
 *
 *****************************************************************************/

static inline void I2C_ISR(uint8_t ch, volatile uint16_t reg)
{
	uint16_t base = baseAddr[ch];						// Convert channel to base address.

	switch(__even_in_range(reg,12))						// Find the interrupt's cause.
	{
	case  0: break;										// Vector  0: No interrupts
	case  2: break;										// Vector  2: ALIFG
	case  4:											// Vector  4: NACKIFG
		REG8(base + I2CREG_CTL1) |= UCTXSTP;			// Send STOP.
		REG8(base + I2CREG_CTL1) &= ~UCNACKIFG;			// Clear no-ACK flag.
		break;
	case  6: break;										// Vector  6: STTIFG
	case  8: break;										// Vector  8: STPIFG
	case 10:											// Vector 10: RXIFG
		if (rxByteCtr[ch])								// Check RX byte counter.
		{
			*rxDataPtr[ch] = REG8(base + I2CREG_RXBUF);	// Save received byte; fetch next byte.
			rxDataPtr[ch]++;							// Increment RX buffer pointer.
			rxByteCtr[ch]--;							// Decrement RX byte counter.
		}
		else if (txByteCtr[ch])							// Is this a read-then-write?
		{
			REG8(base + I2CREG_CTL1) |= UCTR;			// Set WRITE mode.
			REG8(base + I2CREG_CTL1) |= UCTXSTT;		// Send re-START.
		}
		else											// One byte left.
		{
			REG8(base + I2CREG_IE) &= ~UCRXIE;			// Disable RX interrupt (see Note 3).
			REG8(base + I2CREG_CTL1) |= UCTXSTP;		// Send STOP before last byte is read.
			*rxDataPtr[ch] = REG8(base + I2CREG_RXBUF);	// Save received byte; clear int flag.
			rxDataPtr[ch]++;							// Increment RX buffer pointer.
			REG8(base + I2CREG_IE) |= UCRXIE;			// Enable RX interrupt (see Note 3).
		}
		break;

	case 12:											// Vector 12: TXIFG
		if (txByteCtr[ch])								// Check TX byte counter.
		{
			REG8(base + I2CREG_TXBUF) = *txDataPtr[ch];	// Load TX buffer.
			txDataPtr[ch]++;							// Increment TX buffer pointer.
			txByteCtr[ch]--;							// Decrement TX byte counter
		}
		else if (rxByteCtr[ch] == 1)					// Is this a (1-byte) write-then-read?
		{
			REG8(base + I2CREG_IE) &= ~UCRXIE;			// Disable RX interrupt (see Note 3).
			REG8(base + I2CREG_CTL1) &= ~UCTR;			// Set READ mode.
			REG8(base + I2CREG_CTL1) |= UCTXSTT;		// Send re-START (but don't fetch).
			while (REG8(base + I2CREG_CTL1) & UCTXSTT);	// Wait for START to be sent.
			REG8(base + I2CREG_CTL1) |= UCTXSTP;		// Send STOP.
			REG8(base + I2CREG_IE) |= UCRXIE;			// Enable RX interrupt (see Note 3).
			rxByteCtr[ch]--;							// Decrement RX byte counter.
		}
		else if (rxByteCtr[ch])							// Is this a write-then-read?
		{
			REG8(base + I2CREG_CTL1) &= ~UCTR;			// Set READ mode.
			REG8(base + I2CREG_CTL1) |= UCTXSTT;		// Send re-START (fetches 1st byte).
			rxByteCtr[ch] -= 2;							// Decrement RX byte counter by 2.
		}
		else											// We're done!
		{
			REG8(base + I2CREG_CTL1) |= UCTXSTP;		// Send STOP.
//			REG8(base + I2CREG_IFG) &= ~UCTXIFG;		// Clear int flag.
		}
		break;

	default: break;
	}
}

/******************************************************************************
 *
 * 	Function:		USCI_B0_ISR, USCI_B1_ISR, USCI_B2_ISR, USCI_B3_ISR
 *
 * 	Description:	These are interrupt service routines that are triggered by
 * 					USCI events.  It looks messy because of define statements
 * 					which format the code differently for different compilers.
 * 					It is also automatically not included if the processor
 * 					we're using does not have this module.
 *
 *****************************************************************************/

#if defined(__MSP430_HAS_USCI_B0__)

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B0_VECTOR))) USCI_B0_ISR (void)
#else
#error Compiler not supported!
#endif
{
	I2C_ISR(I2C_0, UCB0IV);
}

#endif /* USCI B0 */

#if defined(__MSP430_HAS_USCI_B1__)

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B1_VECTOR))) USCI_B1_ISR (void)
#else
#error Compiler not supported!
#endif
{
	I2C_ISR(I2C_1, UCB1IV);
}

#endif /* USCI B1 */

#if defined(__MSP430_HAS_USCI_B2__)

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B2_VECTOR
__interrupt void USCI_B2_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B2_VECTOR))) USCI_B2_ISR (void)
#else
#error Compiler not supported!
#endif
{
	I2C_ISR(I2C_2, UCB2IV);
}

#endif /* USCI B2 */

#if defined(__MSP430_HAS_USCI_B3__)

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B3_VECTOR
__interrupt void USCI_B3_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B3_VECTOR))) USCI_B3_ISR (void)
#else
#error Compiler not supported!
#endif
{
	I2C_ISR(I2C_3, UCB3IV);
}

#endif /* USCI B3 */
