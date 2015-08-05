#ifndef MSP430_I2C_H_
#define MSP430_I2C_H_

/******************************************************************************
 * 	Configuration options
 *****************************************************************************/
#define INCLUDE_TEST						// Include test code.
#define I2C_CLOCK_FREQ_HZ 	1048576			// master clock frequency
#define I2C_CLOCK_SOURCE	UCSSEL__SMCLK	// use main clock
/*****************************************************************************/

typedef enum							// available I2C channels
{
#ifdef __MSP430_HAS_USCI_B0__			// Does the processor have USCI_B0?
	I2C_0,
#endif

#ifdef __MSP430_HAS_USCI_B1__			// Does the processor have USCI_B1?
	I2C_1,
#endif

#ifdef __MSP430_HAS_USCI_B2__			// Does the processor have USCI_B2?
	I2C_2,
#endif

#ifdef __MSP430_HAS_USCI_B3__			// Does the processor have USCI_B3?
	I2C_3,
#endif
	I2C_NUM_CHANNELS,					// This is how many channels we have.
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
	I2C_MODE_INVALID = 0xFF				// You're indecisive.
} i2cMode_t;

typedef struct							// settings for an I2C channel
{
	i2cMode_t mode;						// operation mode
	uint32_t speed;						// bus speed
	uint8_t address;					// desired slave address
} i2cConfig_t;

i2cResult_t I2CInit(i2cChannel_t channel, i2cConfig_t *configPtr);
i2cResult_t I2CEnable(i2cChannel_t channel);
i2cResult_t I2CDisable(i2cChannel_t channel);
i2cResult_t I2CWrite(i2cChannel_t channel, uint8_t address,
				uint8_t *data, uint32_t count);
i2cResult_t I2CRead(i2cChannel_t channel, uint8_t address,
				uint8_t *data, uint32_t count);
i2cResult_t I2CWriteThenRead(i2cChannel_t channel, uint8_t address,
				uint8_t *writeData, uint32_t writeCount,
				uint8_t *readData, uint32_t readCount);
i2cResult_t I2CReadThenWrite(i2cChannel_t channel, uint8_t address,
				uint8_t *readData, uint32_t readCount,
				uint8_t *writeData, uint32_t writeCount);
uint8_t     I2CIsBusy (i2cChannel_t channel);

#ifdef INCLUDE_TEST
uint8_t I2CTest(i2cChannel_t channel, uint8_t repeatFlag);
#endif

#endif /* MSP430_I2C_H_ */
