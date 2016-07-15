/*******************************************************************************
 *
 *	Filename:		LTC2640.h
 *
 *	Description:	Driver for Linear Technology's LTC2640 DAC.
 *					This is a 8/10/12-bit DAC that uses SPI to communicate.
 *
 *	Notes:			After setting the CS pin low, a 24-bit command is issued.
 *					The 4-bit command is loaded first, followed by 4 don't care
 *					bits, followed by a 16-bit count value.  The count value is
 *					MSB first, and can be 12, 10, or 8 bits, depending on what
 *					resolution of DAC you're using.  The rest of the bits in the
 *					16-bit value don't matter.
 *
 *					When power is applied, the DAC outputs 0V.  If the REF-SEL
 *					pin isn't present, the DAC's internal reference is selected.
 *					If the REF-SEL pin is present, it selects the default
 *					reference at power-up, and is ignored after that.
 *
 *					Available 4-bit commands are provided in the eDacCmd_t
 *					enumeration.  The first three consist of "write" and "On"
 *					operations.  A "write" loads a 16-bit data word into the
 *					input register.  An "on" operation turns the DAC circuitry
 *					on and converts the writen value to an analog voltage.  The
 *					third commmand combines the "write" and "on" commands.  THE
 *					ON COMMAND MUST BE CALLED EVERY TIME YOU WISH TO UPDATE THE
 *					OUTPUT.
 *
 *					DacCmdOff turns off the DAC circuitry to draw less power and
 *					pulls the output pin to ground via 200k ohms.  The DAC will
 *					remember its last output value while turned off as long as
 *					power is still applied.
 *
 *					The last two commands choose whether to use the internal
 *					reference (1.25V or 2.048V depending on part in use).
 *
 ******************************************************************************/

#include "project.h"					// settings for the entire project
#include "LTC2640.h"					// header for this module

/******************************************************************************
 *
 *	Function:		DACConfig
 *
 *	Description:	Change the DAC's mode (turn it on or off).
 *
 *	Parameters:		command - whether the DAC is on or off
 *					counts - what to set output to; this is ignored by the DAC
 *						unless the command includes a write.
 * 
 *****************************************************************************/

dacResult_t DACConfig(eDacCmd_t command, DUINT16_T counts)
{
	eDacResult_t dacResult = DacResultInvalidSelection;	// return value
	spiResult_t spiResult;								// SPI function status
	DUINT32_T spiData;									// data to send
	
	// Check for invalid count value.
	if (counts <= 0x0FFF)
	{
		// Send data to the DAC.
		spiData = (command << 20) | (counts << 4);
		spiResult = SPIWriteWords(&spiData, 2);
		
		// Check for errors.
		if (spiResult == SPI_RESULT_OK)
			dacResult = DacResultOK;
	}
	
	return dacResult;
}

/******************************************************************************
 * 
 *	Function:		DACSetOutput
 * 
 *	Description:	Output a voltage on the DAC (and turn it on).
 * 
 *	Parameters:		counts - what to set output to.
 * 
 *****************************************************************************/

dacResult_t DACSetOutput(uint16_t counts)
{
	return DACConfig(DacCmdWriteAndOn, counts);
}

#ifdef INCLUDE_TESTS
dacResult_t DACTest(void)
{
	dacResult_t dacResult;
	uint16_t counts;
	
	do
	{
		// Select the DAC's internal reference.
		dacResult = DACConfig(DacCmdRefInt, 0x000);
		if (dacResult != DacResultOK)
			break;
		
		// Turn the DAC on.
		dacResult = DACConfig(DacCmdOn, 0x00);
		if (dacResult != DacResultOK)
			break;
	
		for (counts = 0; counts <= 0x0FFF; counts += 0xFF)
		{
			// Set the DAC's output.
			dacResult = DACSetOutput(counts);
			
			// Fail if something goes wrong.
			if (dacResult != DacResultOK)
				break;
		}
		
		// Turn the DAC off.
		dacResult = DACConfig(DacCmdOff, 0x000);
	} while (DFALSE);
	
	return dacResult;
}

#endif	// INCLUDE_TESTS