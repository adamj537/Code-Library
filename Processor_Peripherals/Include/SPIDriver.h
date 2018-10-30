/******************************************************************************
 *
 *	Filename:		SPIDriver.h
 *
 *	Author:			Adam Johnson
 *
 *	Description:	Driver for SPI functions.
 *
 *	Terms of Use:	MIT License
 *
 *****************************************************************************/

#ifndef SPIDRIVER_H
#define SPIDRIVER_H

typedef enum							// status returned by callback
{
	SPI_STATUS_TX_DONE,					// done transmitting buffer
	SPI_STATUS_RX_DONE,					// done receiving buffer
	SPI_STATUS_ERROR,					// error has occurred
} spiStatus_t;

typedef enum							// enumeration for operation mode
{
	SPI_MODE_0,							// CPOL = 0, CPHA = 0
	SPI_MODE_1,							// CPOL = 0, CPHA = 1
	SPI_MODE_2,							// CPOL = 1, CPHA = 0
	SPI_MODE_3,							// CPOL = 1, CPHA = 1
} spiMode_t;

typedef struct							// settings for the peripheral
{
	spiMode_t mode;						// operating mode (polarity and phase)
	uint8_t prescaler;					// bus speed divider
	uint8_t dataSize;					// size of each data transaction [bits]
	bool master;						// true = master, false = slave
	bool biDirectional;					// directional mode state
	bool lsbFirst;						// true = data transfers start with LSB; false = MSB
	bool softwareSS;					// is slave pin managed by hardware or software
} spiConfig_t;

// SPI Interrupt callback function prototype
typedef void (*SPIIntCallback_t)(spiStatus_t status);

// Initialize an SPI peripheral.
void SPIInit(uint8_t channel, spiConfig_t *configPtr);

// Register a callback for when transmission is complete.
void SPIRegisterCallback(uint8_t channel, SPIIntCallback_t Callback);

// Transfer data.
void SPITransfer(uint8_t channel, uint8_t *txArray, uint8_t *rxArray, size_t size);
void SPIRead(uint8_t channel, uint8_t *rxArray, size_t count);
void SPIWrite(uint8_t channel, uint8_t *txArray, size_t count);

// See whether the peripheral is busy or idle.
bool SPIIsBusy(uint8_t channel);

#ifdef INCLUDE_TEST
bool SPIMasterTest(void);				// Test this library in master mode.
bool SPISlaveTest(void);				// Test this library in slave mode.
#endif

#endif /* SPIDRIVER_H */
