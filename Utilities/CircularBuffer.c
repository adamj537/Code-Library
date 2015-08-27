/******************************************************************************
 *
 *	Filename:		CircularBuffer.c
 *
 *	Author:			Adam Johnson
 *
 *	Description:	This module implements circular buffers.  You can declare
 *					a circular buffer variable, then add and remove elements
 *					from it, and get its length.  Useful for serial
 *					communication, sensor data, etc.
 *
 *					Visually, the buffer looks like an array with a "head" and
 *					"tail".  You add elements to the head, and remove them from
 *					the tail.
 *
 *****************************************************************************/

#include <stdint.h>						// universal data types
#include "CircularBuffer.h"				// header for this module

/******************************************************************************
 *
 *	Function:		CBInit
 *
 *	Description:	Erase a buffer's contents, or set up a new buffer.
 *
 *****************************************************************************/

void CBInit(circBuff_t *buffer)
{
	*buffer->length = 0;	// Set number of data to zero.
	*buffer->readPos = 0;	// Set read position to first index in array.
	*buffer->writePos = 0;	// Set write position to first index in array.
}
 
/******************************************************************************
 *
 *	Function:		CBAdd
 *
 *	Description:	Pushes a new datum to a buffer.
 *
 *	Return value:	0 for success; else buffer is full and cannot accept data
 *
 *****************************************************************************/
 
uint8_t CBAdd (circBuff_t *buffer, uint8_t newData)
{
	uint8_t error = 0;					// an optimistic return value :)
	
	// If the buffer is full...
	if (*buffer->length >= BUFFER_SIZE)
	{
		// Alert the user.
		error = 1;
	}
	
	// Add the new element.
	*buffer->data[*buffer->writePos] = newData;

	// Mark the buffer's new tail...
	*buffer->writePos++;
	
	// ...wrap around the array if needed.
	if (*buffer->writePos >= BUFFER_SIZE)
	{
		*buffer->writePos = 0;
	}

	// Increment the buffer's length.
	*buffer->length++;
	
	return error;
}

/******************************************************************************
 *
 *	Function:		CBFetch
 *
 *	Description:	View a datum in a buffer, but don't remove it.
 *
 *****************************************************************************/

uint8_t CBFetch (circBuff_t buffer, uint8_t position)
{
	// Not yet implemented :(
}

/******************************************************************************
 *
 *	Function:		CBRemove
 *
 *	Description:	Pop a datum from a buffer.
 *
 *****************************************************************************/
 
uint8_t CBRemove (circBuff_t *buffer)
{
	uint8_t temp;						// stores a value from the buffer

	// Read from the head of the buffer.
	temp = *buffer->data[*buffer.readPos];

	// Mark the buffer's new head...
	*buffer->readPos++;
	
	// ...wrap around the array if needed.
	if (*buffer.readPos >= BUFFER_SIZE)
	{
		*buffer->readPos = 0;
	}
	
	// Decrement the length.
	*buffer->length--;

	// Return the value from the buffer.
	return temp;
}

/******************************************************************************
 *
 *	Function:		CBGetLength
 *
 *	Description:	Count data in a buffer.
 *
 *****************************************************************************/

uint8_t CBGetLength (circBuff_t *buffer)
{
	return *buffer->length;
}
