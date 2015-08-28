/******************************************************************************
 *
 *	Filename:		Scheduler.c
 *
 *	Author:			Adam Johnson
 *
 *	Description:	A lightweight task scheduler.
 *
 *	Inspired by:	http://www.embedded.com/design/programming-languages-and-tools/4007578/A-six-step-process-for-migrating-embedded-C-into-a-C--object-oriented-framework
 *
 *	Notes:			Another good task scheduler is at http://riosscheduler.org
 *					More info at http://www.ics.uci.edu/~givargis/pubs/C50.pdf
 *
 *****************************************************************************/

#include <stdint.h>								// universal data types
#include "Calculate.h"							// math library (for GCD)
#include "Timer.h"								// hardware timer driver
#include "Scheduler.h"							// header for this module

callbackPtr_t callbackArray[MAX_TASKS];			// array of tasks
uint32_t interval[MAX_TASKS];					// array of intervals [ms]
uint8_t numTasks;								// number of tasks scheduled
uint16_t gcd;									// greatest common divisor
uint16_t maxInterval;							// greatest interval
volatile uint16_t ticks;						// elapsed timer ticks

void TasksInit()
{
	uint16_t i;									// counter
	
	gcd = 500;									// Set a reasonable GCD.
	numTasks = 0;								// Start with zero tasks.
	maxInterval = 0;							// Reset the max interval.
	ticks = 0;									// Reset the tick count.
	
	TimerInit();								// Set up timer hardware.
}

uint8_t TasksAdd(callbackPtr_t Callback, uint16_t msInterval)
{
	callbackArray[numTasks] = Callback;			// Remember the task.
	interval[numTasks] = msInterval;			// Remember the interval.
	
	numTasks++;									// Update the task count.
	gcd = CalcGCD(gcd, msInterval);				// Update the GCD.
	if (msInterval > maxInterval)				// Update the max interval.
	{
		maxInterval = msInterval;
	}
}

uint8 TasksDelete(callbackPtr_t Callback)
{
	uint8 i;									// counter
	
	for (i = 0; i < numTasks; i++)				// Find the task to remove.
	{
		if (callbackArray[i] == Callback)
		{
			break;
		}
	}
	
	numTasks--;									// Update the task count.
	
	do											// Remove task from arrays.
	{
		interval[i] = interval[i + 1];
		callbackArray[i] = callbackArray[i + 1];
		i++;
	} while (i < numTasks);
	
	for (i = 0; i < numTasks; i++)
	{
		gcd = CalcGCD(gcd, interval[i]);		// Calculate new GCD.
		if (interval[i] > maxInterval) {		// Calculate new max interval.
			maxInterval = interval[i];
		}
	}
}

void TasksRun(void)
{
	uint8 i;	// counter
	
	for (i = 0; i < numTasks; i++)				// Check each task.
	{
		if (ticks == interval[i])
		{
			callbackPtr[i]();					// Run the callback function.
		}
	}
	
	if (ticks >= maxInterval)					// Only count to max interval.
	{
		ticks = 0;
	}
}

void TasksISR(void)
{
	TimerReset(gcd);							// Reload timer to run at GCD.
	
	ticks++;									// Tick the clock.
}