/******************************************************************************
 *
 *	Filename:		BitLogic.h
 *
 *	Author:			Adam Johnson
 *
 *	Description:	Useful macros for doing basic boolean math.
 *
 *	Origin:			github.com/adamj537/Code-Library
 *
 *	Terms of Use:	MIT License
 *
 *****************************************************************************/

#ifndef BITLOGIC_H
#define BITLOGIC_H

// Definition for "TRUE" (because I hate that it's usually defined lowercase)
#ifndef TRUE
#define TRUE	1
#endif

// Definition for "FALSE" (because I hate that it's usually defined lowercase)
#ifndef FALSE
#define FALSE	0
#endif

// Macros to do boolean calculations with a bit number
#define SETBIT(BYTE, BIT)		((BYTE) |= (1 << (BIT)))
#define CLEARBIT(BYTE, BIT)		((BYTE) &= ~(1 << (BIT)))
#define TOGGLEBIT(BYTE, BIT)	((BYTE) ^= (1 << (BIT)))
#define ISBITSET(BYTE, BIT)		((0 != ((BYTE) & (1 << (BIT)))) ? TRUE : FALSE)
#define ISBITCLEAR(BYTE, BIT)	(!ISBITSET((BYTE), (BIT)))

// Macro to convert a bit number into a byte value
#define BV(bit)					(1 << (bit))

// Macros to do boolean calculations with a byte value
#define SETMASK(BYTE, MASK)		((BYTE) |= (MASK))
#define CLEARMASK(BYTE, MASK)	((BYTE) &= ~(MASK))
#define TOGGLEMASK(BYTE, MASK)	((BYTE) ^= (MASK))
#define ISMASKSET(BYTE, MASK)	((0 != ((BYTE) & (MASK))) ? TRUE : FALSE)
#define ISMASKCLEAR(BYTE, MASK)	(!ISMASKSET((BYTE), (MASK)))

// Macro to modify a register value
#define MODIFYREG(REG, CLRMSK, SETMSK) \
	((REG) = (((REG) & (~(CLRMSK))) | (SETMSK)))

// Macros to figure out if an expression is even or odd
#define ISODD(x)				((x) % 2)
#define ISEVEN(x)				(!ISODD(x))

// Macros to swap bytes in a variable.
#define SWAP16(num) \
	(((num & 0xFF00) >> 8) | \
	 ((num & 0x00FF) << 8))

#define SWAP32(num)	\
	(((num & 0xFF000000) >> 24) | \
	 ((num & 0x00FF0000) >>  8) | \
	 ((num & 0x0000FF00) <<  8) | \
	 ((num & 0x000000FF) << 24))

#endif