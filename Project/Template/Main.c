/******************************************************************************
 *
 *	Notes:	Project.h is included first because sometimes compiler include files
 *			can use special defined symbols (like _XTAL_FREQ for PIC), and it's
 *			nice to tuck them in the Project.h file.   project.h also should
 *			contain any global defined symbols (like INCLUDE_TEST).  stdint.h
 *			allows us to use universal data types (like uint8_t) that are
 *			processor independent and make code look nicer and port easier.  If
 *			for some reason we can't (or don't want to) use stdint.h, we can
 *			make our own file (call it Datatypes.h) and include that instead.
 *			Other included files go after that so they also get the benefit of
 *			the global symbols and nice data types.
 *
 *			Tests, when included, are run before anything else.  All included
 *			modules should have a test.  This makes testing code easier, and
 *			makes changing code less risky.
 *
 *			If the processor handles all interrupts via one "handler," then
 *			that handler should be in this file.  Otherwise, each handler
 *			should be tucked away in the file of the associated module.
 *
 *	Terms of Use:	MIT License
 *
 *****************************************************************************/

#include "project.h"					// global settings
#include "peripherals.h"				// processor-specific header
#include <stdint.h>						// universal data types
										// TODO:  Include relevant driver(s).
										// TODO:  Include relevant app(s).

void Setup(void)
{
										// TODO:  Initialize driver(s).
										// TODO:  Initialize app(s).
}

int main(void)
{
	Setup();
	
#ifdef INCLUDE_TEST						// TODO:  Add/delete tests as needed.
	TestGPIO();							// Test GPIO driver.
	TestTimer();						// Test timer driver.
	TestI2C();							// Test I2C driver.
	TestADC();							// Test ADC driver.
	TestEEPROM();						// Test EEPROM driver.
	TestFlashMem();						// Test Flash memory driver.
	TestDisplay();						// Test display driver.
#endif

	while (1)
	{
										// TODO:  Run application(s).
	}
}