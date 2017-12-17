/******************************************************************************
 *
 *	Filename:		UARTDriver.h
 *
 *	Author:			Adam Johnson
 *
 *	Description:	Driver for a processor's internal UART or USART.
 *
 *	Terms of Use:	MIT License
 *
 *****************************************************************************/

#ifndef UARTDRIVER_H
#define UARTDRIVER_H

typedef enum							// result of requested UART action
{
	UART_RESULT_OK = 0,					// All is well!
	UART_RESULT_FAIL_FRAMING,			// A low stop-bit was detected.
	UART_RESULT_FAIL_PARITY,			// Parity-bit mismatch detected.
	UART_RESULT_FAIL_OVERRUN,			// Received bytes so fast that we lost one.
	UART_RESULT_FAIL_BUSY,				// The peripheral is busy already.
	UART_RESULT_FAIL,					// It's the target's fault in some other way.
	UART_RESULT_NOT_IMPLEMENTED,		// It's my fault.
	UART_RESLT_INVALID_SELECTION,		// It's your fault.
} uartResult_t;

typedef enum							// how many data bits to send in a frame
{
	UART_DATA_BITS_5,
	UART_DATA_BITS_6,
	UART_DATA_BITS_7,
	UART_DATA_BITS_8,
	UART_DATA_BITS_9
} uartDataBits_t;

typedef enum							// how many stop bits to send in a frame
{
	UART_STOP_BITS_1,
	UART_STOP_BITS_2
} uartStopBits_t;

typedef enum							// what type of parity bit to use
{
	UART_PARITY_NONE,
	UART_PARITY_ODD,
	UART_PARITY_EVEN,
} uartParity_t;

typedef struct							// settings for a UART channel
{
	uint32_t baudRate;					// baud rate
	uartDataBits_t dataBits;			// data bits
	uartStopBits_t stopBits;			// stop bits
	uartParity_t parity;				// parity
} uartConfig_t;

uartResult_t UARTInit(uint8_t channel, uartConfig_t *configPtr);
uartResult_t UARTEnable(uint8_t channel);
uartResult_t UARTDisable(uint8_t channel);
uartResult_t UARTWrite(uint8_t channel, uint8_t *data, uint32_t count);
uartResult_t UARTRead(uint8_t channel, uint8_t *data, uint32_t count);
uartResult_t UARTIsBusy(uint8_t channel);

#ifdef INCLUDE_TESTS
uartResult_t UARTTest(uint8_t channel);
#endif

#endif /* UARTDRIVER_H */
