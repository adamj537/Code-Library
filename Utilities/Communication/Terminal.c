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

// Standard includes
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Console.h"

#define IS_SPACE(x)	(x == 32 ? 1 : 0)

static unsigned long __Errorlog;		// flag indicating error is present
unsigned int ilen=1;					// input length


//*****************************************************************************
//
//! Initialization
//!
//! This function
//!        1. Configures the UART to be used.
//!
//! \return none
//
//*****************************************************************************

void InitTerm()
{
  UARTConfig(CONSOLE,MAP_PRCMPeripheralClockGet(CONSOLE_PERIPH), 
                  UART_BAUD_RATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                   UART_CONFIG_PAR_NONE));
  __Errorlog = 0;
}

//*****************************************************************************
//
//!    Outputs a character string to the console
//!
//! \param str is the pointer to the string to be printed
//!
//! This function
//!        1. prints the input string character by character on to the console.
//!
//! \return none
//
//*****************************************************************************

void Message(const char *str)
{
	if(str != NULL)
	{
		while(*str!='\0')
		{
			UARTCharPut(CONSOLE,*str++);
		}
	}
}

//*****************************************************************************
//
//!    Clear the console window
//!
//! This function
//!        1. clears the console window.
//!
//! \return none
//
//*****************************************************************************
void 
ClearTerm()
{
	Message("\33[2J\r");
}

//*****************************************************************************
//
//! Error Function
//!
//! \param 
//!
//! \return none
//! 
//*****************************************************************************

void Error(char *pcFormat, ...)
{
	char  cBuf[256];
	va_list list;
	va_start(list,pcFormat);
	vsnprintf(cBuf,256, pcFormat, list);
	Message(cBuf);
	__Errorlog++;
}

//*****************************************************************************
//
//! Get the Command string from UART
//!
//! \param  pucBuffer is the command store to which command will be populated
//! \param  ucBufLen is the length of buffer store available
//!
//! \return Length of the bytes received. -1 if buffer length exceeded.
//! 
//*****************************************************************************

int GetCmd(char *pcBuffer, unsigned int uiBufLen)
{
    char cChar;
    int iLen = 0;
    
    // Wait to receive a character over UART
    cChar = MAP_UARTCharGet(CONSOLE);
    
    // Echo the received character
    MAP_UARTCharPut(CONSOLE, cChar);
    iLen = 0;
    
    // Checking the end of Command
    while((cChar != '\r') && (cChar !='\n') )
    {
        // Handling overflow of buffer
        if(iLen >= uiBufLen)
        {
            return -1;
        }
        
        // Copying Data from UART into a buffer
        if(cChar != '\b')
        { 
            *(pcBuffer + iLen) = cChar;
            iLen++;
        }
        else
        {
            // Deleting last character when you hit backspace 
            if(iLen)
            {
                iLen--;
            }
        }
        // Wait to receive a character over UART
        cChar = MAP_UARTCharGet(CONSOLE);
        
        // Echo the received character
        MAP_UARTCharPut(CONSOLE, cChar);
    }

    *(pcBuffer + iLen) = '\0';

    Report("\n\r");

    return iLen;
}

//*****************************************************************************
//
//!    Trim the spaces from left and right end of given string
//!
//! \param  Input string on which trimming happens
//!
//! \return length of trimmed string
//
//*****************************************************************************

int TrimSpace(char * pcInput)
{
    size_t size;
    char *endStr, *strData = pcInput;
    char index = 0;
    size = strlen(strData);

    if (!size)
        return 0;

    endStr = strData + size - 1;
    while (endStr >= strData && IS_SPACE(*endStr))
        endStr--;
    *(endStr + 1) = '\0';

    while (*strData && IS_SPACE(*strData))
    {
        strData++;
        index++;
    }
    memmove(pcInput,strData,strlen(strData)+1);

    return strlen(pcInput);
}

//*****************************************************************************
//
//!    prints the formatted string on to the console
//!
//! \param format is a pointer to the character string specifying the format in
//!           the following arguments need to be interpreted.
//! \param [variable number of] arguments according to the format in the first
//!         parameters
//! This function
//!        1. prints the formatted error statement.
//!
//! \return count of characters printed
//
//*****************************************************************************

int Report(const char *pcFormat, ...)
{
	int iRet = 0;
	char *pcBuff;
	char *pcTemp;
	int iSize = 256;
	va_list list;
	
	pcBuff = (char*)malloc(iSize);
	if (pcBuff == NULL)
	{
		return -1;
	}
	
	while (1)
	{
		va_start(list,pcFormat);
		iRet = vsnprintf(pcBuff,iSize,pcFormat,list);
		va_end(list);
		if (iRet > -1 && iRet < iSize)
		{
			break;
		}
		else
		{
			iSize *= 2;
			if((pcTemp = realloc(pcBuff,iSize)) == NULL)
			{ 
				Message("Could not reallocate memory\n\r");
				iRet = -1;
				break;
			}
			else
			{
				pcBuff=pcTemp;
			}
		}
	}
	Message(pcBuff);
	free(pcBuff);
	return iRet;
}
