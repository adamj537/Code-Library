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
 *	Notes:			XTAL_FREQ is required by PIC compilers.  So just use that
 *					across all types of projects.  If it someday conflicts with
 *					something, then define it in a separate file (see Nop.h as
 *					an example).
 *
 *	Terms of Use:	MIT License
 *
 *****************************************************************************/

#ifndef PROJECT__H
#define PROJECT__H

#define INCLUDE_TEST			// Include test code.
#define ID			"TEST"		// firmware ID
#define FVN			"  V1"		// firmware version
#define XTAL_FREQ	16000000	// oscillator frequency [Hz]

#endif
