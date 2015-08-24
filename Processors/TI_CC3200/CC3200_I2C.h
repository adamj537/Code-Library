/******************************************************************************
 *	Filename:		CC3200_I2C.h
 *
 *	Author:			Adam Johnson
 *
 *	Description:	Master I2C library for CC3200.  Tested on CC3200 LaunchXL
 *					board.
 *
 *****************************************************************************/

#ifndef CC3200_I2C_H
#define CC3200_I2C_H

typedef void (*i2cCallback_t)(uint8_t *dataPtr);	// Prototype for callback functions

typedef enum							// Enumeration for the I2C peripherals
{
	I2C_CH_0,							// this is the only channel on CC3200
	I2C_NUM_CHANNELS
} i2cChannel_t;

typedef enum							// error codes
{
	I2C_RESULT_OK = 0,					// All is well!
	I2C_RESULT_FAIL,					// It's the target's fault.
	I2C_RESULT_NOT_IMPLEMENTED,			// It's my fault.
	I2C_RESULT_INVALID_SELECTION		// It's your fault.
} i2cResult_t;

typedef enum							// Callback assignments
{
	I2C_CB_TX,							// A transaction is complete (master modes)
	I2C_CB_RX,							// A transaction was received (slave modes)
	I2C_CB_ARB_LOST,					// Arbitration lost (multi-master mode)
	I2C_CB_NO_ACK,						// Ack was expected but not received
	I2C_CB_START,						// START + myAddress received/sent
	I2C_CB_STOP,						// STOP received/sent
	I2C_NUM_CALLBACKS
} i2cCbType_t;

typedef struct							// configuration struct
{
	i2cChannel_t channel;				// index of the peripheral to access
	uint32_t speed;						// desired bus speed
	uint8_t slaveAddress;				// desired slave address to talk to
} i2cConfig_t;

i2cResult_t I2CInit(i2cConfig_t *configPtr);
i2cResult_t I2CEnable(i2cChannel_t channel);
i2cResult_t I2CDisable(i2cChannel_t channel);
i2cResult_t I2CRegisterCallback(i2cChannel_t channel, i2cCbType_t type, i2cCallback_t callbackPtr);
i2cResult_t I2CEnableCallback(i2cChannel_t channel, i2cCbType_t type);
i2cResult_t I2CDisableCallback(i2cChannel_t channel, i2cCbType_t type);
i2cResult_t I2CRead(i2cChannel_t channel, uint8_t *dataPtr, uint32_t count);
i2cResult_t I2CWrite(i2cChannel_t channel, uint8_t *dataPtr, uint32_t count);
i2cResult_t I2CWriteThenRead(i2cChannel_t channel, uint8_t *writeDataPtr, uint32_t writeCount, uint8_t *readDataPtr, uint32_t readCount);
i2cResult_t I2CReadThenWrite(i2cChannel_t channel, uint8_t *readDataPtr, uint32_t readCount, uint8_t *writeDataPtr, uint32_t writeCount);
bool I2CIsBusy(i2cChannel_t channel);

#ifdef INCLUDE_TEST
i2cResult_t I2CTest(i2cChannel_t channel, uint8_t repeatFlag);
#endif

#endif /* CC3200_H */
