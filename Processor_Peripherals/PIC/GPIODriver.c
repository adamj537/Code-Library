/******************************************************************************
 *
 *	Filename:		GPIODriver.c
 *
 *	Description:	Driver for GPIO functions for PIC microcontrollers.
 *					GPIO on the PIC is accessed via three registers:
 *					TRISx - sets the pin direction; 1 = input, 0 = output
 *					LATx - sets pin output; 0 = drive low, 1 = drive high
 *					PORTx - reads pin status; 0 = low, 1 = high
 *
 *					Some ports also have these registers:
 *					ANSELx - analog mode enable; 0 = disabled, 1 = enabled
 *					WPUx - weak pullup enable; 0 = disabled, 1 = enabled
 *					INLVLx - logic level select; 1 = CMOS, 0 = TTL
 *
 *	Notes:			THIS FILE HAS NOT BEEN TESTED!
 *
 *					On reset, pins are input (TRISx = 1)
 *
 *					A write operation to the LATx register has the same effect
 *					as a write to the corresponding PORTx register.  A read of
 *					the LATx register reads of the values held in the I/O PORT
 *					latches, while a read of the PORTx register reads the actual
 *					I/O pin value.
 *
 *					As a rule of thumb, always read inputs from PORTx and write
 *					outputs to LATx.  If you need to read what you set an output
 *					to, read LATx (source:  DarioG of microchip.com/forums).
 *
 *****************************************************************************/

#include <xc.h>							// pin register definitions
#include <stdint.h>						// universal data types
#include <stdbool.h>					// defines boolean type
#include "BitLogic.h"					// useful macros for boolean logic
#include "GPIODriver.h"					// header for this module

void GPIOConfigurePort(uint8_t port, gpioConfig_t *configPtr)
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
	// Use the ANSELx register to disable analog function on selected pins.
	// Setting a bit to zero enables GPIO function.
#ifdef ANSELA
	if (port == 0)
		ANSELA = mask;
#endif

#ifdef ANSELB
	else if (port == 1)
		ANSELB = mask;
#endif

#ifdef ANSELC
	else if (port == 2)
		ANSELC = mask;
#endif

#ifdef ANSELD
	else if (port == 3)
		ANSELD = mask;
#endif
}

void GPIOSetPortDirection(uint8_t port, GPIO_t direction)
{
#ifdef TRISA
	if (port == 0)
		TRISA = direction;
#endif

#ifdef TRISB
	else if (port == 1)
		TRISB = direction;
#endif

#ifdef TRISC
	else if (port == 2)
		TRISC = direction;
#endif

#ifdef TRISD
	else if (port == 3)
		TRISD = direction;
#endif
}

/******************************************************************************
*	Function:		GPIOSetPortPull
*
*	Description:	Enables or disables pullup and pulldown resistors.
*					Note that for the PIC series, we can only pull up.
*
*	Parameters:		port - index of the port to act upon
*					mask - which bits to act upon
*					type - TRUE for pullup; FALSE for pulldown (ignored because
*						we don't have pulldown resistors)
******************************************************************************/

void GPIOSetPortPull(uint8_t port, GPIO_t mask, bool type)
{
#ifdef WPUA
	if (port == 0)
		WPUA = mask;
#endif

#ifdef WPUB
	else if (port == 1)
		WPUB = mask;
#endif

#ifdef WPUC
	else if (port == 2)
		WPUC = mask;
#endif

#ifdef WPUD
	else if (port == 3)
		WPUD = mask;
#endif
}

GPIO_t GPIOReadPort(uint8_t port)
{
	GPIO_t result;
	
#ifdef PORTA
	if (port == 0)
		result = PORTA;
#endif

#ifdef PORTB
	else if (port == 1)
		result = PORTB;
#endif

#ifdef PORTC
	else if (port == 2)
		result = PORTC;
#endif

#ifdef PORTD
	else if (port == 3)
		result = PORTD;
#endif

	else
		result = 0;
	
	return result;
}

void GPIOWritePort(uint8_t port, GPIO_t value)
{
#ifdef LATA
	if (port == 0)
		LATA = value;
#endif

#ifdef LATB
	else if (port == 1)
		LATB = value;
#endif

#ifdef LATC
	else if (port == 2)
		LATC = value;
#endif

#ifdef LATD
	else if (port == 2)
		LATC = value;
#endif
}

void GPIOTogglePort(uint8_t port, GPIO_t mask)
{
#ifdef LATA
	if (port == 0)
		TOGGLEMASK(LATA, mask);
#endif

#ifdef LATB
	else if (port == 1)
		TOGGLEMASK(LATB, mask);
#endif

#ifdef LATC
	else if (port == 2)
		TOGGLEMASK(LATC, mask);
#endif

#ifdef LATD
	else if (port == 3)
		TOGGLEMASK(LATD, mask);
#endif
}

uint8_t GPIOReadPin(uint8_t port, uint8_t pin)
{
#ifdef PORTA
	if ((port == 0) && ISBITSET(PORTA, pin))
		return TRUE;
#endif

#ifdef PORTB
	else if ((port == 1) && ISBITSET(PORTB, pin))
		return TRUE;
#endif

#ifdef PORTC
	else if ((port == 2) && ISBITSET(PORTC, pin))
		return TRUE;
#endif

#ifdef PORTD
	else if ((port == 3) && ISBITSET(PORTD, pin))
		return TRUE;
#endif

	else
		return FALSE;
}

void GPIOWritePin(uint8_t port, uint8_t pin, uint8_t value)
{
#ifdef LATA
	if (port == 0)
		(value) ? SETBIT(LATA, pin) : CLEARBIT(LATA, pin);
#endif

#ifdef LATB
	else if (port == 1)
		(value) ? SETBIT(LATB, pin) : CLEARBIT(LATB, pin);
#endif

#ifdef LATC
	else if (port == 2)
		(value) ? SETBIT(LATC, pin) : CLEARBIT(LATC, pin);
#endif

#ifdef LATD
	else if (port == 3)
		(value) ? SETBIT(LATD, pin) : CLEARBIT(LATD, pin);
#endif
}

void GPIOTogglePin(uint8_t port, uint8_t pin)
{
#ifdef LATA
	if (port == 0)
		TOGGLEBIT(LATA, pin);
#endif

#ifdef LATB
	else if (port == 1)
		TOGGLEBIT(LATB, pin);
#endif

#ifdef LATC
	else if (port == 2)
		TOGGLEBIT(LATC, pin);
#endif

#ifdef LATD
	else if (port == 3)
		TOGGLEBIT(LATD, pin);
#endif
}