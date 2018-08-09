/******************************************************************************
 *
 *	Filename:		SPIDriver.h
 *
 *	Author:			Adam Johnson
 *
 *	Description:	Driver for SPI functions.  Low- and high-level functions.
 *
 *	Terms of Use:	MIT License
 *
 *****************************************************************************/

#ifndef SPIDRIVER_H
#define SPIDRIVER_H

typedef enum							// result of a requested SPI action
{
	SPI_RESULT_OK = 0,					// All is well!
	SPI_RESULT_FAIL,					// It's the target's fault.
	SPI_RESULT_NOT_IMPLEMENTED,			// It's my fault.
	SPI_RESULT_INVALID_SELECTION		// It's your fault.
} spiResult_t;

typedef enum							// enumeration for operation mode
{
	SPI_MODE_0,							// CPOL = 0, CPHA = 0
	SPI_MODE_1,							// CPOL = 0, CPHA = 1
	SPI_MODE_2,							// CPOL = 1, CPHA = 0
	SPI_MODE_3,							// CPOL = 1, CPHA = 1
} spiMode_t;

typedef enum							// types of callback functions
{
	SPI_NUM_CALLBACKS
} spiCbType_t;

typedef struct							// settings for an SPI channel
{
	uint32_t speed;						// bus speed
	spiMode_t mode;						// operation mode
	bool master;						// true = master, false = slave
} spiConfig_t;

typedef void (*spiCallback_t)(uint8_t *dataPtr);	// prototype for callback functions

// High-level functions:
spiResult_t SPIInit(uint8_t channel, spiConfig_t *configPtr);
spiResult_t SPIRegisterCallback(uint8_t channel, spiCbType_t type, i2cCallback_t callbackPtr);
spiResult_t SPITransfer(uint8_t channel, uint8_t *mosiDataPtr, uint8_t *misoDataPtr, uint8_t count);
uint8_t     SPIIsBusy(uint8_t channel);

#endif /* I2CDRIVER_H */
