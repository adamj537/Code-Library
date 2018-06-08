/******************************************************************************
 *
 *	Filename:		I2CDriver.h
 *
 *	Author:			Adam Johnson
 *
 *	Description:	Driver for I2C functions.  Low- and high-level functions.
 *
 *	Terms of Use:	MIT License
 *
 *****************************************************************************/

#ifndef I2CDRIVER_H
#define I2CDRIVER_H

typedef enum							// result of a requested I2C action
{
	I2C_RESULT_OK = 0,					// All is well!
	I2C_RESULT_FAIL,					// It's the target's fault.
	I2C_RESULT_NOT_IMPLEMENTED,			// It's my fault.
	I2C_RESULT_INVALID_SELECTION		// It's your fault.
} i2cResult_t;

typedef enum							// enumeration for operation mode
{
	I2C_MODE_MASTER,					// You're the master.
	I2C_MODE_MULTI_MASTER,				// You want to share.
	I2C_MODE_SLAVE,						// You're the slave.
	I2C_MODE_SLAVE_AND_GEN_CALL,		// You're an eavesdropping slave.
} i2cMode_t;

typedef struct							// settings for an I2C channel
{
	i2cMode_t mode;						// operation mode
	uint32_t speed;						// bus speed
} i2cConfig_t;

typedef enum							// types of callback functions
{
	I2C_CB_TX,							// A transaction is complete (master modes)
	I2C_CB_RX,							// A transaction was received (slave modes)
	I2C_CB_ARB_LOST,					// Arbitration lost (multi-master mode)
	I2C_CB_NO_ACK,						// Ack was expected but not received
	I2C_CB_START,						// START + myAddress received/sent
	I2C_CB_STOP,						// STOP received/sent
	I2C_NUM_CALLBACKS
} i2cCbType_t;

typedef void (*i2cCallback_t)(uint8_t *dataPtr);	// prototype for callback functions

// High-level functions:
i2cResult_t I2CInit(uint8_t channel, i2cConfig_t *configPtr);
i2cResult_t I2CRegisterCallback(uint8_t channel, i2cCbType_t type, i2cCallback_t callbackPtr);
i2cResult_t I2CWrite(uint8_t channel, uint8_t address,
				uint8_t *dataPtr, uint32_t count);
i2cResult_t I2CRead(uint8_t channel, uint8_t address,
				uint8_t *dataPtr, uint32_t count);
i2cResult_t I2CWriteThenRead(uint8_t channel, uint8_t address,
				uint8_t *writeDataPtr, uint32_t writeCount,
				uint8_t *readDataPtr, uint32_t readCount);
i2cResult_t I2CReadThenWrite(uint8_t channel, uint8_t address,
				uint8_t *readDataPtr, uint32_t readCount,
				uint8_t *writeDataPtr, uint32_t writeCount);
bool        I2CIsBusy(uint8_t channel);

#ifdef INCLUDE_TESTS
uint8_t I2CTest(uint8_t channel);
#endif

// "Or" these definitions with 7-bit address when using low-level functions:
#define I2C_READ    1					// read from I2C device
#define I2C_WRITE   0					// write to I2C device

typedef enum							// response to send after reading
{										// (for low-level functions)
	NACK = 0,
	ACK
} i2cAck_t;
// Low-level functions:
void        I2CStop(uint8_t channel);						// Send STOP.
uint8_t     I2CStart(uint8_t channel, uint8_t addr);		// Send START.
void        I2CStartWait(uint8_t channel, uint8_t addr);	// Send START & wait for ACK.
uint8_t     I2CWrite(uint8_t channel, uint8_t data);		// Send one byte.
uint8_t     I2CRead(uint8_t channel, i2cAck_t response);	// Read one byte; request more or STOP.

#endif /* I2CDRIVER_H */
