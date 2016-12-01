/******************************************************************************
 *
 *	Filename:		project.h
 *
 *	Author:			Adam Johnson
 *
 *	Description:	Contains information and settings applicable to the entire
 *					project.  This file should be included before any others
 *					so that the symbols defined here are guaranteed to be
 *					global.
 *
 *****************************************************************************/

#ifndef PROJECT__H
#define PROJECT__H

#define INCLUDE_TEST			// Include test code.
#define ID			"TEST"		// firmware ID - a 4 character string
#define FVN			"  V1"		// firmware version - a 4 character string
#define XTAL_FREQ	16000000	// oscillator frequency [Hz]

#endif
