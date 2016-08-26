/******************************************************************************
 *
 *	Filename:		GPIODriver.c
 *
 *	Description:	Driver for GPIO functions for the ATtiny.  GPIO on the
 *					Atmel is accessed via four registers:
 *					DDxn - sets the pin direction; 0 = input, 1 = output
 *					PORTxn - sets pin output; 0 = drive low, 1 = drive high
 *					PUExn - pullup enable; 0 = disabled, 1 = enabled
 *					PINxn - reads pin status; 0 = low, 1 = high
 *
 *	Notes:			On reset, pins are tri-stated (DDxn = 0, PUExn = 0)
 *
 *****************************************************************************/

#include <avr/io.h>						// pin register definitions
#include <stdint.h>						// universal data types
#include <stdbool.h>					// defines boolean type
#include "BitLogic.h"					// useful macros for boolean logic
#include "GPIODriver.h"					// header for this module

void GPIOConfigPort(uint8_t port, gpioConfig_t *configPtr)
{
	// Set the desired pins for GPIO function.
	GPIOSetPortFunction(port, configPtr->functionRegister);
	
	// Set the port's direction register.
	GPIOSetPortDirection(port, configPtr->directionRegister);
	
	// Enable or disable pull-up registers (TRUE for pull-UP).
	GPIOSetPortPull(port, configPtr->pullRegister, TRUE);
	
	// Set the port's initial value.
	GPIOWritePort(port, configPtr->valueRegister);
}

void GPIOSetPortFunction(uint8_t port, GPIO_t mask)
{
	// Nothing to do here.  It's the other drivers' responsibility to set
	// override signals to GPIO pins, and set them back when they're done.
}

void GPIOSetPortDirection(uint8_t port, GPIO_t direction)
{
#ifdef DDRA
	if (port == 0)
		DDRA = direction;
#endif

#ifdef DDRB
	else if (port == 1)
		DDRB = direction;
#endif

#ifdef DDRC
	else if (port == 2)
		DDRC = direction;
#endif

#ifdef DDRD
	else if (port == 3)
		DDRD = direction;
#endif
}

/******************************************************************************
*	Function:		GPIOSetPortPull
*
*	Description:	Enables or disables pullup and pulldown resistors.
*					Note that for the AVR series, we can only pull up.
*
*	Parameters:		port - index of the port to act upon
*					mask - which bits to act upon
*					type - TRUE for pullup; FALSE for pulldown (ignored because
*						we don't have pulldown resistors)
******************************************************************************/
void GPIOSetPortPull(uint8_t port, GPIO_t mask, bool type)
{
#ifdef PUEA
	if (port == 0)
		PUEA = mask;
#endif

#ifdef PUEB
	else if (port == 1)
		PUEB = mask;
#endif

#ifdef PUEC
	else if (port == 2)
		PUEC = mask;
#endif

#ifdef PUED
	else if (port == 3)
		PUED = mask;
#endif
}

GPIO_t GPIOReadPort(uint8_t port)
{
	GPIO_t result;

#ifdef PINA
	if (port == 0)
		result = PINA;
#endif

#ifdef PINB
	else if (port == 1)
		result = PINB;
#endif

#ifdef PINC
	else if (port == 2)
		result = PINC;
#endif

#ifdef PIND
	else if (port == 3)
		result = PIND;
#endif

	else
		result = 0;
		
	return result;
}

void GPIOWritePort(uint8_t port, GPIO_t value)
{
#ifdef PORTA
	if (port == 0)
		PORTA = value;
#endif

#ifdef PORTB
	else if (port == 1)
		PORTB = value;
#endif

#ifdef PORTC
	else if (port == 2)
		PORTC = value;
#endif

#ifdef PORTD
	else if (port == 3)
		PORTD = value;
#endif
}

void GPIOTogglePort(uint8_t port, GPIO_t mask)
{
#ifdef PORTA
	if (port == 0)
		TOGGLEMASK(PORTA, mask);
#endif

#ifdef PORTB
	else if (port == 1)
		TOGGLEMASK(PORTB, mask);
#endif

#ifdef PORTC
	else if (port == 2)
		TOGGLEMASK(PORTC, mask);
#endif

#ifdef PORTD
	else if (port == 3)
		TOGGLEMASK(PORTD, mask);
#endif
}

uint8_t GPIOReadPin(uint8_t port, uint8_t pin)
{
#ifdef PINA
	if ((port == 0) && ISBITSET(PINA, pin))
		return TRUE;
#endif

#ifdef PINB
	else if ((port == 1) && ISBITSET(PINB, pin))
		return TRUE;
#endif

#ifdef PINC
	else if ((port == 2) && ISBITSET(PINC, pin))
		return TRUE;
#endif

#ifdef PIND
	else if ((port == 3) && ISBITSET(PIND, pin))
		return TRUE;
#endif

	else
		return FALSE;
}

void GPIOWritePin(uint8_t port, uint8_t pin, uint8_t value)
{
#ifdef PORTA
	if (port == 0)
		(value) ? SETBIT(PORTA, pin) : CLEARBIT(PORTA, pin);
#endif

#ifdef PORTB
	else if (port == 1)
		(value) ? SETBIT(PORTB, pin) : CLEARBIT(PORTB, pin);
#endif

#ifdef PORTC
	else if (port == 2)
		(value) ? SETBIT(PORTC, pin) : CLEARBIT(PORTC, pin);
#endif

#ifdef PORTD
	else if (port == 3)
		(value) ? SETBIT(PORTD, pin) : CLEARBIT(PORTD, pin);
#endif
}

void GPIOTogglePin(uint8_t port, uint8_t pin)
{
#ifdef PORTA
	if (port == 0)
		TOGGLEBIT(PORTA, pin);
#endif

#ifdef PORTB
	else if (port == 1)
		TOGGLEBIT(PORTB, pin);
#endif

#ifdef PORTC
	else if (port == 2)
		TOGGLEBIT(PORTC, pin);
#endif

#ifdef PORTD
	else if (port == 3)
		TOGGLEBIT(PORTD, pin);
#endif
}