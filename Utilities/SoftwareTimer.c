/******************************************************************************
 *
 *	Filename:		SoftwareTimer.c
 *
 *	Author:			Adam Johnson
 *
 *	Description:	This module contains code for timing events and delaying.
 *
 *****************************************************************************/

// Included modules
#include <stdint.h>						// universal data types
#include "SoftwareTimer.h"				// header for this module

// Defined constants & macros
#define TICKS_PER_MS		1			// number of ISR calls per ms

// Global Variables
static volatile uint32_t g_ticks;		// incremented once per call to ISR

/******************************************************************************
 *
 *	Function:		SoftTimerReset
 *
 *	Description:	This function resets the timer.
 *
 ******************************************************************************/

void SoftTimerReset(void)
{
	g_ticks = 0;						// Reset the number of ticks.
}

/******************************************************************************
 *
 *	Function:		SoftTimerGetMS
 *
 *	Description:	This function returns the number of milliseconds since the
 *					timer was started.
 *
 *	Return Value:	number of milliseconds since the timer was started
 *
 ******************************************************************************/

uint32_t SoftTimerGetMS(void)
{
	return (g_ticks / TICKS_PER_MS);	// Report how many ms have passed.
}

/******************************************************************************
 *
 *	Function:		SoftTimerDings
 *
 *	Description:	This function is used to check if a desired time period
 *					has elapsed.  Use it to time things that must occur after
 *					a desired time period or things that occur periodically.
 *
 *	Parameters:		pastTime - a timestamp (use SoftTimerGetMS to get it)
 *					waitTime - the desired elapsed time [ms]
 *
 *	Return Value:	nonzero indicates the timer has "dinged".
 *
 ******************************************************************************/

uint8_t SoftTimerDings(uint32_t pastTime, uint32_t waitTime)
{
	uint8_t doneFlag;					// flag set when time is over
	uint32_t nowTime;					// current timestamp

	doneFlag = 0;						// Assume we're not done.

	nowTime = SoftTimerGetMS();		// Get the current time.
	if ((nowTime - pastTime) > waitTime)// Has the desired time elapsed?
	{
		doneFlag = 1;					// Yay!  We're done.
	}

	return doneFlag;					// Let the user know.
}

/******************************************************************************
 *
 *	Function:		SoftTimerDelay
 *
 *	Description:	This function waits a specified time before returning.
 *					Use wherever you need a delay.
 *
 *	Parameters:		msec - desired delay, in milliseconds
 *
 ******************************************************************************/

void SoftTimerDelay(uint16_t msec)
{
	uint32_t pastTime;					// timestamp

	pastTime = SoftTimerGetMS();		// Get a new timestamp.

	while (1)							// Wait...
	{
		if (SoftTimerDings(pastTime, msec))// ...for a timeout.
		{
			break;
		}
	}
}

/******************************************************************************
 *
 *	Function:		SoftTimerISR
 *
 *	Description:	An interrupt service routine.  Call this in a timer
 *					interrupt that runs once every millisecond.
 *					This routine makes the timer module "tick."
 *
 ******************************************************************************/

void SoftTimerISR(void)
{
	g_ticks++;
}

/*****************************************************************************/
/* Test Functions                                                            */
/*****************************************************************************/

#ifdef INCLUDE_TEST_FUNCTIONS

#include "GPIODriver.h"

// Ensure that the test's settings are defined.
#ifndef SWTIMER_TEST_PORT
#error "SWTIMER_TEST_PORT should be defined in project.h,"
#error "like this: #define SWTIMER_TEST_PORT PORTD"
#endif

#ifndef SWTIMER_TEST_PIN
#error "SWTIMER_TEST_PIN should be defined in project.h,"
#error "like this:  #define SWTIMER_TEST_PIN 5"
#endif

/******************************************************************************
 *
 *	Function:		SoftTimerTest
 *
 *	Description:	This function and SoftTimerTestISR work together to test the
 *					Timer.c module.  TestTimer will cause the display to count
 *					a 30 second interval.  This gives a test of the timer's
 *					accuracy over time.  TestTimerISR toggles an I/O pin every
 *					time the hardware timer ticks, to verify that Timer.c is
 *					being clocked at the correct frequency.
 *
 *	TODO: - Use a display library to count the seconds, or devise a better test.
 *
 ******************************************************************************/

void SoftTimerTest(void)
{
	uint8_t numSeconds = 0;				// number of elapsed seconds

	while (numSeconds <= 30)			// Run the test for 30 seconds.
	{
		SoftTimerDelay(1000);			// Wait a second.
		numSeconds++;					// Increment the number of seconds.
//		DisplayMainInt(numSeconds);		// Display the number of seconds.
	}
}

/******************************************************************************
 *
 *	Function:		SoftTimerTestISR
 *
 *	Description:	This function toggles an I/O pin every time the hardware
 *					timer ticks, to verify that Timer.c is being clocked at the
 *					correct frequency.
 *
 ******************************************************************************/

void SoftTimerTestISR(void)
{
	// Toggle a pin on each timer tick.
	GPIOTogglePin(SWTIMER_TEST_PORT, SWTIMER_TEST_PIN);
}

#endif
