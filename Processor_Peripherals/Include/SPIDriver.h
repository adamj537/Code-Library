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
	SPI_MODE_0,							// CPOL = , CPHA = 
	SPI_MODE_1,							// CPOL = 0, CPHA = 1
	SPI_MODE_2,							// CPOL = , CPHA = 
	SPI_MODE_3,							// CPOL = , CPHA = 
} spiMode_t;

typedef enum							// types of callback functions
{
	SPI_NUM_CALLBACKS
} spiCbType_t;

typedef struct							// settings for an SPI channel
{
	spiMode_t mode;						// operation mode
	uint32_t speed;						// bus speed
} spiConfig_t;

typedef void (*spiCallback_t)(uint8_t *dataPtr);	// prototype for callback functions

// High-level functions:
spiResult_t SPIInit(uint8_t channel, spiConfig_t *configPtr);
spiResult_t SPIRegisterCallback(uint8_t channel, spiCbType_t type, i2cCallback_t callbackPtr);
spiResult_t SPITransfer(uint8_t channel, uint8_t *mosiDataPtr, uint8_t *misoDataPtr, uint8_t count);
uint8_t     SPIIsBusy(uint8_t channel);

#endif /* I2CDRIVER_H */
