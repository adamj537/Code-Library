/******************************************************************************
 *
 *	Filename:		Power.h
 *
 *	Author:			Adam Johnson
 *
 *	Description:	Contains functions to turn power on and off, manage
 *					auto-power off, and initialize/close other modules.  Note
 *					that auto-off setting is found using the settings module.
 *
 *****************************************************************************/

#ifndef POWER_H
#define	POWER_H

uint8_t PowerIsOn(void);			// is the power on or off?
void    PowerOn(void);				// turn power on; initialize other modules
void    PowerOff(void);				// turn power off; clear LCD
//void  PowerToggle(void);			// toggle power on or off
void    PowerProcess(void);			// perform auto-off function if timeout
void    PowerResetTimeout(void);	// reset the auto-off timer
void    PowerSetTimeout(void);		// set the auto-off timeout
void    PowerLowBatISR(void);		// Interrupt Service Routine - battery low
#endif	/* POWER_H */

