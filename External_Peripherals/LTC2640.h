/*******************************************************************************
 *
 *	Filename:		LTC2640.h
 *
 *	Description:	Driver for Linear Technology's LTC2640 DAC.
 *					This is a 8/10/12-bit DAC that uses SPI to communicate.
 *
 *	Status:			NOT TESTED
 *
 ******************************************************************************/

#ifndef LTC2640_H
#define LTC2640_H

typedef enum							// error codes
{
	DacResultOK,						// All is well.
	DacResultFail,						// It's the chip's fault.
	DacResultNotImplemented,			// It's my fault.
	DacResultInvalidSelection			// It's your fault.
} dacResult_t;

typedef enum							// configuration commands
{
	DacCmdWrite			 = 0b0000,		// Write new output value.
	DacCmdUpdate		 = 0b0001,		// Turn output on / update output.
	DacCmdWriteAndUpdate = 0b0011,		// Write and turn on / update output.
	DacCmdOff			 = 0b0100,		// Turn output off.
	DacCmdRefInt		 = 0b0110,		// Select Internal Reference
	DacCmdRefExt		 = 0b0111		// Select External Reference
} dacCmd_t;

// Change the DAC's mode (turn it on or off).
dacResult_t DACConfig(dacCmd_t command, uint16_t counts);

// Output a voltage on the DAC (and turn it on).
dacResult_t DACSetOutput(uint16_t counts);

// Test the DAC.
dacResult_t DACTest(void);

#endif /* LTC2640_H */