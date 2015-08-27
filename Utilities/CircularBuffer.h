/******************************************************************************
 *
 *	Filename:		CircularBuffer.h
 *
 *	Author:			Adam Johnson
 *
 *	Description:	This module implements circular buffers.  You can declare
 *					a circular buffer variable, then add and remove elements
 *					from it, and get its length.
 *
 *					Visually, the buffer looks like an array with a "head" and
 *					"tail".  You add elements to the head, and remove them from
 *					the tail.
 *
 *****************************************************************************/

#ifndef CIRCULARBUFFER_H
#define	CIRCULARBUFFER_H

#define BUFFER_SIZE		128				// maximum number of bytes per buffer
										// Must fit in a uint8_t.
typedef struct
{
	uint8_t data[BUFFER_SIZE];			// array holding values in the buffer
	uint8_t readPos;					// index in array where oldest datum is
	uint8_t length;						// number of datum in the buffer
	uint8_t writePos;					// index in array where new datum goes
} circBuff_t;

void    CBInit(circBuff_t *buffer);						// Init/erase a buffer.
uint8_t CBAdd (circBuff_t buffer, uint8_t character);	// Push newest datum.
uint8_t CBFetch (circBuff_t buffer, uint8_t position);	// Peek at a datum.
uint8_t CBRemove (circBuff_t buffer);					// Pop oldest datum.
uint8_t CBGetLength (circBuff_t buffer);				// Get number of data.

#endif	/* CIRCULARBUFFER_H */

