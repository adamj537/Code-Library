/******************************************************************************
 *
 *	Filename:		Power.c
 *
 *	Author:			Adam Johnson
 *
 *	Description:	Contains functions to turn power on and off, manage
 *					auto-power off, and control the Low Volt Detector.  The
 *					auto-off setting is found using the settings module.
 *					Maximum auto-off delay is 65535 seconds (18 hours), since
 *					the autoOffTimer variable is 16 bits.
 *
 *****************************************************************************/

// Main include files
#include "Main.h"
#include <xc.h>
#include "Datatypes.h"
#include "Bitlogic.h"

// Dependent Modules
#include "EEPROM.h"
#include "Display.h"
#include "Timer.h"

#define PPORT			LATC			// GPIO port controlling power
#define PPIN			1				// GPIO pin controlling power
#define BAT_IDLE_MS		60000			// interval between battery checks [ms]
#define BAT_CHECK_MS	1000			// time to wait for LVD [ms]

enum batStates {						// state machine for battery checks
	IDLE,								// Initialize the low-volt detecter.
	CHECK,								// Check the battery.
};

// Global Variables
static   uint32_t userTime;				// timestamp of last user activity
static   uint32_t offTime = 0;			// auto-off setting [milliseconds]
static   uint8_t  offEnable = 0;		// auto power off enable flag
static   uint8_t  batState = IDLE;		// state machine to check battery
static   uint32_t batTime = 0;			// timestamp of last battery check
volatile uint8_t  batLow = 0;			// battery status (1 = low voltage)
static   uint8_t  batPrevLow = 0;		// previous battery status

/******************************************************************************
 *
 *	Function:		LVDinit
 *
 *	Description:	This function turns on the Low Voltage Detect (LVD) module.
 *					It triggers an interrupt when the supply voltage reaches a
 *					set point.  Here we are using an external analog input as
 *					the setpoint (it can also use internal references).  At the
 *					time of this writing, the battery monitor circuit was set
 *					to vary between 1 to 2 V as the battery voltage varies
 *					between 5 to 9 V.  Trip voltage is 1.2 V.
 *
 ******************************************************************************/

static void LVDinit(void)
{
	// Low Voltage Detect Module
	LVDCON = 0x0F;					// Set to use external analog input.
	PIE2bits.LVDIE = 0;				// Disable low voltage interrupt.
	LVDCONbits.LVDEN = 1;			// Enable low voltage module.
	while (!IRVST);					// Wait for LVD to stabilize.
	PIR2bits.LVDIF = 0;				// Clear low voltage interrupt flag.
	PIE2bits.LVDIE = 1;				// Enable the low voltage interrupt.
}

/******************************************************************************
 *
 *	Function:		PowerIsOn
 *
 *	Description:	This function returns the state of the main voltage
 *					regulator (on or off).
 *
 *	Return Value:	0 for OFF; other values indicate ON.
 *
 ******************************************************************************/

uint8_t PowerIsOn(void)
{
	return ISBITSET(PPORT, PPIN);	// Get status of regulator's enable pin.
}

/******************************************************************************
 *
 *	Function:		PowerResetTimeout
 *
 *	Description:	This function may be called whenever a user action is
 *					detected.  It stores the time when the function was called,
 *					so that elapsed time can be calculated.
 *
 ******************************************************************************/

void  PowerResetTimeout(void)
{
	userTime = TimerGetMS();		// Take a new timestamp.
}

/******************************************************************************
 *
 *	Function:		PowerSetTimeout
 *
 *	Description:	This function may be called whenever the auto-off timeout
 *					changes.
 *
 ******************************************************************************/

void  PowerSetTimeout(void)
{
	offTime = EEPROMreadWord(AUTO_OFF_ADDR);	// Fetch auto power off setting.
	offTime *= 60000;							// Convert minutes to ms.

	if (offTime != FALSE)
	{
		offEnable = TRUE;						// Enable auto off.
		PowerResetTimeout();					// Set the auto-off feature.
	}
	else
	{
		offEnable = FALSE;
	}
}

/******************************************************************************
 *
 *	Function:		PowerOn
 *
 *	Description:	This function turns battery power on for the device.
 *
 ******************************************************************************/

void PowerOn(void)
{
	SETBIT(PPORT, PPIN);			// Keep the power on.

	PowerSetTimeout();				// Set up the auto-off feature.
}

/******************************************************************************
 *
 *	Function:		PowerOff
 *
 *	Description:	This function turns off power.
 *
 ******************************************************************************/

void PowerOff(void)
{
	CLEARBIT(PPORT, PPIN);			// Turn off power.
}

/******************************************************************************
 *
 *	Function:		PowerToggle
 *
 *	Description:	This function checks the status of power, and changes it.
 *
 ******************************************************************************/
/*
void  PowerToggle(void)
{
	if (ISBITSET(PPORT, PPIN))		// Get status of regulator's enable pin.
	{
		PowerOn();					// Turn power on.
	}
	else
	{
		PowerOff();					// Turn power off.
	}
}
*/
/******************************************************************************
 *
 *	Function:		PowerProcess
 *
 *	Description:	This function should be called in the main program loop.
 *					It checks the auto power off timer, and turns power off if
 *					it reaches zero.
 *
 ******************************************************************************/

void  PowerProcess(void)
{
	uint32_t currentTime;					// time right now
	uint32_t elapsedTime;					// time since the last user input

	currentTime = TimerGetMS();				// Fetch the current time.

	elapsedTime = currentTime - userTime;	// Calculate elapsed time.

	if (offEnable)							// Check auto-off.
	{
		if (elapsedTime >= offTime)			// If it's time to turn off...
		{
			PowerOff();						// turn off.
		}
	}

	if (batState == IDLE)					// Check battery periodically.
	{
		if (currentTime - batTime >= BAT_IDLE_MS)
		{
			batLow = FALSE;					// Assume battery ok.
			LVDinit();						// Start the LVD.
			batState = CHECK;
		}
	}										// Wait a bit.
	else if (currentTime - batTime >= BAT_IDLE_MS + BAT_CHECK_MS)
	{
		if (batLow && !batPrevLow)			// If bat is low and was ok...
		{
			DisplaySetSegment(LOWBAT);		// ...battery is low.
			batPrevLow = TRUE;
		}
		else if (!batLow && batPrevLow)		// If bat is ok and was low...
		{
			DisplayClearSegment(LOWBAT);	// ...battery is ok.
			batPrevLow = FALSE;
		}
		batTime = TimerGetMS();				// Reset the timer.
		batState = IDLE;
	}
}

/******************************************************************************
 *
 *	Function:		PowerLowBatISR
 *
 *	Description:	This function should be called by an interrupt service
 *					routine which runs when the battery voltage is low.
 *
 ******************************************************************************/

void PowerLowBatISR(void)
{
	batLow = 1;					// Set low-voltage flag.

								// The function calling this disables the LVD.
}