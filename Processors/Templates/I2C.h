/******************************************************************************
 *
 *	Filename:		I2C.h
 *
 *	Author:			Adam Johnson
 *
 *	Description:	This is a template for an I2C driver.  All new drivers
 *					should have functions and variable types that match these.
 *
 *****************************************************************************/

#ifndef I2C__H
#define I2C__H

typedef enum							// available I2C peripherals
{
	I2C_CH_0,
	I2C_NUM_CHANNELS					// This is how many channels we have.
} i2cChannel_t;

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

i2cResult_t I2CInit(i2cChannel_t channel, i2cConfig_t *configPtr);
i2cResult_t I2CWrite(i2cChannel_t channel, uint8_t address, uint8_t *dataPtr,
				uint8_t count);
i2cResult_t I2CRead(i2cChannel_t channel, uint8_t address, uint8_t *dataPtr,
				uint8_t count);
i2cResult_t I2CWriteThenRead(i2cChannel_t channel, uint8_t address,
				uint8_t *writeDataPtr, uint8_t writeCount,
				uint8_t *readDataPtr, uint8_t readCount);
i2cResult_t I2CReadThenWrite(i2cChannel_t channel, uint8_t address,
				uint8_t *readDataPtr, uint8_t readCount,
				uint8_t *writeDataPtr, uint8_t writeCount);
uint8_t     I2CIsBusy(i2cChannel_t channel);

#endif