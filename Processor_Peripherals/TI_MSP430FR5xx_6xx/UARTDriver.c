/******************************************************************************
 *
 *	Filename:		UARTDriver.c
 *
 *	Author:			Adam Johnson
 *
 *	Description:	Contains functions to control UART peripherals for TI's
 *					MSP430 processors.
 *
 *****************************************************************************/

#include "project.h"					// global settings
#include <stdint.h>						// compiler-specific data types
#include <driverlib.h>					// TI peripheral drivers
#include <assert.h>						// assert is used in this module
#include "hal_UART.h"					// header for this module

/******************************************************************************
 * 	Configuration options
 *****************************************************************************/
#ifndef UART_CLOCK_FREQ_HZ
#warning "UART_CLOCK_FREQ_HZ is not defined"
#define UART_CLOCK_FREQ_HZ 	CS_getSMCLK()	// master clock frequency
#endif

#ifndef UART_CLOCK_SOURCE
#warning "UART_CLOCK_SOURCE is not defined"
#define UART_CLOCK_SOURCE	UCSSEL__SMCLK	// use main clock
#endif
/*****************************************************************************/

// This array holds the address of the lowest register in each I2C peripheral.
// If we know that address and the "offset" of each register (defined above),
// then we can access any register of any I2C peripheral.
static const uint16_t baseAddr[] = {
#ifdef __MSP430_HAS_USCI_B0__			// USCI B0
	__MSP430_BASEADDRESS_USCI_B0__,
#endif

#ifdef __MSP430_HAS_USCI_B1__			// USCI B1
	__MSP430_BASEADDRESS_USCI_B1__,
#endif

#ifdef __MSP430_HAS_USCI_B2__			// USCI B2
	__MSP430_BASEADDRESS_USCI_B2__,
#endif

#ifdef __MSP430_HAS_USCI_B3__			// USCI B3
	__MSP430_BASEADDRESS_USCI_B3__,
#endif
	// Note:  EUSCI_B0 does not support UART
#ifdef __MSP430_HAS_EUSCI_A0__			// eUSCI A0
	__MSP430_BASEADDRESS_EUSCI_A0__,
#endif

#ifdef __MSP430_HAS_EUSCI_A1__			// eUSCI A1
	__MSP430_BASEADDRESS_EUSCI_A1__,
#endif
};

// Figure out how many channels we have available.
#define UART_NUM_CHANNELS	(sizeof(baseAddr)/sizeof(baseAddr[0])

uartResult_t UARTInit(uartChannel_t channel, uartConfig_t *configPtr)
{
	uartResult_t result = UART_RESULT_OK;	// an optimistic return value ;)
	uint16_t base = baseAddr[channel];		// Convert channel to base address.
	EUSCI_A_UART_initParam initParam;		// driverlib configuration settings

	// Ensure the index is valid.
	assert(channel < UART_NUM_CHANNELS);

	// Select clock source.
	initParam.selectClockSource = UART_CLOCK_SOURCE;

	// TODO:  Calculate the prescaler and baud rate register values.
	initParam.clockPrescalar = 0;
	initParam.firstModReg = 0;
	initParam.secondModReg = 0;

	// Select the parity.
	if (configPtr->parity == UART_PARITY_NONE)
		initParam.parity = EUSCI_A_UART_NO_PARITY;
	else if (configPtr->parity == UART_PARITY_ODD)
		initParam.parity = EUSCI_A_UART_ODD_PARITY;
	else if (configPtr->parity == UART_PARITY_EVEN)
		initParam.parity = EUSCI_A_UART_EVEN_PARITY;
	else
		result = UART_RESLT_INVALID_SELECTION;

	// TODO: Select the direction of receive and transmit shift register.
	initParam.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;

	// Select the stop bit setting.
	if (configPtr->stopBits == UART_STOP_BITS_1)
		initParam.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
	else if (configPtr->stopBits == UART_STOP_BITS_2)
		initParam.numberofStopBits = EUSCI_A_UART_TWO_STOP_BITS;
	else
		result = UART_RESLT_INVALID_SELECTION;

	// TODO: Select the mode of operation.
	initParam.uartMode = EUSCI_A_UART_MODE;

	// TODO: Select oversampling mode.
	initParam.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;

	// Initialize the UART.
	if (EUSCI_A_UART_init(base, &initParam) != STATUS_SUCCESS)
		result = UART_RESULT_FAIL;

	return result;
}

uartResult_t UARTEnable(uartChannel_t channel)
{
	return UART_RESULT_NOT_IMPLEMENTED;
}

uartResult_t UARTDisable(uartChannel_t channel)
{
	return UART_RESULT_NOT_IMPLEMENTED;
}

uartResult_t UARTWrite(uartChannel_t channel, uint8_t *data, uint32_t count)
{
	return UART_RESULT_NOT_IMPLEMENTED;
}

uartResult_t UARTRead(uartChannel_t channel, uint8_t *data, uint32_t count)
{
	return UART_RESULT_NOT_IMPLEMENTED;
}

uartResult_t UARTIsBusy(uartChannel_t channel)
{
	return UART_RESULT_NOT_IMPLEMENTED;
}
