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
 *	Terms of Use:	MIT License
 *
 *****************************************************************************/

#include <stdint.h>								// universal data types
#include "Calculate.h"							// math library (for GCD)
#include "Timer.h"								// hardware timer driver
#include "Scheduler.h"							// header for this module

callbackPtr_t g_callbackArray[MAX_TASKS];		// array of tasks
uint32_t g_interval[MAX_TASKS];					// array of intervals [ms]
uint8_t g_numTasks;								// number of tasks scheduled
uint16_t g_gcd;									// greatest common divisor
uint16_t g_maxInterval;							// greatest interval
volatile uint16_t ticks;						// elapsed timer ticks

void TasksInit()
{
	uint16_t i;									// counter
	
	g_gcd = 500;								// Set a reasonable GCD.
	g_numTasks = 0;								// Start with zero tasks.
	g_maxInterval = 0;							// Reset the max interval.
	ticks = 0;									// Reset the tick count.
	
	TimerInit();								// Set up timer hardware.
}

uint8_t TasksAdd(callbackPtr_t Callback, uint16_t msInterval)
{
	g_callbackArray[g_numTasks] = Callback;		// Remember the task.
	g_interval[g_numTasks] = msInterval;		// Remember the interval.
	
	g_numTasks++;								// Update the task count.
	g_gcd = CalcGCD(g_gcd, msInterval);			// Update the GCD.
	if (msInterval > g_maxInterval)				// Update the max interval.
	{
		g_maxInterval = msInterval;
	}
}

uint8_t TasksDelete(callbackPtr_t Callback)
{
	uint8_t i;									// counter
	
	for (i = 0; i < g_numTasks; i++)			// Find the task to remove.
	{
		if (g_callbackArray[i] == Callback)
		{
			break;
		}
	}
	
	g_numTasks--;								// Update the task count.
	
	do											// Remove task from arrays.
	{
		g_interval[i] = g_interval[i + 1];
		g_callbackArray[i] = callbackArray[i + 1];
		i++;
	} while (i < g_numTasks);
	
	for (i = 0; i < g_numTasks; i++)
	{
		g_gcd = CalcGCD(g_gcd, g_interval[i]);	// Calculate new GCD.
		if (g_interval[i] > g_maxInterval)		// Calculate new max interval.
		{
			g_maxInterval = g_interval[i];
		}
	}
}

void TasksRun(void)
{
	uint8 i;	// counter
	
	for (i = 0; i < g_numTasks; i++)			// Check each task.
	{
		if (ticks == g_interval[i])
		{
			callbackPtr[i]();					// Run the callback function.
		}
	}
	
	if (ticks >= g_maxInterval)					// Only count to max interval.
	{
		ticks = 0;
	}
}

void TasksISR(void)
{
	TimerReset(g_gcd);							// Reload timer to run at GCD.
	
	ticks++;									// Tick the clock.
}