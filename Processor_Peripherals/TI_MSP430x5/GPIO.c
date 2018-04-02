/******************************************************************************
 *
 *	Filename:		MSP430_GPIO.c
 *
 *	Author:			Adam Johnson
 *
 *	Description:	Contains functions to control GPIO for an MSP430.  Tested
 *					on an MSP430F5359, but should work on all MSP430x5 and
 *					MSP43x6 at least.  Note that some devices don't have
 *					pullup/pulldown resistors, or the ability to interrupt on
 *					certain ports.  This module is based on MSP430Ware gpio
 *					library for MSP430x5.
 *
 *	Notes:			In this family (MSP430F5xx/6xx), ports can be read/written
 *					as bytes (P1, P2, P3, ... Pn) or as words (P1 + P2 = PA,
 *					P3 + P4 = PB).  In the HAL, GPIO_PORTA corresponds to
 *					MSP430's P1, GPIO_PORTB corresponds to P2, etc, and you
 *					must access the ports as bytes (although internally I'm
 *					using word access).
 *
 *					Also, drive strength is configurable for each pin on many
 *					devices, but this HAL does not include that as a
 *					configurable option, so the user must set it elsewhere.
 *
 *					PxIN, PxOUT, PxDIR, and PxSEL are present for all ports on
 *					all devices.  Other registers may not be (for example,
 *					MSP430F1611 does not have internal pullup/down resistors,
 *					and on most devices, only port 1 and 2 have interrupt
 *					capability.  Here's a list of available registers:
 *
 *					PxIN  - input mode: value of pin (read only; 0 = low)
 *					PxOUT - output mode:  value of pin (0 = low)
 *							input mode:  pin pullup/down selection (0 = down)
 *					PxDIR - pin direction (0 = input)
 *					PxSEL - pin function (0 = GPIO; does not set pin direction)
 *					PxREN - pullup/down resistor enable (0 = disabled)
 *					PxDS  - pin drive strength (0 = reduced, 1 = full)
 *					PxIFG - pin interrupt flags (1 = pending interrupt)
 *					PxIE  - pin interrupt enable bits (0 = disabled)
 *					PxIES - pin interrupt edge select (0 = low-to-high)
 *
 *					Unused pins should be configured as GPIO, output direction,
 *					and left unconnected on the PCB.  PxOUT is don't care.
 *
 *	Terms of Use:	MIT License
 *
 *****************************************************************************/

#include <msp430.h>						// required by development platform
#include <stdint.h>						// compiler-specific data types
#include "project.h"					// global settings; driver configuration
#include "Bitlogic.h"					// handy macros that make life easier
#include "GPIO.h"						// header for this module

#define MAX_GPIO_INT_PORT	4			// only ports 1-4 can interrupt
#define MAX_GPIO_PIN		8			// size of each port (8 bits)

// Macros for hardware access
#define HWREG32(x)			(*((volatile uint32_t *)((uint16_t)x)))
#define HWREG16(x)			(*((volatile uint16_t *)((uint16_t)x)))
#define HWREG8(x)			(*((volatile uint8_t *)((uint16_t)x)))

// GPIO Register Offsets
#define GPIO_REG_IN			0x0000		// Input Register
#define GPIO_REG_OUT		0x0002		// Output Register
#define GPIO_REG_DIR		0x0004		// Direction Register
#define GPIO_REG_REN		0x0006		// Resistor Enable Register
#define GPIO_REG_DS			0x0008		// Drive Strength Register
#define GPIO_REG_SEL		0x000A		// Function Selection Register
#define GPIO_REG_IES		0x0018		// Interrupt Edge Select Register
#define GPIO_REG_IE			0x001A		// Interrupt Enable Register
#define GPIO_REG_IFG		0x001C		// Interrupt Flag Register

/*****************************************************************************
** GLOBAL VARIABLES
*****************************************************************************/

/* MSP430s differ in the number of GPIO ports they have.  This array contains
 * the base address of the registers of all available ports on the device in
 * use.  It is generated at compile-time, and used below to find the registers
 * of the port(s) in use.
 */
static const uint16_t gpioPortToBaseAddress[] = {
#if defined(__MSP430_HAS_PORT1_R__)
    __MSP430_BASEADDRESS_PORT1_R__,
#elif defined(__MSP430_HAS_PORT1__)
    __MSP430_BASEADDRESS_PORT1__,
#else
    0xFFFF,
#endif
#if defined(__MSP430_HAS_PORT2_R__)
    __MSP430_BASEADDRESS_PORT2_R__,
#elif defined(__MSP430_HAS_PORT2__)
    __MSP430_BASEADDRESS_PORT2__,
#else
    0xFFFF,
#endif
#if defined(__MSP430_HAS_PORT3_R__)
    __MSP430_BASEADDRESS_PORT3_R__,
#elif defined(__MSP430_HAS_PORT3__)
    __MSP430_BASEADDRESS_PORT3__,
#else
    0xFFFF,
#endif
#if defined(__MSP430_HAS_PORT4_R__)
    __MSP430_BASEADDRESS_PORT4_R__,
#elif defined(__MSP430_HAS_PORT4__)
    __MSP430_BASEADDRESS_PORT4__,
#else
    0xFFFF,
#endif
#if defined(__MSP430_HAS_PORT5_R__)
    __MSP430_BASEADDRESS_PORT5_R__,
#elif defined(__MSP430_HAS_PORT5__)
    __MSP430_BASEADDRESS_PORT5__,
#else
    0xFFFF,
#endif
#if defined(__MSP430_HAS_PORT6_R__)
    __MSP430_BASEADDRESS_PORT6_R__,
#elif defined(__MSP430_HAS_PORT6__)
    __MSP430_BASEADDRESS_PORT6__,
#else
    0xFFFF,
#endif
#if defined(__MSP430_HAS_PORT7_R__)
    __MSP430_BASEADDRESS_PORT7_R__,
#elif defined(__MSP430_HAS_PORT7__)
    __MSP430_BASEADDRESS_PORT7__,
#else
    0xFFFF,
#endif
#if defined(__MSP430_HAS_PORT8_R__)
    __MSP430_BASEADDRESS_PORT8_R__,
#elif defined(__MSP430_HAS_PORT8__)
    __MSP430_BASEADDRESS_PORT8__,
#else
    0xFFFF,
#endif
#if defined(__MSP430_HAS_PORT9_R__)
    __MSP430_BASEADDRESS_PORT9_R__,
#elif defined(__MSP430_HAS_PORT9__)
    __MSP430_BASEADDRESS_PORT9__,
#else
    0xFFFF,
#endif
#if defined(__MSP430_HAS_PORT10_R__)
    __MSP430_BASEADDRESS_PORT10_R__,
#elif defined(__MSP430_HAS_PORT10__)
    __MSP430_BASEADDRESS_PORT10__,
#else
    0xFFFF,
#endif
#if defined(__MSP430_HAS_PORT11_R__)
    __MSP430_BASEADDRESS_PORT11_R__,
#elif defined(__MSP430_HAS_PORT11__)
    __MSP430_BASEADDRESS_PORT11__,
#else
    0xFFFF,
#endif
    0xFFFF,
#if defined(__MSP430_HAS_PORTJ_R__)
    __MSP430_BASEADDRESS_PORTJ_R__
#elif defined(__MSP430_HAS_PORTJ__)
    __MSP430_BASEADDRESS_PORTJ__
#else
    0xFFFF
#endif
};

/* Some MSP430 GPIO ports have configurable pull-up/down resistors; others do
 * not.  This array contains flags (set at compile-time) which tell whether
 * each port has these resistors (and the associated registers) or not.
 */
static const uint8_t gpioPortHasResistors[] = {
#if defined(__MSP430_HAS_PORT1_R__)
    TRUE,
#else
    FALSE,
#endif
#if defined(__MSP430_HAS_PORT2_R__)
    TRUE,
#else
    FALSE,
#endif
#if defined(__MSP430_HAS_PORT3_R__)
    TRUE,
#else
    FALSE,
#endif
#if defined(__MSP430_HAS_PORT4_R__)
    TRUE,
#else
    FALSE,
#endif
#if defined(__MSP430_HAS_PORT5_R__)
    TRUE,
#else
    FALSE,
#endif
#if defined(__MSP430_HAS_PORT6_R__)
    TRUE,
#else
    FALSE,
#endif
#if defined(__MSP430_HAS_PORT7_R__)
    TRUE,
#else
    FALSE,
#endif
#if defined(__MSP430_HAS_PORT8_R__)
    TRUE,
#else
    FALSE,
#endif
#if defined(__MSP430_HAS_PORT9_R__)
    TRUE,
#else
    FALSE,
#endif
#if defined(__MSP430_HAS_PORT10_R__)
    TRUE,
#else
    FALSE,
#endif
#if defined(__MSP430_HAS_PORT11_R__)
    TRUE,
#else
    FALSE,
#endif
    FALSE,
#if defined(__MSP430_HAS_PORTJ_R__)
    TRUE,
#else
    FALSE
#endif
};

// Flags for "GPIO_INT_EDGE_BOTH" software emulation (one bit per pin).
static uint8_t GPIOIntDetectEdgeBothFlags[MAX_GPIO_INT_PORT];

// Callback functions for interrupts
static GPIOIntCallback_t GPIOCallbackFuncs[MAX_GPIO_INT_PORT][MAX_GPIO_PIN];

/*
* @brief Initialize the selected GPIO port
* @param[in]  port The index/identifier of the GPIO port to initialize
* @return GPIO_RESULT_OK on success, GPIO_RESULT_FAIL on failure
*/
GPIOResult_t GPIOInit(GPIOPort_t port)
{
	// TODO:	Perhaps this should set all GPIO according to recommendations
	//			for unused pins?

	return GPIO_RESULT_OK;
}

/*
* @brief Uninitialize the selected GPIO port
* @param[in]  port The index/identifier of the GPIO port to uninitialize.
* @return GPIO_RESULT_OK on success, GPIO_RESULT_FAIL on failure
*/
GPIOResult_t GPIODeinit(GPIOPort_t port)
{
	// TODO:	Perhaps this should set all GPIO according to recommendations
	//			for unused pins?

	return GPIO_RESULT_OK;
}

/*
* @brief Configure the pins specified by mask in the selected port
* @param[in]  port The index/identifier of the GPIO port to configure
* @param[in]  mask The specific pins within the port to configure
* @param[in]  configPtr A pointer to a sGpioConfig_t
* @return GPIO_RESULT_OK on success, GPIO_RESULT_INVALID_SELECTION on failure
*/
GPIOResult_t GPIOConfigPort(GPIOPort_t port, GPIOPortSize_t mask, GPIOConfig_t *configPtr)
{
	GPIOResult_t result = GPIO_RESULT_INVALID_SELECTION;	// return value from function
	uint16_t baseAddress;								// address of port's registers
	uint16_t wordMask;									// mask converted to work with 16-bit registers

	// Fetch the base address of the port's registers.
	if (port < mathUtils_ArraySize(gpioPortToBaseAddress, uint16_t))
		baseAddress = gpioPortToBaseAddress[port];
	else
		baseAddress = 0xFFFF;
	
	// Check for invalid ports.
	if(baseAddress != 0xFFFF)
	{
		// Shift by 8 if port is odd (upper 8-bits).
		if (port & 1)
		{
			wordMask = mask << 8;
		}
		// Don't alter the mask if the port is even.
		else
		{
			wordMask = mask;
		}

		// PowerSave Mode overrides other settings.
		if (configPtr->ui32_Powersave == TRUE)
		{
			//Configure the unused pin as GPIO.
			CLEARMASK(HWREG16(baseAddress + GPIO_REG_SEL), wordMask);

			// Set direction to output.
			CLEARMASK(HWREG16(baseAddress + GPIO_REG_DIR), wordMask);

			// PxOUT is don't care (I choose to put pins low).
			CLEARMASK(HWREG16(baseAddress + GPIO_REG_OUT), wordMask);
		}
		else
		{
			// Set the pins' function.
			if (configPtr->ui32_MuxPosition == 0)
			{
				CLEARMASK(HWREG16(baseAddress + GPIO_REG_SEL), wordMask);
			}
			else
			{
				SETMASK(HWREG16(baseAddress + GPIO_REG_SEL), wordMask);
			}

			// Set the pin direction.
			if (configPtr->ui32_Direction == GPIO_DIR_IN)
			{
				// Set the pin as input.
				CLEARMASK(HWREG16(baseAddress + GPIO_REG_DIR), wordMask);
			}
			else
			{
				// Set the pin as output.
				SETMASK(HWREG16(baseAddress + GPIO_REG_DIR), wordMask);
			}

			// Check if pull-up/down resistors exist on the port.
			if (gpioPortHasResistors[port] == TRUE)
			{
				// Enable pull-up/down resistors if requested.
				if (configPtr->ui32_InputPull == GPIO_PULL_NONE)
				{
					// Disable resistors.
					CLEARMASK(HWREG16(baseAddress + GPIO_REG_REN), wordMask);
				}
				else if (configPtr->ui32_InputPull == GPIO_PULL_UP)
				{
					// Enable resistors.
					SETMASK(HWREG16(baseAddress + GPIO_REG_REN), wordMask);

					// Select pull-up direction.
					SETMASK(HWREG16(baseAddress + GPIO_REG_OUT), wordMask);
				}
				else if (configPtr->ui32_InputPull == GPIO_PULL_DOWN)
				{
					// Enable resistors.
					SETMASK(HWREG16(baseAddress + GPIO_REG_REN), wordMask);

					// Select pull-down direction.
					CLEARMASK(HWREG16(baseAddress + GPIO_REG_OUT), wordMask);
				}
			}

			// Clear interrupt flag register.  According to the user manual,
			// writing to PxOUT, PxDIR, PxREN can result in setting the
			// corresponding PxIFG flag.
			if (port < MAX_GPIO_INT_PORT)
			{
				CLEARMASK(HWREG16(baseAddress + GPIO_REG_IFG), wordMask);
			}
		}

		result = GPIO_RESULT_OK;
	}
	
	return result;
}

/*
* @brief Configure a single pin in the specified port.
* @param[in]  port The index/identifier of the GPIO port the pin belongs to.
* @param[in]  pin The index/identifier of the GPIO pin to configure
* @param[in]  configPtr A pointer to a sGpioConfig_t
* @return GPIO_RESULT_OK on success, GPIO_RESULT_INVALID_SELECTION on failure
*/
GPIOResult_t GPIOConfigPin(GPIOPort_t port, GPIOPortSize_t pin, GPIOConfig_t *configPtr)
{
	GPIOResult_t result = GPIO_RESULT_INVALID_SELECTION;	// return value

	// Check for valid pin.
	if (pin < MAX_GPIO_PIN)
	{
		result = GPIOConfigPort(port, BV(pin), configPtr);
	}

	return result;
}

/*
 * @brief Read the state of the pins in the specified port with respect to the mask
 * that are configured as inputs.
 * @param[in]  port The index/identifier of the GPIO port to read
 * @param[in]  mask A bit mask of pins to read from the port
 * @return The value of the pins of the port in the mask that are configured as inputs
 */
GPIOPortSize_t GPIOReadPort(GPIOPort_t port, GPIOPortSize_t mask)
{
	GPIOPortSize_t portValue = 0;	// return value
	uint16_t baseAddress;				// address of port's registers
	uint16_t wordMask;					// mask converted to work with 16-bit registers

	// Fetch the base address of the port's registers.
	if (port < mathUtils_ArraySize(gpioPortToBaseAddress, uint16_t))
		baseAddress = gpioPortToBaseAddress[port];
	else
		baseAddress = 0xFFFF;

	// Check for invalid ports.
	if(baseAddress != 0xFFFF)
	{
		// Shift by 8 if port is odd (upper 8-bits).
//		if((port & 1) ^ 1)
		if (port & 1)
		{
			wordMask = mask << 8;
		}
		// Don't alter the mask if the port is even.
		else
		{
			wordMask = mask;
		}

		// Fetch the value of the bits (and mask them according to user input).
		portValue = ISMASKSET(HWREG16(baseAddress + GPIO_REG_IN), wordMask);
	}
	
	return portValue;
}

/*
 * @brief Read the state of the pin specified (if input)
 * @param[in]  port The index/identifier of the GPIO port to read
 * @param[in]  pin The index/identifier of the GPIO pin to read
 * @return The value of the pin (if input)
 */
uint8_t GPIOReadPin(GPIOPort_t port, GPIOPortSize_t pin)
{
	GPIOPortSize_t portValue = 0;	// value of port (masked)
	uint8_t pinValue = 0;			// return value

	// Check for valid pin.
	if (pin < MAX_GPIO_PIN)
	{
		portValue = GPIOReadPort(port, BV(pin));
	}

	pinValue = ISBITSET(portValue, pin);

	return pinValue;
}

/*
 * @brief Write the value of the pins in the specified port with respect to the mask
 *  that are configured as outputs.
 * @param[in] port The index/identifier of the GPIO port to write
 * @param[in] mask A bit mask of pins to write in the port
 * @param[in] ui8_portValue The value to write to the masked pins.
 * @return GPIO_RESULT_OK on success, GPIO_RESULT_INVALID_SELECTION on failure
 */
GPIOResult_t GPIOWritePort(GPIOPort_t port, GPIOPortSize_t mask, uint8_t ui8_portValue)
{
	GPIOResult_t result = GPIO_RESULT_INVALID_SELECTION;
	uint16_t baseAddress;				// address of port's registers
	uint16_t wordMask;					// mask converted to work with 16-bit registers

	// Fetch the base address of the port's registers.
	if (port < mathUtils_ArraySize(gpioPortToBaseAddress, uint16_t))
		baseAddress = gpioPortToBaseAddress[port];
	else
		baseAddress = 0xFFFF;

	// Check for invalid ports.
	if(baseAddress != 0xFFFF)
	{
		// Shift by 8 if port is odd (upper 8-bits).
		if (port & 1)
		{
			wordMask = mask << 8;
		}
		// Don't alter the mask if the port is even.
		else
		{
			wordMask = mask;
		}

		// Should we clear or set bits in the mask?
		if (ui8_portValue == 0)
		{
			// Write 0 to selected bits.
			CLEARMASK(HWREG16(baseAddress + GPIO_REG_OUT), wordMask);
		}
		else
		{
			// Write 1 to selected bits.
			SETMASK(HWREG16(baseAddress + GPIO_REG_OUT), wordMask);
		}

		result = GPIO_RESULT_OK;
	}
		
	return result;
}

/*
 * @brief Write the value of the pin specified (if an output)
 * @param[in] port The index/identifier of the GPIO port to write
 * @param[in] pin The index/identifier of the GPIO pin to write
 * @param[in] ui8_pinValue The value to write to the pin.
 * @return GPIO_RESULT_OK on success, GPIO_RESULT_INVALID_SELECTION on failure
 */
GPIOResult_t GPIOWritePin(GPIOPort_t port, GPIOPortSize_t pin, uint8_t pinValue)
{
	GPIOResult_t result = GPIO_RESULT_INVALID_SELECTION;	// return value

	// Check for valid pin.
	if (pin < MAX_GPIO_PIN)
	{
		result = GPIOWritePort(port, BV(pin), pinValue);
	}

	return result;
}

/*
 * @brief Toggle the state of the pins in the specified port with respect to the mask
 * that are configured as outputs.
 * @param[in] port The index/identifier of the GPIO port to Toggle
 * @param[in] mask A bit mask of pins to toggle on the port
 * @return GPIO_RESULT_OK on success, GPIO_RESULT_INVALID_SELECTION on failure
 */
GPIOResult_t GPIOTogglePort(GPIOPort_t port, GPIOPortSize_t mask)
{
	GPIOResult_t result = GPIO_RESULT_INVALID_SELECTION;
	uint16_t baseAddress;				// address of port's registers
	uint16_t wordMask;					// mask converted to work with 16-bit registers

	// Fetch the base address of the port's registers.
	if (port < mathUtils_ArraySize(gpioPortToBaseAddress, uint16_t))
		baseAddress = gpioPortToBaseAddress[port];
	else
		baseAddress = 0xFFFF;

	// Check for invalid ports.
	if(baseAddress != 0xFFFF)
	{
		// Shift by 8 if port is odd (upper 8-bits).
		if (port & 1)
		{
			wordMask = mask << 8;
		}
		// Don't alter the mask if the port is even.
		else
		{
			wordMask = mask;
		}

		// Toggle selected bits.
		TOGGLEMASK(HWREG16(baseAddress + GPIO_REG_OUT), wordMask);

		result = GPIO_RESULT_OK;
	}

	return result;
}

/*
 * @brief Toggle the state of the pin specified (if configured as output)
 * @param[in] port The index/identifier of the GPIO port to Toggle
 * @param[in] pin The index/identifier of the GPIO pin to Toggle
 * @return GPIO_RESULT_OK on success, GPIO_RESULT_INVALID_SELECTION on failure
 */
GPIOResult_t GPIOTogglePin(GPIOPort_t port, GPIOPortSize_t pin)
{
	GPIOResult_t result = GPIO_RESULT_INVALID_SELECTION;	// return value

	// Check for valid pin.
	if (pin < MAX_GPIO_PIN)
	{
		result = GPIOTogglePort(port, BV(pin));
	}

	return result;
}

/*
 * @brief Configure a GPIO pin as an external interrupt source and register a callback function or unregister
 *  a previously registered callback function.
 * @param[in] port The index/identifier of the GPIO port to configure.
 * @param[in] eGP_gpioPin The index/identifier of the GPIO pin to configure.
 * @param[in] sPtr_gpioIntConfig A pointer to a sGPIOIntConfig_t structure with the desired pin/interrupt configuration
 * @param[in] funcPtr_GpioCallback A pointer to a function that will receive the interrupt, or NULL to unregister the callback function
 * @return GPIO_RESULT_OK on success, GPIO_RESULT_FAIL on failure
 */
GPIOResult_t GPIOConfigInterrupt(GPIOPort_t port, GPIOPortSize_t gpioPin, sGPIOIntConfig_t *sPtr_gpioIntConfig, GPIOIntCallback_t funcPtr_GpioCallback)
{
	GPIOResult_t result = GPIO_RESULT_OK;	// return value from function
	uint16_t baseAddress;					// address of port's registers
	uint16_t wordMask;						// mask of port's pins, converted to work with 16-bit registers

	// Check for valid pin.
	if (gpioPin < MAX_GPIO_PIN)
	{
		wordMask = BV(eGP_gpioPin);
	}
	else
	{
		result = GPIO_RESULT_INVALID_SELECTION;
	}
	
	// Fetch the base address of the port's registers.
	if (port < MAX_GPIO_INT_PORT)
		baseAddress = gpioPortToBaseAddress[port];
	else
		baseAddress = 0xFFFF;

	// Check for invalid ports.
	if ((baseAddress != 0xFFFF) && (result == GPIO_RESULT_OK))
	{
		// Shift by 8 if port is odd (upper 8-bits).
		if (port & 1)
		{
			wordMask <<= 8;
		}

		// Store the new callback.
		GPIOCallbackFuncs[port][gpioPin] = gpioCallback;

		// Set the pin's function.
		if (sPtr_gpioIntConfig->ui32_MuxPosition == 0)
		{
			CLEARMASK(HWREG16(baseAddress + GPIO_REG_SEL), wordMask);
		}
		else
		{
			SETMASK(HWREG16(baseAddress + GPIO_REG_SEL), wordMask);
		}

		// Set the pin as input.
		CLEARMASK(HWREG16(baseAddress + GPIO_REG_DIR), wordMask);

		// Enable pull-up/down resistors if requested.
		if (sPtr_gpioIntConfig->ui32_InputPull == GPIO_PULL_NONE)
		{
			// Disable resistors.
			CLEARMASK(HWREG16(baseAddress + GPIO_REG_REN), wordMask);
		}
		else if (sPtr_gpioIntConfig->ui32_InputPull == GPIO_PULL_UP)
		{
			// Enable resistors.
			SETMASK(HWREG16(baseAddress + GPIO_REG_REN), wordMask);

			// Select pull-up direction.
			SETMASK(HWREG16(baseAddress + GPIO_REG_OUT), wordMask);
		}
		else if (sPtr_gpioIntConfig->ui32_InputPull == GPIO_PULL_DOWN)
		{
			// Enable resistors.
			SETMASK(HWREG16(baseAddress + GPIO_REG_REN), wordMask);

			// Select pull-down direction.
			CLEARMASK(HWREG16(baseAddress + GPIO_REG_OUT), wordMask);
		}
		else
		{
			result = GPIO_RESULT_INVALID_SELECTION;
		}

		// Select the interrupt edge transition.
		// Rising and falling edge are supported directly.
		// Both edges (at the same time) are supported via software emulation.
		// Level interrupts are not supported.
		if (sPtr_gpioIntConfig->ui32_DetectionCriteria == GPIO_INT_EDGE_RISING)
		{
			// Set rising edge for interrupt.
			CLEARMASK(HWREG16(baseAddress + GPIO_REG_IES), wordMask);

			// Clear the "detect both edges" flag for the pin.
			CLEARMASK(GPIOIntDetectEdgeBothFlags[port], BV(eGP_gpioPin));
		}
		else if (sPtr_gpioIntConfig->ui32_DetectionCriteria == GPIO_INT_EDGE_FALLING)
		{
			// Set falling edge for interrupt.
			SETMASK(HWREG16(baseAddress + GPIO_REG_IES), wordMask);

			// Clear the "detect both edges" flag for the pin.
			CLEARMASK(GPIOIntDetectEdgeBothFlags[port], BV(eGP_gpioPin));
		}
		else if (sPtr_gpioIntConfig->ui32_DetectionCriteria == GPIO_INT_EDGE_BOTH)
		{
			// Set the "detect both edges" flag for the pin.
			SETMASK(GPIOIntDetectEdgeBothFlags[port], BV(eGP_gpioPin));
		}
		else
		{
			result = GPIO_RESULT_INVALID_SELECTION;
		}
	}
	else
	{
		result = GPIO_RESULT_INVALID_SELECTION;
	}
		
	return result;
}

// TODO:  Add register and unregister functions for interrupts.

/*
 * @brief Enable the interrupt for the specified pin. (Must be configured first!)
 * @param[in] port The index/identifier of the GPIO port to enable the interrupt for.
 * @param[in] pin The index/identifier of the GPIO pin to enable the interrupt for.
 * @return GPIO_RESULT_OK on success, GPIO_RESULT_INVALID_SELECTION on failure.
 */
GPIOResult_t GPIOEnableInterrupt(GPIOPort_t port, GPIOPortSize_t pin)
{
	GPIOResult_t result = GPIO_RESULT_INVALID_SELECTION;	// return value from function
	uint16_t baseAddress;								// address of port's registers
	uint16_t wordMask;									// mask of port's pins, converted to work with 16-bit registers

	// Check for valid pin.
	if (pin < MAX_GPIO_PIN)
	{
		// Convert the pin to its bitmask.
		wordMask = BV(pin);

		// Fetch the base address of the port's registers.
		if (port < mathUtils_ArraySize(gpioPortToBaseAddress, uint16_t))
			baseAddress = gpioPortToBaseAddress[port];
		else
			baseAddress = 0xFFFF;

		// Check for invalid ports.
		if(baseAddress != 0xFFFF)
		{
			// Shift by 8 if port is odd (upper 8-bits).
			if (port & 1)
			{
				wordMask <<= 8;
			}

			// This bit of code has to do with emulating interrupt detection
			// of rising and falling edges at the same time (which is supported
			// on some other architectures, but not on the MSP430).
			// If we're trying to catch rising and falling edges...
			if (ISBITSET(GPIOIntDetectEdgeBothFlags[port], pin))
			{
				// If the pin is high...
				if (GPIOReadPin(port, pin) != 0)
				{
					// ...set falling edge for interrupt.
					SETMASK(HWREG16(baseAddress + GPIO_REG_IES), wordMask);
				}

				// If the pin is low...
				else
				{
					// ...set rising edge for interrupt.
					CLEARMASK(HWREG16(baseAddress + GPIO_REG_IES), wordMask);
				}
			}

			// Enable the interrupt.
			SETMASK(HWREG16(baseAddress + GPIO_REG_IE), wordMask);

			result = GPIO_RESULT_OK;
		}
	}
	
	return result;
}

/*
 * @brief Disable the interrupt for the specified pin. 
 * @param[in] port The index/identifier of the GPIO port to enable the interrupt for.
 * @param[in] pin The index/identifier of the GPIO pin to disable the interrupt for.
 * @return GPIO_RESULT_OK on success, GPIO_RESULT_INVALID_SELECTION on failure.
 */
GPIOResult_t GPIODisableInterrupt(GPIOPort_t port, GPIOPortSize_t pin)
{
	GPIOResult_t result = GPIO_RESULT_INVALID_SELECTION;	// return value from function
	uint16_t baseAddress;								// address of port's registers
	uint16_t wordMask;									// mask of port's pins, converted to work with 16-bit registers

	// Check for valid pin.
	if (pin < MAX_GPIO_PIN)
	{
		wordMask = BV(pin);

		// Fetch the base address of the port's registers.
		if (port < mathUtils_ArraySize(gpioPortToBaseAddress, uint16_t))
			baseAddress = gpioPortToBaseAddress[port];
		else
			baseAddress = 0xFFFF;

		// Check for invalid ports.
		if(baseAddress != 0xFFFF)
		{
			// Shift by 8 if port is  (upper 8-bits).
			if((port & 1) ^ 1)
			{
				wordMask <<= 8;
			}

			// Disable the interrupt.
			CLEARMASK(HWREG16(baseAddress + GPIO_REG_IE), wordMask);

			result = GPIO_RESULT_OK;
		}
	}

	return result;
}

/******************************************************************************
 * Interrupt Service Routines
 *****************************************************************************/

// Port 1 Interrupt Service Routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = PORT1_VECTOR
__interrupt void Port_1 (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT1_VECTOR))) Port_1 (void)
#else
#error Compiler not supported!
#endif
{
	uint8_t bit = 8;

	// Figure out which pin in the port caused the interrupt.
	// You might think that we'd check the interrupt flag register, but no!
	// The user manual's example has us check the interrupt vector register.
	// Also, clearing the interrupt flag is not necessary, because according to
	// the user manual "any access (read or write) of the lower byte of the
	// P1IV register (either word or byte access) automatically resets the
	// highest pending interrupt flag.
	switch (__even_in_range(P1IV, 16))
	{
	case 0:		// Vector 0 - no interrupt
		break;
	case 2:		// Vector 2 - bit 0
		bit = 0;
		break;
	case 4:		// Vector 4 - bit 1
		bit = 1;
		break;
	case 6:		// Vector 6 - bit 2
		bit = 2;
		break;
	case 8:		// Vector 8 - bit 3
		bit = 3;
		break;
	case 10:	// Vector 10 - bit 4
		bit = 4;
		break;
	case 12:	// Vector 12 - bit 5
		bit = 5;
		break;
	case 14:	// Vector 14 - bit 6
		bit = 6;
		break;
	case 16:	// Vector 16 - bit 7
		bit = 7;
		break;
	default:
		break;
	}

	// Remember that GPIO_PORTA corresponds to MSP430's Port 1.
	// It does NOT correspond to MSP430's Port A (which is Port 1 + Port 2).

	// This bit of code has to do with emulating interrupt detection of rising
	// falling edges at the same time (which is supported on some other
	// architectures, but not on the MSP430).  See comments above.
	// If we're trying to catch rising and falling edges...
	if (ISBITSET(GPIOIntDetectEdgeBothFlags[GPIO_PORTA], bit))
	{
		// If the pin is high...
		if (GPIOReadPin(GPIO_PORTA, bit) != 0)
		{
			// ...set falling edge for interrupt.
			SETMASK(P1IES, bit);
		}

		// If the pin is low...
		else
		{
			// ...set rising edge for interrupt.
			CLEARMASK(P1IES, bit);
		}
	}

	// If a callback has been assigned for transmit complete, call it.
	if (bit < 8)
	{
		if (GPIOCallbackFuncs[GPIO_PORTA][bit] != NULL)
		{
			GPIOCallbackFuncs[GPIO_PORTA][bit]();
		}
	}
}

// Port 2 Interrupt Service Routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = PORT2_VECTOR
__interrupt void Port_2 (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT2_VECTOR))) Port_2 (void)
#else
#error Compiler not supported!
#endif
{
	uint8_t bit = 8;

	// Figure out which pin in the port caused the interrupt.
	// You might think that we'd check the interrupt flag register, but no!
	// The user manual's example has us check the interrupt vector register.
	// Also, clearing the interrupt flag is not necessary, because according to
	// the user manual "any access (read or write) of the lower byte of the
	// P1IV register (either word or byte access) automatically resets the
	// highest pending interrupt flag.
	switch (__even_in_range(P2IV, 16))
	{
	case 0:		// Vector 0 - no interrupt
		break;
	case 2:		// Vector 2 - bit 0
		bit = 0;
		break;
	case 4:		// Vector 4 - bit 1
		bit = 1;
		break;
	case 6:		// Vector 6 - bit 2
		bit = 2;
		break;
	case 8:		// Vector 8 - bit 3
		bit = 3;
		break;
	case 10:	// Vector 10 - bit 4
		bit = 4;
		break;
	case 12:	// Vector 12 - bit 5
		bit = 5;
		break;
	case 14:	// Vector 14 - bit 6
		bit = 6;
		break;
	case 16:	// Vector 16 - bit 7
		bit = 7;
		break;
	default:
		break;
	}

	// Remember that GPIO_PORTB corresponds to MSP430's Port 2.
	// It does NOT correspond to MSP430's Port B (which is Port 3 + Port 4).

	// This bit of code has to do with emulating interrupt detection of rising
	// falling edges at the same time (which is supported on some other
	// architectures, but not on the MSP430).  See comments above.
	// If we're trying to catch rising and falling edges...
	if (ISBITSET(GPIOIntDetectEdgeBothFlags[GPIO_PORTB], bit))
	{
		// If the pin is high...
		if (GPIOReadPin(GPIO_PORTB, bit) != 0)
		{
			// ...set falling edge for interrupt.
			SETMASK(P2IES, bit);
		}

		// If the pin is low...
		else
		{
			// ...set rising edge for interrupt.
			CLEARMASK(P2IES, bit);
		}
	}

	// If a callback has been assigned for transmit complete, call it.
	if (bit < 8)
	{
		if (GPIOCallbackFuncs[GPIO_PORTB][bit] != NULL)
		{
			GPIOCallbackFuncs[GPIO_PORTB][bit]();
		}
	}
}

// Port 3 Interrupt Service Routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = PORT3_VECTOR
__interrupt void Port_3 (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT3_VECTOR))) Port_3 (void)
#else
#error Compiler not supported!
#endif
{
	uint8_t bit = 8;

	// Figure out which pin in the port caused the interrupt.
	// You might think that we'd check the interrupt flag register, but no!
	// The user manual's example has us check the interrupt vector register.
	// Also, clearing the interrupt flag is not necessary, because according to
	// the user manual "any access (read or write) of the lower byte of the
	// P1IV register (either word or byte access) automatically resets the
	// highest pending interrupt flag.
	switch (__even_in_range(P3IV, 16))
	{
	case 0:		// Vector 0 - no interrupt
		break;
	case 2:		// Vector 2 - bit 0
		bit = 0;
		break;
	case 4:		// Vector 4 - bit 1
		bit = 1;
		break;
	case 6:		// Vector 6 - bit 2
		bit = 2;
		break;
	case 8:		// Vector 8 - bit 3
		bit = 3;
		break;
	case 10:	// Vector 10 - bit 4
		bit = 4;
		break;
	case 12:	// Vector 12 - bit 5
		bit = 5;
		break;
	case 14:	// Vector 14 - bit 6
		bit = 6;
		break;
	case 16:	// Vector 16 - bit 7
		bit = 7;
		break;
	default:
		break;
	}

	// Remember that GPIO_PORTC corresponds to MSP430's Port 3.
	// It does NOT correspond to MSP430's Port C (which is Port 5 + Port 6).

	// This bit of code has to do with emulating interrupt detection of rising
	// falling edges at the same time (which is supported on some other
	// architectures, but not on the MSP430).  See comments above.
	// If we're trying to catch rising and falling edges...
	if (ISBITSET(GPIOIntDetectEdgeBothFlags[GPIO_PORTC], bit))
	{
		// If the pin is high...
		if (GPIOReadPin(GPIO_PORTC, bit) != 0)
		{
			// ...set falling edge for interrupt.
			SETMASK(P3IES, bit);
		}

		// If the pin is low...
		else
		{
			// ...set rising edge for interrupt.
			CLEARMASK(P3IES, bit);
		}
	}

	// If a callback has been assigned for transmit complete, call it.
	if (bit < 8)
	{
		if (GPIOCallbackFuncs[GPIO_PORTC][bit] != NULL)
		{
			GPIOCallbackFuncs[GPIO_PORTC][bit]();
		}
	}
}


// Port 4 Interrupt Service Routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = PORT4_VECTOR
__interrupt void Port_4 (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT4_VECTOR))) Port_4 (void)
#else
#error Compiler not supported!
#endif
{
	uint8_t bit = 8;

	// Figure out which pin in the port caused the interrupt.
	// You might think that we'd check the interrupt flag register, but no!
	// The user manual's example has us check the interrupt vector register.
	// Also, clearing the interrupt flag is not necessary, because according to
	// the user manual "any access (read or write) of the lower byte of the
	// P1IV register (either word or byte access) automatically resets the
	// highest pending interrupt flag.
	switch (__even_in_range(P4IV, 16))
	{
	case 0:		// Vector 0 - no interrupt
		break;
	case 2:		// Vector 2 - bit 0
		bit = 0;
		break;
	case 4:		// Vector 4 - bit 1
		bit = 1;
		break;
	case 6:		// Vector 6 - bit 2
		bit = 2;
		break;
	case 8:		// Vector 8 - bit 3
		bit = 3;
		break;
	case 10:	// Vector 10 - bit 4
		bit = 4;
		break;
	case 12:	// Vector 12 - bit 5
		bit = 5;
		break;
	case 14:	// Vector 14 - bit 6
		bit = 6;
		break;
	case 16:	// Vector 16 - bit 7
		bit = 7;
		break;
	default:
		break;
	}

	// Remember that GPIO_PORTD corresponds to MSP430's Port 4.
	// It does NOT correspond to MSP430's Port D (which is Port 7 + Port 8).

	// This bit of code has to do with emulating interrupt detection of rising
	// falling edges at the same time (which is supported on some other
	// architectures, but not on the MSP430).  See comments above.
	// If we're trying to catch rising and falling edges...
	if (ISBITSET(GPIOIntDetectEdgeBothFlags[GPIO_PORTD], bit))
	{
		// If the pin is high...
		if (GPIOReadPin(GPIO_PORTD, bit) != 0)
		{
			// ...set falling edge for interrupt.
			SETMASK(P4IES, bit);
		}

		// If the pin is low...
		else
		{
			// ...set rising edge for interrupt.
			CLEARMASK(P4IES, bit);
		}
	}

	// If a callback has been assigned for transmit complete, call it.
	if (bit < 8)
	{
		if (GPIOCallbackFuncs[GPIO_PORTD][bit] != NULL)
		{
			GPIOCallbackFuncs[GPIO_PORTD][bit]();
		}
	}
}

#ifdef INCLUDE_TEST
/************************************************************************/
/* Test Functions                                                       */
/************************************************************************/

#define TEST_PORT			GPIO_PORTA

#define TEST_LED_PIN		0
#define TEST_LED_OFF		FALSE
#define TEST_LED_ON			TRUE

#define TEST_BTN_FUNC		0
#define TEST_BTN_PIN		1
#define TEST_BTN_PULL		GPIO_PULL_UP
#define TEST_BTN_TRIGGER	GPIO_INT_EDGE_BOTH

GPIOResult_t GPIOTest(void)
{
	GPIOResult_t result = GPIO_RESULT_INVALID_SELECTION;
	GPIOConfig_t	 ledConfig;
	sGPIOIntConfig_t btnConfig;
	
	do
	{
		// Initialize Port A
		if (GPIOInit(TEST_PORT))
			break;
			
		// Setup Configuration structure for LED
		ledConfig.ui32_Direction = GPIO_DIR_OUT;
		ledConfig.ui32_InputPull = GPIO_PULL_NONE;
		ledConfig.ui32_MuxPosition = FALSE;
		ledConfig.ui32_Powersave = FALSE;
		
		// Configure the LED Pin
		if (GPIOConfigPin(TEST_PORT, TEST_LED_PIN, &ledConfig))
			break;
			
		// Make sure LED is off
		if (GPIOWritePin(TEST_PORT, TEST_LED_PIN, TEST_LED_OFF))
			break;
			
		// Setup Configuration structure for Button Interrupt
		btnConfig.ui32_InputPull = TEST_BTN_PULL;
		btnConfig.ui32_MuxPosition = TEST_BTN_MUX;
		btnConfig.ui32_DetectionCriteria = TEST_BTN_TRIGGER;
		btnConfig.ui32_FilterInputSignal = GPIO_RESULT_INVALID_SELECTION;
		btnConfig.ui32_WakeIfSleeping = GPIO_RESULT_INVALID_SELECTION;
		
		// Configure the Button Pin and Interrupt
		if (GPIOConfigInterrupt(TEST_PORT, TEST_BTN_PIN, &btnConfig, GPIOTestCallback))
			break;
			
		// Enable the Button Interrupt
		if (GPIOEnableInterrupt(TEST_PORT, TEST_BTN_PIN))
			break;
		
		result = GPIO_RESULT_OK;
	} while (FALSE);
	
	return result;
}

void GPIOTestCallback(void)
{
	uint8_t ui8_pinState;

	// Read the state of the Button Pin
	ui8_pinState = GPIOReadPin(TEST_PORT, TEST_BTN_PIN);

	// Set the LED state equal to the Button state
	GPIOWritePin(TEST_PORT, TEST_LED_PIN, ui8_pinState);
}

#endif // INCLUDE_TEST

