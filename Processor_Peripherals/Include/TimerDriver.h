/******************************************************************************
 *
 *	Filename:		hal_timer.h
 *
 *	Author:			Adam Johnson
 *
 *	Description:	This module contains code for timing events and delaying.
 *					The user can register a callback function that runs after
 *					a certain time period has elapsed.  There is also a delay
 *					function, as well as functions to measure elapsed time.
 *
 *	Terms of Use:	MIT License
 *
 *****************************************************************************/

#ifndef TIMERDRIVER_H
#define TIMERDRIVER_H

typedef void (*timerCallback_t)(void);	// prototype for callback functions

// This function resets the timer
void TimerInit(void);

// Fetch the number of milliseconds since the timer was started.
uint32_t TimerGetMS(void);

// Check if a desired time period has elapsed.
bool TimerDings(uint32_t pastTime, uint32_t waitTime);

// Waits a specified time before returning
void TimerDelay(uint16_t msec);

// Register an action to occur on an interval.
void TimerRegister(uint8_t index, uint32_t interval, timerCallback_t action);

// Stop an action from recurring.
void TimerClear(uint8_t index);

// Run any registered actions.
bool TimerProcess(void);

#endif /* TIMERDRIVER_H */
