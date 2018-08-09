/******************************************************************************
 *
 *	Filename:		Nop.h
 *
 *	Description:	The no-operation command is useful, but syntax differs
 *					between compilers.  This file wraps the implementations
 *					used by various embedded compilers, to make code more
 *					portable.  So basically, you can now always type "NOP();".
 *
 *	Terms of Use:	MIT License
 *
 *****************************************************************************/

#ifndef NOP_H
#define NOP_H

// Define no operation for TI & IAR compilers.
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#define NOP()	__no_operation()
#endif

// Define no operation for AVR compilers.
#if defined (__AVR__)
#define NOP()	do { __asm__ __volatile__ ("nop"); } while (0)
#endif

// Define no operation for PIC18 compilers.
#if defined(__18CXX)
#define NOP()	__nop()
#endif

// Define no operation for Keil compilers.
#if defined (__C51__)
#define NOP()	_nop_()
#endif

#endif