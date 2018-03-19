/******************************************************************************
 *
 *	Filename:		GPIODriver.h
 *
 *	Author:			Adam Johnson
 *
 *  Description:	Driver for GPIO functions
 *
 *	Notes:			The direction register is opposite between PIC and AVR.
 *					For AVR, 0 = input.  For PIC, 0 = output.
 *					So the PIC version of this driver inverts this register.
 *
 *	Terms of Use:	MIT License
 *
 *****************************************************************************/

#ifndef GPIODRIVER_H
#define GPIODRIVER_H

#include <stdint.h>						// universal data types
#include <stdbool.h>					// defines "bool"

#ifndef GPIO_PORT_IS_DEFINED			// ensure port size is defined
#error "GPIO_t should be typedefed in your project."
#warning "like this:  typedef uint8_t GPIO_t"
#warning "Define GPIO_PORT_IS_DEFINED too."
#endif

typedef struct							// port settings structure
{
	GPIO_t function;					// port function register value
	GPIO_t direction;					// port direction register; 0 = input
	GPIO_t value;						// port value register
	GPIO_t pull;						// pin pull up/down register
	bool pullType;						// FALSE = down, TRUE = up
} gpioConfig_t;

// Initialize GPIO port's clock if necessary.
void GPIOInit(uint8_t port);

// Port-oriented Configuration Functions
void GPIOConfigPort(uint8_t port, gpioConfig_t *configPtr);	// Set everything.
void GPIOSetPortFunction(uint8_t port, GPIO_t mask);		// Set as GPIO.
void GPIOSetPortDirection(uint8_t port, GPIO_t direction);	// Set direction.
void GPIOSetPortPull(uint8_t port, GPIO_t mask, bool type);	// Set pull up/down.

// Port-oriented read/write functions
GPIO_t GPIOReadPort(uint8_t port);							// Read a port.
void GPIOWritePort(uint8_t port, GPIO_t value);				// Write to a port.
void GPIOTogglePort(uint8_t port, GPIO_t mask);				// Toggle a port.

// Pin-oriented Configuration Functions
void GPIOConfigPin(uint8_t port, gpioConfig_t *configPtr);	// Set everything.
void GPIOSetPinFunction(uint8_t port, uint8_t pin, bool val);
void GPIOSetPinDirection(uint8_t port, uint8_t pin, bool direction);
void GPIOSetPinPull(uint8_t port, uint8_t pin, bool type);	// Set pull up/down.

// Pin-oriented functions
bool GPIOReadPin(uint8_t port, uint8_t pin);				// Read a pin.
void GPIOWritePin(uint8_t port, uint8_t pin, uint8_t value);// Write a pin.
void GPIOTogglePin(uint8_t port, uint8_t pin);				// Toggle a pin.

#endif /* GPIODRIVER_H */