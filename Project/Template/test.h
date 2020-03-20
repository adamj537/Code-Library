/******************************************************************************
 *
 *	Filename:		test.h
 *
 *	Author:			Adam Johnson
 *
 *	Description:	Contains information and settings applicable to test code.
 *					This file should be included only in test builds.
 *
 *****************************************************************************/

#ifndef TEST_H
#define TEST_H

#ifdef INCLUDE_TEST	// Don't include this in release build configurations.

/******************************************************************************
 *  Configure UART Driver Test.
 *****************************************************************************/

#define TEST_UART_CHANNEL			0
#define TEST_UART_BAUDRATE			9600
#define TEST_UART_DATABITS			UART_DATA_BITS_8
#define TEST_UART_STOPBITS			UART_STOP_BITS_1
#define TEST_UART_PARITY			UART_PARITY_NONE

#endif	/* INCLUDE_TEST */
#endif	/* TEST_H */
