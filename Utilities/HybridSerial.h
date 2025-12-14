/******************************************************************************
 *
 *	Filename:		HybridSerial.h
 *
 *	Description:	A strange serial library, which uses an external DAC to
 *					drive the TX line, and the processor's internal UART to
 *					control the RX line.
 *
 *****************************************************************************/

#ifndef HYBRIDSERIAL_H
#define	HYBRIDSERIAL_H

typedef enum
{
	HybridResultOK,						// All is well :)
	HybridResultFail,					// It's the chip's fault.
	HybridResultNotImplemented,			// It's my fault.
	HybridResultInvalidSelection,		// It's your fault.
	HybridResultNoData					// Nothing to receive.
} hybridResult_t;

/*! Prototype for callback functions */
typedef void (*HybridRxCallback)(void);

hybridResult_t HybridInit(void);			// Set up the hardware UART.
hybridResult_t HybridWrite(uint8_t data);	// Write a byte.
hybridResult_t HybridRead(uint8_t *data);	// Read a byte.
hybridResult_t HybridRegisterRXCallback(HybridRxCallback callback); // RX Callback

hybridResult_t HybridTest(void);
#endif	/* HYBRIDSERIAL_H */

