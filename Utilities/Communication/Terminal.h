/******************************************************************************
 *
 *	Filename:		Terminal.h
 *
 *	Description:	UART interface for working with a console application.
 *					Inspired by code from Texas Instruments CC3200 DriverLib,
 *
 *	Note:			DriverLib is Copyright (C) 2014 Texas Instruments Inc.
 *
 *****************************************************************************/

#ifndef TERMINAL_H
#define TERMINAL_H

/****************************************************************************/
/*								MACROS										*/
/****************************************************************************/
#define CONSOLE_BAUD_RATE	115200		// desired baud rate
#define CONSOLE_UART		UARTA0_BASE	// UART to use
#define CONSOLE_BUF_SIZE	64			// size of console buffer for RX

extern unsigned char g_ucUARTBuffer[];	// console buffer

void DispatcherUARTConfigure(void);
void DispatcherUartSendPacket(unsigned char *inBuff, unsigned short usLength);
int GetCmd(char *pcBuffer, unsigned int uiBufLen);
void InitTerm(void);
void ClearTerm(void);
void Message(const char *format);
void Error(char *format,...);
int TrimSpace(char * pcInput);
int Report(const char *format, ...);

#endif

