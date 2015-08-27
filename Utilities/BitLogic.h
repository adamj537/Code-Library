#ifndef BITLOGIC_H
#define BITLOGIC_H

// Macros to do boolean calculations with a bit number
#define SETBIT(BYTE, BIT)		(BYTE |= (1 << BIT))
#define CLEARBIT(BYTE, BIT)		(BYTE &= ~(1 << BIT))
#define TOGGLEBIT(BYTE, BIT)	(BYTE ^= (1 << BIT))
#define ISBITSET(BYTE, BIT)		(BYTE & (1 << BIT))
#define ISBITCLEAR(BYTE, BIT)	(!ISBITSET(BYTE, BIT))

// Macro to convert a bit number into a byte value
#define BV(bit)					(1 << (bit))

// Macros to do boolean calculations with a byte value
#define SETMASK(BYTE, MASK)		(BYTE |= (MASK))
#define CLEARMASK(BYTE, MASK)	(BYTE &= ~(MASK))
#define TOGGLEMASK(BYTE, MASK)	(BYTE ^= (MASK))
#define ISMASKSET(BYTE, MASK)	(BYTE & (MASK))
#define ISMASKCLEAR(BYTE, MASK)	(!ISMASKSET(BYTE, MASK))

// Macros to figure out if an expression is even or odd
#define ISODD(x)				((x) % 2)
#define ISEVEN(x)				(!ISODD(x))

// Definition for "TRUE" (because I hate that it's usually defined lowercase)
#ifndef TRUE
#define TRUE	1
#endif

// Definition for "FALSE" (because I hate that it's usually defined lowercase)
#ifndef FALSE
#define FALSE	0
#endif

#endif