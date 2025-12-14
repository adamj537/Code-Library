/******************************************************************************
 *
 *	Filename:		FileSystem.h
 *
 *	Author:			Adam Johnson
 *
 *	Description:	Implements a fancy file system on your microcontroller.
 *
 *****************************************************************************/

#ifndef FILE_SYSTEM__H
#define FILE_SYSTEM__H

typedef enum							// result of a requested file action
{
	FILE_RESULT_OK = 0,					// All is well!
	FILE_RESULT_FAIL,					// It's the target's fault.
	FILE_RESULT_NOT_IMPLEMENTED,		// It's my fault.
	FILE_RESULT_INVALID_SELECTION,		// It's your fault.
} fileErr_t;

typedef enum							// different ways to open a file
{
	FILE_MODE_READ,
	FILE_MODE_WRITE,
	FILE_MODE_APPEND,
	FILE_MODE_CREATE,
} fileMode_t;

typedef struct							// holds info about a file
{
	uint8_t status;						// does the file exist?
	uint32_t size;						// size of file [bytes]
	uint32_t maxSize;					// size allocated to file [bytes]
} fileInfo_t;

typedef file_t	uint8_t;				// defines size of a file handle
										// This is used to access any file once
										// it's opened.

fileErr_t FileOpen(char *fileNamePtr, fileMode_t mode, uint32_t maxSize, file_t *fileHandle);
fileErr_t FileClose(file_t fileHandle);
fileErr_t FileRead(file_t fileHandle, uint32_t index, uint8_t *data, uint32_t numData);
fileErr_t FileWrite(file_t fileHandle, uint32_t index, uint8_t *data, uint32_t numData);
fileErr_t FileSearch(char *fileNamePtr, fileInfo_t *info);
fileErr_t FileDelete(file_t fileHandle);

#ifdef INCLUDE_TEST
uint8_t FileTest(void);					// Test the file system library.
#endif

#endif
