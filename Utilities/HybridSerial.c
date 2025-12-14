/******************************************************************************
 *
 *	Filename:		HybridSerial.c
 *
 *	Description:	A strange serial library, which uses an external DAC to
 *					drive the TX line, and the processor's internal UART to
 *					control the RX line.
 *
 *****************************************************************************/

#include "project.h"
#include "DAC7311.h"
#include "mcc_generated_files/eusart.h"
#include "HybridSerial.h"

#define DAC_LOW_BIT			0xFFF	// to make serial line go low
#define DAC_HIGH_BIT		0x000	// to make serial line go high
#define DAC_DELAY_1200BAUD	100		// delay to make 1200 baud

static HybridRxCallback _UartRxCallback;

/******************************************************************************
 *
 *	Function:		HybridInit
 *
 *	Description:	Set up the hardware UART.
 *
 *****************************************************************************/

hybridResult_t HybridInit(void)
{
	EUSART_Initialize();
	
    _UartRxCallback = DNULL;
	
	return HybridResultOK;
}

/******************************************************************************
 *
 *	Function:		HybridWrite
 *
 *	Description:	Write a byte (via DAC).
 *
 *****************************************************************************/

hybridResult_t HybridWrite(uint8_t data)
{
	hybridResult_t hybridResult;		// return value
	dacResult_t dacResult;				// status of DAC functions
	uint8_t i;							// counter
	
	// Start bit (low).
	dacResult = DACSetOutput(DAC_LOW_BIT);
	if (dacResult != DacResultOK)
		hybridResult = HybridResultFail;
	
	// Delay to make 1200 baud.
	_delay(DAC_DELAY_1200BAUD);
	
	// Send each bit, LSB first.
	for (i = 8; i != 0; i--)
	{
		// Send the bit.
		if (data & 0x01)
			DACSetOutput(DAC_HIGH_BIT);
		else
			DACSetOutput(DAC_LOW_BIT);
		
		if (dacResult != DacResultOK)
			hybridResult = HybridResultFail;
		
		// Shift to next bit.
		data >>= 1;
		
		// Delay to make 1200 baud.
		_delay(DAC_DELAY_1200BAUD);
	}
	
	// Stop bit (high).
	DACSetOutput(DAC_HIGH_BIT);
	
	// Delay to make 1200 baud.
	_delay(DAC_DELAY_1200BAUD);

	return hybridResult;
}

/******************************************************************************
 *
 *	Function:		HybridRead
 *
 *	Description:	Read a byte.
 *
 *****************************************************************************/

hybridResult_t HybridRead(uint8_t *data)
{
	hybridResult_t hybridResult;		// return value
	
	*data = EUSART_Read();				// Fetch the received data.
		
	hybridResult = HybridResultOK;		// Function worked :)
	
	return hybridResult;
}

/******************************************************************************
 *
 *	Function:		HybridRegisterRXCallback
 *
 *	Description:	Register a callback function when a new data byte is 
 *                  received..
 *
 *****************************************************************************/

hybridResult_t HybridRegisterRXCallback(HybridRxCallback callback)
{
    _UartRxCallback = callback;
    
    return HybridResultOK;
}

/******************************************************************************
 *
 *	Function:		HybridRXCallback
 *
 *	Description:	Function called by the underlying UART library when data is
 *                  received.
 *
 *****************************************************************************/

void HybridRXCallback(void)
{
    if (_UartRxCallback != DNULL)
    {
        _UartRxCallback();
    }
}

/******************************************************************************
 *
 *	Function:		HybridTest
 *
 *	Description:	Echo character received.
 *
 *****************************************************************************/

hybridResult_t HybridTest(void)
{
	hybridResult_t hybridResult;		// serial port driver status
	DUINT8_T data;						// byte received from serial port
	
	HybridInit();						// Set up communications.
	
	do
	{
		// Read from serial port.
		hybridResult = HybridRead(&data);

		// If we got data...
		if (hybridResult == HybridResultOK)
		{
			// Echo the data back.
			hybridResult = HybridWrite(data);
		}
	} while (!EUSART_DataReady);
	
	return hybridResult;
}