/******************************************************************************
 *
 *	Filename:		SoftwareTimer.h
 *
 *	Author:			Adam Johnson
 *
 *	Description:	This module contains code for timing events and delaying.
 *
 *****************************************************************************/

#ifndef SOFTTIMER_H
#define	SOFTTIMER_H

void     SoftTimerReset(void);									// Reset the timer.
uint8_t  SoftTimerDings(uint32_t pastTime, uint32_t waitTime);	// Check a time interval.
uint32_t SoftTimerGetMS(void);									// Find elapsed time.
void     SoftTimerDelay(uint16_t msec);							// delay routine (in milliseconds)
void     SoftTimerISR(void);									// interrupt service routine
#ifdef INCLUDE_TEST
void     SoftTimerTest(void);									// Test the timer module.
void     SoftTimerTestISR(void);								// interrupt service routine
#endif

#endif	/* SOFTTIMER_H */