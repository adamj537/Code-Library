/******************************************************************************
 *
 *	Filename:		GPIODriver.h
 *
 *  Description:	Driver for GPIO functions
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
	GPIO_t pull;						// pin pullup/down register
	bool pullType;						// FALSE = down, TRUE = up
} gpioConfig_t;

// Configuration Functions
void GPIOConfigPort(uint8_t port, gpioConfig_t *configPtr);	// Set everything.
void GPIOSetPortFunction(uint8_t port, GPIO_t mask);		// Set as GPIO.
void GPIOSetPortDirection(uint8_t port, GPIO_t direction);	// Set direction.
void GPIOSetPortPull(uint8_t port, GPIO_t mask, bool type);	// Set pullup/down.

// Port-oriented functions
GPIO_t GPIOReadPort(uint8_t port);							// Read a port.
void GPIOWritePort(uint8_t port, GPIO_t value);				// Write to a port.
void GPIOTogglePort(uint8_t port, GPIO_t mask);				// Toggle a port.

// Pin-oriented functions
bool GPIOReadPin(uint8_t port, uint8_t pin);				// Read a pin.
void GPIOWritePin(uint8_t port, uint8_t pin, uint8_t value);// Write a pin.
void GPIOTogglePin(uint8_t port, uint8_t pin);				// Toggle a pin.

#endif /* GPIODRIVER_H */