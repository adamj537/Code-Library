/******************************************************************************
 *
 *	Title:			CC3200 Example Project
 *
 *	Filename:		Main.c
 *
 *	Author:			Adam Johnson
 *
 *	Description:	An example project for the CC3200.  Configures the
 *					microcontroller and runs tests on the internal peripherals.
 *					Tested on CC3200 LaunchXL board.
 *
 *****************************************************************************/

#include "rom.h"					// macros to call ROM DriverLib
#include "rom_map.h"				// macros choose ROM or Flash DriverLib
#include "hw_memmap.h"				// defines base address of peripherals
#include "hw_types.h"				// data types required by DriverLib
#include <stdbool.h>				// defines "bool", "true", "false"
#include <stdint.h>					// defines standard data types
#include <stddef.h>					// defines "NULL"
#include "hw_ints.h"
#include "interrupt.h"
#include "prcm.h"					// DriverLib - power reset clock manager
#include "uart.h"
#include "pin.h"
#include "adc.h"

#include "project.h"
#include "CC3200_I2C.h"
#include "CC3200_ADC.h"

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

void Setup(void)
{
	// In case of TI-RTOS vector table is initialized by OS itself.
#ifndef USE_TIRTOS
	// Set vector table base.
#if defined(ccs)
	MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
	MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif

	// Enable processor.
	MAP_IntMasterEnable();
	MAP_IntEnable(FAULT_SYSTICK);

	// Set mandatory configurations for the MCU.
	PRCMCC3200MCUInit();

	// Configure PIN_55 for UART0 UART0_TX.
	MAP_PinTypeUART(PIN_55, PIN_MODE_3);

	// Configure PIN_57 for UART0 UART0_RX.
	MAP_PinTypeUART(PIN_57, PIN_MODE_3);

	// Configure PIN_01 for I2C0 I2C_SCL.
	MAP_PinTypeI2C(PIN_01, PIN_MODE_1);

	// Configure PIN_02 for I2C0 I2C_SDA.
	MAP_PinTypeI2C(PIN_02, PIN_MODE_1);

	// Configure PIN_58 for ADC (could also use PIN_59 or PIN_60).
	// PIN_57 is also an ADC, but is being used (above) for UART0_RX.
	MAP_PinTypeADC(PIN_58, PIN_MODE_255);
}

int main(void)
{
	Setup();							// Initialize the processor.

#ifdef INCLUDE_TEST
	I2CTest(I2C0, false);				// Test I2C driver.
	AdcTest(ADC1);						// Test ADC driver.
#endif

	while (1);							// Stay here forever.
}
