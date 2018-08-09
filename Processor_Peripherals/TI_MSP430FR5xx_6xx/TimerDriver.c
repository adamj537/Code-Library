/******************************************************************************
 *
 *	Filename:		hal_timer.c
 *
 *	Description:	This module contains code for timing and scheduling events.
 *					The user can register a callback function that runs after
 *					a certain time period has elapsed.  There is also a delay
 *					function, as well as functions to measure elapsed time.
 *
 *	Origin:			github.com/adamj537/Code-Library
 *
 *	Terms of Use:	MIT License
 *
 *****************************************************************************/

#include "project.h"					// global settings
#include <stddef.h>						// includes definition of "NULL"
#include <assert.h>						// assert is used in this module
#include "hal_timer.h"					// header for this module

#ifndef TIMER_MAX_CALLBACKS
#warning "TIMER_MAX_CALLBACKS should be defined in your project."
#warning "defaulting to TIMER_MAX_CALLBACKS = 1"
#define TIMER_MAX_CALLBACKS		1		// number of timer callbacks allowed
#endif

// time passed since timer was reset [ms]
static volatile uint32_t g_ticks;

// frequency with which the action should be performed
static volatile uint32_t g_actionFrequencyArray[TIMER_MAX_CALLBACKS];

// time each action was last performed
static volatile uint32_t g_actionLastRanArray[TIMER_MAX_CALLBACKS];

// flag indicating if it's time to run an action
static volatile bool g_actionFlagArray[TIMER_MAX_CALLBACKS];

// function pointers called when action takes place
static timerCallback_t g_actionCallbackArray[TIMER_MAX_CALLBACKS];

/******************************************************************************
 *	@brief		This function resets the timer
 *	@remarks	Call this function before anything else or the timer won't run.
 *****************************************************************************/
void tim_init(void)
{
	uint8_t i;							// counter

	// Reset the number of ticks.
	g_ticks = 0;

	// Clear all callbacks.
	for (i = 0; i < TIMER_MAX_CALLBACKS; i++)
	{
		g_actionCallbackArray[i] = NULL;
		g_actionFrequencyArray[i] = 0;
		g_actionLastRanArray[i] = 0;
		g_actionFlagArray[i] = false;
	}

	// Configure Timer A0 for Ticks.
	Timer_A_initUpModeParam Timer_A_Param =
	{
		.clockSource = TIMER_A_CLOCKSOURCE_SMCLK,
		.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_64,
		.timerPeriod = 62500/TICK_PER_SECOND,
		.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE,
		.captureCompareInterruptEnable_CCR0_CCIE = TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
		.timerClear = TIMER_A_DO_CLEAR,
		.startTimer= true
	};

	Timer_A_initUpModeParam *Timer_A_Param_ptr = &Timer_A_Param;

	Timer_A_initUpMode(__MSP430_BASEADDRESS_T0A3__, Timer_A_Param_ptr);
}

/******************************************************************************
 *	@brief	Fetch the number of milliseconds since the timer was started.
 *	@return	number of milliseconds since the tim_init was run
 *****************************************************************************/
uint32_t tim_getMS(void)
{
	// If you change the clock rate, remember to change this function too :)
	return g_ticks;
}

/******************************************************************************
 *	@brief		Check if a desired time period has elapsed.
 *	@remarks	Use this function to time things that must occur after a
 *				desired time period or things that occur periodically.
 *	@param[in]	pastTime - a timestamp (use tim_getMS to get it)
 *	@param[in]	waitTime - the desired elapsed time [ms]
 *	@return		true if timer has "dinged", false otherwise
 *****************************************************************************/
bool tim_dings(uint32_t pastTime, uint32_t waitTime)
{
	uint32_t nowTime;					// current timestamp
	bool doneFlag = false;				// flag set when time is over

	nowTime = tim_getMS();				// Fetch the current time.
	if ((nowTime - pastTime > waitTime))// If the desired time has elapsed...
	{
		doneFlag = true;				// Yay!  We're done.
	}

	return doneFlag;					// Let the user know.
}

/******************************************************************************
 *	@brief		Waits a specified time before returning
 *	@param[in]	msec - desired delay [ms]
 *****************************************************************************/
void tim_delay(uint16_t msec)
{
	uint32_t pastTime;					// timestamp

	pastTime = tim_getMS();				// Get a new timestamp.

	while(1)							// Wait...
	{
		if (tim_dings(pastTime, msec))	// ...for a timeout.
		{
			break;
		}
	}
}

/******************************************************************************
 *	@brief		Register a callback function to occur on an interval.
 *	@remarks	The registered event will keep recurring until it is unregistered.
 *	@param[in]	index - callback id (must be less than TIMER_MAX_CALLBACKS)
 *	@param[in]	interval - how long until the event occurs [ms]
 *	@param[in]	action - function pointer to be called on the interval
 *****************************************************************************/
void tim_register(uint8_t index, uint32_t interval, timerCallback_t action)
{
	// Ensure the index is valid.
	assert(index < TIMER_MAX_CALLBACKS);

	// Remember the action, when it was last done, and how often it runs.
	g_actionCallbackArray[index] = action;
	g_actionLastRanArray[index] = tim_getMS();
	g_actionFrequencyArray[index] = interval;
}

/******************************************************************************
 *	@brief		Stop an action from recurring.
 *	@param[in]	index - callback id (must be less than TIMER_MAX_CALLBACKS)
 *****************************************************************************/
void tim_clear(uint8_t index)
{
	// Ensure the index is valid.
	assert(index < TIMER_MAX_CALLBACKS);

	// Clear the callback and interval.
	g_actionCallbackArray[index] = NULL;
	g_actionFrequencyArray[index] = 0;
}

/******************************************************************************
 *	@brief		Run any registered actions.
 *	@remarks	Run this function in your program's main loop.
 * 				The return value is useful so that you can wake from sleep on
 * 				an interrupt, and this function will tell you when the action
 * 				has been processed so you can put the processor back to sleep.
 *	@return		true if an action was performed; false if no action necessary
 *****************************************************************************/
bool tim_process(void)
{
	bool doneFlag = false;				// true after an action is performed
	uint8_t i;							// counter

	// For each possible callback...
	for (i = 0; i < TIMER_MAX_CALLBACKS; i++)
	{
		// If the callback exists and it's time to call it...
		if ((g_actionCallbackArray[i] != NULL) && (g_actionFlagArray[i] != false))
		{
			// Update when the callback last ran.
			// Do this before calling the callback to take into account the time
			// spent running the callback.
			g_actionLastRanArray[i] = g_ticks;

			// Call it.
			g_actionCallbackArray[i]();

			// Reset the flag.
			g_actionFlagArray[i] = false;

			// Inform the user that an action was performed.
			doneFlag = true;
		}
	}

	// Let the main program know if an action was performed.
	return doneFlag;
}

/* Timer Tick Interrupt Handler 	*/
/* TA0 CCR0 Vector 					*/
#pragma vector=TIMER0_A0_VECTOR;
__interrupt void timTick (void)
{
	uint8_t i;							// counter

	// Increment the number of ms the timer has been running.
	g_ticks++;

	// For each possible callback...
	for (i = 0; i < TIMER_MAX_CALLBACKS; i++)
	{
		// If the callback exists and it's time to call it...
		if ((g_actionCallbackArray[i] != NULL) &&
			((g_ticks - g_actionLastRanArray[i]) >= g_actionFrequencyArray[i]))
		{
			// Set a flag so the specified action will be performed.
			g_actionFlagArray[i] = true;

#if ENABLE_SLEEP
			// When the interrupt returns, exit sleep mode.
			__bic_SR_register_on_exit(CPUOFF);
#endif
		}
	}
}


