/******************************************************************************
 * @file	FlashManager.c
 * @author	Adam Johnson
 * @remarks	Contains functions for EEPROM emulation using two pages of
 *			Flash memory.  Based on code from NXP's app note AN11008,
 *			titled "Flash based non-volatile storage."  Originally
 *			developed for MSP430.
 *****************************************************************************/

#include <stdint.h>						// standard-size types
#include <stdbool.h>					// defines "bool" type
#include "Flash.h"						// device specific Flash functions
#include "FlashManager.h"				// header for this module

/******************************************************************************
 *	Configuration Settings
 *****************************************************************************/

// size of a sector in bytes (this must be a multiple of the smallest size of
// flash memory that can be erased)
#define SECTOR_SIZE				128

// minimum number of bytes that can be written at once
#define MIN_WRITE_SIZE			1

// max size of variable in bytes.  There are several restrictions on this.
// It can't be bigger than SECTOR_SIZE - 4 (or the sector will be too small for
// even a single variable).  It must also be sized such that the flashData_t
// (below) has a size that is an exact multiple of MIN_WRITE_SIZE (or else
// Flash can't write a whole flashData_t at once).
#define MAX_VARIABLE_SIZE		22

// start address of 1st sector (best practice:  put at the end of flash)
#define SECTOR1_ADDR			MEM_ADDR_INFO_C

// start address of 2nd sector (best practice:  locate next to 1st sector)
#define SECTOR2_ADDR			MEM_ADDR_INFO_D

// Commenting this line will remove the checksum code from this module.  This
// will reduce the size of the code, and you'll also be able to store memory
// with greater density.  However, you'll be less likely to detect errors.
// Also note that the size of flashData_t will change, so you may need to
// reevaluate MAX_VARIABLE_SIZE.
#define FLASH_USE_CHECKSUM

/******************************************************************************
 *	Local Settings, Typedefs
 *****************************************************************************/
// This struct defines a sector record.  It must be byte-aligned.  Only the
// first byte of each flag array is used; the rest of the bytes are there so
// that each flag can be written in a single write-operation.
typedef struct __attribute__((__packed__))
{
	uint8_t flag1[MIN_WRITE_SIZE];
	uint8_t flag2[MIN_WRITE_SIZE];
	uint8_t flag3[MIN_WRITE_SIZE];
} flashHeader_t;

// structure stored in flash for each variable.  This structure must be byte-
// aligned (a.k.a. packed), and must have a size that is an exact multiple of
// the MIN_WRITE_SIZE.
typedef struct __attribute__((__packed__))
{
	uint8_t flag;						// variable's status
	uint16_t id;						// variable's id
	uint8_t data[MAX_VARIABLE_SIZE];	// variable's data
#ifdef FLASH_USE_CHECKSUM
	uint8_t checksum;					// 2's complement checksum of id and data
#endif
} flashData_t;

typedef struct							// entry in the variable lookup table
{
	uint16_t id;						// variable's ID
	uint32_t offset;					// variable's location in the sector
} flashTableEntry_t;

// This flag indicates a sector is blank.
#define HEADER_FLAG_EMPTY		0xFFFFFF

// This flag indicates a sector is being initialized.
#define HEADER_FLAG_INIT		0xAAFFFF

// This flag indicates a sector has been initialized.
#define HEADER_FLAG_VALID		0xAAAAFF

// This flag indicates a sector is corrupted or discarded.
#define HEADER_FLAG_INVALID		0xAAAAAA

// This flag indicates a data entry is blank.
#define DATA_FLAG_BLANK			0xFF

// This flag indicates a data entry is valid.
#define DATA_FLAG_VALID			0xAA

// This datatype is used to point to a sector.
typedef uint8_t flashSector_t;

// maximum number of variables supported
#define MAX_VARIABLES	\
	((SECTOR_SIZE - sizeof(flashHeader_t)) / sizeof(flashData_t))

/******************************************************************************
 *	Local variables
 *****************************************************************************/

// Allocate memory for non-volatile memory so it isn't used by the linker
// for something else.  The location is specified.
#pragma DATA_SECTION(sectorArray1, ".infoC")
static uint8_t sectorArray1[SECTOR_SIZE];
#pragma DATA_SECTION(sectorArray2, ".infoD")
static uint8_t sectorArray2[SECTOR_SIZE];

// define sectors
static flashSector_t *sectorPtr1 = sectorArray1;
static flashSector_t *sectorPtr2 = sectorArray2;

// the variable lookup table
static flashTableEntry_t mLookupTable[MAX_VARIABLES];

// number of entries in the lookup table
static uint16_t mNumVariables;

// the next free offset in the valid sector
static uint32_t validFreeOffset;

// pointer to valid sector
static flashSector_t *validSectorPtr;

/******************************************************************************
 *	Local Functions
 *****************************************************************************/

/**
 * @brief	Erases a sector
 * @param	sectorPtr - pointer to flash sector to act upon
 * @returns	true for success; false otherwise	
 */
static bool EraseSector(flashSector_t *sectorPtr)
{
	bool result;	// return value

	// Erase the sector.
	FlashSegmentErase(sectorPtr);

	// Check that it was erased.
	result = FlashEraseCheck(sectorPtr, SECTOR_SIZE);

	return result;
}

/**
 * @brief	Gets the offset of the next free location in a sector
 * @param	sectorPtr - pointer to flash sector to act upon
 * @returns	offset (equals SECTOR_SIZE when sector is full)
 */
uint32_t GetNextFreeOffset(flashSector_t *sectorPtr)
{
	flashData_t *varRec = (flashData_t *)(sectorPtr + sizeof(flashHeader_t));

	// Loop through variable records.
	while (varRec->flag != DATA_FLAG_BLANK)
	{
		varRec++;

		// If we reach the end of sector then we are finished.
		if ((uint8_t *)varRec >= (sectorPtr + SECTOR_SIZE))
		{
			return SECTOR_SIZE;
		}
	}

	return (uint32_t)((uint8_t *)varRec - sectorPtr);
}

/**
 * @brief	Check if a variable record is valid or not.
 * @param	varRec - variable record to check
 * @returns	true if variable is valid; false otherwise
 */
static bool IsVariableRecordValid(flashData_t *varRec)
{
	bool result = true;					// an optimistic return value :)
#ifdef FLASH_USE_CHECKSUM
	uint16_t i;							// counter
	uint8_t checksum;					// 2's complement checksum of data
#endif

	// If the variable record has an invalid flag...
	if (varRec->flag != DATA_FLAG_VALID)
	{
		// The record is invalid.
		result = false;
	}

#ifdef FLASH_USE_CHECKSUM
	// If the flag was fine, check the checksum.
	if (result == true)
	{
		// Calculate the checksum.
		// It's a 2's complement of the ID and DATA.
		checksum = varRec->id & 0xFF;
		checksum += (varRec->id >> 8) & 0xFF;
		for (i = 0; i < MAX_VARIABLE_SIZE; i++)
		{
			checksum += varRec->data[i];
		}
		checksum = 0x100 - checksum;

		// If the checksum is incorrect...
		if (varRec->checksum != checksum)
		{
			// The record is invalid.
			result = false;
		}
	}
#endif

	return result;
}

/**
 * @brief	Construct the lookup table from the valid sector.
 */
static void ConstructLookupTable(void)
{
	flashData_t *varRec = (flashData_t *)(validSectorPtr + sizeof(flashHeader_t));
	uint16_t i;		// counter
	bool found;			// flag to indicate we found a variable

	// Reset the global variable indicating number of variables in memory.
	mNumVariables = 0;

	// Loop through variable records in the sector.
	while (varRec->flag != DATA_FLAG_BLANK)
	{
		// Initialize a flag to say if a variable exists in the lookup table.
		found = false;

		// If variable record is valid then add it to the lookup table.
		if (IsVariableRecordValid(varRec))
		{
			// Search for variable in lookup table. If already found then
			// update existing entry with new offset.
			for (i = 0; i < mNumVariables; i++)
			{
				// If the variable is already in the lookup table...
				if (mLookupTable[i].id == varRec->id)
				{
					// Update existing entry with new offset.
					mLookupTable[i].offset = (uint16_t)((uint8_t *)varRec - validSectorPtr);

					// Set the flag saying the variable is in the lookup table.
					found = true;

					// ??
					break;
				}
			}

			// If variable is not already in lookup table then add it.
			if (!found)
			{
				// Remember the variable ID.
				mLookupTable[mNumVariables].id = varRec->id;

				// Remember where in the sector the variable is stored.
				mLookupTable[mNumVariables].offset = (uint16_t)((uint8_t *)varRec - validSectorPtr);

				// Increment the number of stored variables.
				mNumVariables++;
			}
		}
	
		// Move to next record in the sector.
		varRec++;

		// if reached end of sector then we are finished.
		if ((uint8_t *)varRec >= (validSectorPtr + SECTOR_SIZE))
		{
			break;
		}

		// if reached maximum number of variables then we are finished.
		if (mNumVariables == MAX_VARIABLES)
		{
			break;
		}
	}
}

/**
 * @brief	Write a variable record into a specific sector at a specific offset.
 * @param	varRec - variable record to store
 * @param	sectorPtr - sector to write to
 * @param	offset - offset in sector for variable record
 * @returns	true for success; false otherwise
 */
static bool SetVariableRecord(flashData_t *varRec, flashSector_t *sectorPtr, uint16_t offset)
{
	uint16_t i;							// counter
	bool result = true;					// an optimistic return value :)

	// Write the variable record to the sector.
	FlashWrite8((uint8_t *)varRec, (sectorPtr + offset), sizeof(flashData_t));

	// Verify the data was written correctly.
	for (i = 0; i < sizeof(flashData_t); i++)
	{
		if ((sectorPtr + offset)[i] != ((uint8_t *)varRec)[i])
		{
			result = false;
		}
	}

  return result;
}

/**
 * @brief	Update the flags for a sector.
 * @param	sectorPtr - pointer to sector
 * @param	flags - new flags to write
 * @returns	true for success; false otherwise
 */
static bool SetSectorFlags(flashSector_t *sectorPtr, uint32_t flags)
{
	uint32_t i;							// counter
	flashHeader_t secRec;
	bool result = true;					// an optimistic return value :)

	// Configure sector record.
	secRec.flag1[0] = (flags >> 16) & 0xFF;
	secRec.flag2[0] = (flags >> 8) & 0xFF;
	secRec.flag3[0] = flags & 0xFF;

	// Write to sector.
	FlashWrite8((uint8_t *)&secRec, sectorPtr, sizeof(flashHeader_t));

	// Verify the data was written correctly.
	for (i = 0; i < sizeof(secRec); i++)
	{
		if (sectorPtr[i] != ((uint8_t *)(&secRec))[i])
		{
			result = false;
		}
	}

	return result;
}

/**
 * @brief	Move data from one sector to another, removing old entries.
 * @param	srcSectorPtr - pointer of source sector
 * @param	dstSectorPtr - pointer to destination sector
 * @returns	true for success; false for error
 */
static bool SwapSectors(flashSector_t *srcSectorPtr, flashSector_t *dstSectorPtr)
{
	uint16_t i;							// counter
	flashData_t *tempDataPtr;			// data to put into destination sector
	uint16_t tempOffset;				// address where we'll put the data
	bool status = true;					// an optimistic return value :)

	// Erase the sector if necessary.
	if (dstSectorPtr[0] != 0xFF)
	{
		status = EraseSector(dstSectorPtr);
	}

	// Mark the destination sector as being initialized.
	if (status == true)
	{
		status = SetSectorFlags(dstSectorPtr, HEADER_FLAG_INIT);
	}

	if (status == true)
	{
		// Find the address where we want the first flashData structure.
		// The first data structure goes after the status flags structure.
		tempOffset = sizeof(flashHeader_t);

		// Copy variables to destination sector.
		for (i = 0; i < mNumVariables; i++)
		{
			tempDataPtr = (flashData_t *)(srcSectorPtr + mLookupTable[i].offset);

			status = SetVariableRecord(tempDataPtr, dstSectorPtr, tempOffset);

			// Stop if something goes wrong.
			if (status != true)
			{
				break;
			}

			// Find the address of the next flashData structure.
			tempOffset += sizeof(flashData_t);
		}
	}

	// We're finished with the source sector.  Mark it as invalid.
	if (status == true)
	{
		status = SetSectorFlags(srcSectorPtr, HEADER_FLAG_INVALID);
	}

	// From now on we'll use the destination sector.  Mark it as valid.
	if (status == true)
	{
		status = SetSectorFlags(dstSectorPtr, HEADER_FLAG_VALID);
	}

	// Erase the source sector (so we can use it in the future).
	if (status == true)
	{
		status = EraseSector(srcSectorPtr);
	}

	if (status == true)
	{
		// Update the global variable that points to the sector we're using.
		validSectorPtr = dstSectorPtr;

		// Get next free location in the sector we're using.
		validFreeOffset = GetNextFreeOffset(validSectorPtr);

		// Regenerate lookup table for the new sector.
		ConstructLookupTable();
	}

	return status;
}

/**
 * @brief	Get the offset of a variable into the valid sector
 * @param	variableId - which variable to look for
 * @param	offset - the offset found (if valid)
 * @returns	offset or INVALID_VAR_OFFSET if not found
 */
static bool GetVariableOffset(uint16_t variableId, uint32_t *offset)
{
	uint16_t i;				// counter
	bool status = false;	// pessimistic return value :(

	for (i = 0; i < mNumVariables; i++)
	{
		// Find the offset.
		if (mLookupTable[i].id == variableId)
		{
			*offset = mLookupTable[i].offset;
			status = true;
		}
	}

  return status;
}

/**************************************************************************
Public functions
**************************************************************************/

/**
 * @brief	Initialize access to non-volatile memory.
 * @remarks	Identifies which sector is the valid one and completes any
 * 			partially completed operations that may have been taking place
 * 			before the last reset
 * @returns	true for success; false otherwise
 */
bool FlashManInit(void)
{
	volatile flashHeader_t *mSec1Rec = (flashHeader_t *)(sectorPtr1);
	volatile flashHeader_t *mSec2Rec = (flashHeader_t *)(sectorPtr2);
	uint32_t sector1Flag;
	uint32_t sector2Flag;
	bool status = true;					// an optimistic flag :)

	// Compile sector flags into single values.
	sector1Flag = ((uint32_t)(mSec1Rec->flag1[0]) << 16) | ((uint32_t)(mSec1Rec->flag2[0]) << 8) | mSec1Rec->flag3[0];
	sector2Flag = ((uint32_t)(mSec2Rec->flag1[0]) << 16) | ((uint32_t)(mSec2Rec->flag2[0]) << 8) | mSec2Rec->flag3[0];

	// If sector 1 has invalid flags then erase it and reset the flag.
	if ((sector1Flag != HEADER_FLAG_EMPTY)	&&
		(sector1Flag != HEADER_FLAG_INIT)	&&
		(sector1Flag != HEADER_FLAG_VALID)	&&
		(sector1Flag != HEADER_FLAG_INVALID))
	{
		// TODO:  Should I check the return value from this?
		EraseSector(sectorPtr1);
		sector1Flag = HEADER_FLAG_EMPTY;
	}

	// If sector 2 has invalid flags then erase it and reset the flag.
	if ((sector2Flag != HEADER_FLAG_EMPTY)	&&
		(sector2Flag != HEADER_FLAG_INIT)	&&
		(sector2Flag != HEADER_FLAG_VALID)	&&
		(sector2Flag != HEADER_FLAG_INVALID))
	{
		// TODO:  Should I check the return value from this?
		EraseSector(sectorPtr2);
		sector2Flag = HEADER_FLAG_EMPTY;
	}

	// What happens next depends on status of both sectors:
	switch (sector1Flag)
	{
	case HEADER_FLAG_EMPTY:
		switch (sector2Flag)
		{
		// Sector 1 empty, sector 2 empty.
		case HEADER_FLAG_EMPTY:
			// Set sector 1 as valid.
			status = SetSectorFlags(sectorPtr1, HEADER_FLAG_VALID);
			// Use sector 1.
			validSectorPtr = sectorPtr1;
			validFreeOffset = sizeof(flashHeader_t);
			break;

		// Sector 1 empty, sector 2 initializing.
		case HEADER_FLAG_INIT:
			// Set sector 2 as valid.
			status = SetSectorFlags(sectorPtr2, HEADER_FLAG_VALID);
			// Use sector 2.
			validSectorPtr = sectorPtr2;
			validFreeOffset = sizeof(flashHeader_t);
			break;

		// Sector 1 empty, sector 2 valid.
		case HEADER_FLAG_VALID:
			// Use sector 2.
			validSectorPtr = sectorPtr2;
			validFreeOffset = GetNextFreeOffset(sectorPtr2);
			break;

		// Sector 1 empty, sector 2 invalid.
		case HEADER_FLAG_INVALID:
			// Swap sectors 2 -> 1.
			status = SwapSectors(sectorPtr2, sectorPtr1);
			// Use sector 1.
			validSectorPtr = sectorPtr1;
			validFreeOffset = GetNextFreeOffset(sectorPtr1);
			break;
		}
		break;

	case HEADER_FLAG_INIT:
		switch (sector2Flag)
		{
		// Sector 1 initializing, sector 2 empty.
		case HEADER_FLAG_EMPTY:
			// Set sector 1 as valid.
			status = SetSectorFlags(sectorPtr1, HEADER_FLAG_VALID);
			// Use sector 1.
			validSectorPtr = sectorPtr1;
			validFreeOffset = sizeof(flashHeader_t);
			break;

		// Sector 1 initializing, sector 2 initializing.
		case HEADER_FLAG_INIT:
			// Erase sector 2.
			status = EraseSector(sectorPtr2);
			// Set sector 1 as valid.
			if (status == true)
				status = SetSectorFlags(sectorPtr1, HEADER_FLAG_VALID);
			// Use sector 1.
			validSectorPtr = sectorPtr1;
			validFreeOffset = sizeof(flashHeader_t);
			break;

		// Sector 1 initializing, sector 2 valid.
		case HEADER_FLAG_VALID:
			// Erase sector 1.
			status = EraseSector(sectorPtr1);
			// Swap sectors 2 -> 1.
			if (status == true)
				status = SwapSectors(sectorPtr2, sectorPtr1);
			// Use sector 1.
			validSectorPtr = sectorPtr1;
			validFreeOffset = GetNextFreeOffset(sectorPtr1);
			break;

		// Sector 1 initializing, sector 2 invalid.
		case HEADER_FLAG_INVALID:
			// Erase sector 2.
			status = EraseSector(sectorPtr2);
			// Set sector 1 as valid.
			if (status == true)
				status = SetSectorFlags(sectorPtr1, HEADER_FLAG_VALID);
			// Use sector 1.
			validSectorPtr = sectorPtr1;
			validFreeOffset = sizeof(flashHeader_t);
			break;
		}
		break;

	case HEADER_FLAG_VALID:
		switch (sector2Flag)
		{
		// Sector 1 valid, sector 2 empty.
		case HEADER_FLAG_EMPTY:
			// Use sector 1.
			validSectorPtr = sectorPtr1;
			validFreeOffset = GetNextFreeOffset(sectorPtr1);
			break;

		// Sector 1 valid, sector 2 initializing.
		case HEADER_FLAG_INIT:
			// Erase sector 2.
			status = EraseSector(sectorPtr2);
			// Swap sectors 1 -> 2.
			if (status == true)
				status = SwapSectors(sectorPtr1, sectorPtr2);
			// Use sector 2.
			validSectorPtr = sectorPtr2;
			validFreeOffset = GetNextFreeOffset(sectorPtr2);
			break;

		// Sector 1 valid, sector 2 valid.
		case HEADER_FLAG_VALID:
			// Erase sector 2.
			status = EraseSector(sectorPtr2);
			// Use sector 1.
			validSectorPtr = sectorPtr1;
			validFreeOffset = GetNextFreeOffset(sectorPtr1);
			break;

		// Sector 1 valid, sector 2 invalid.
		case HEADER_FLAG_INVALID:
			// Erase sector 2.
			status = EraseSector(sectorPtr2);
			// Use sector 1.
			validSectorPtr = sectorPtr1;
			validFreeOffset = GetNextFreeOffset(sectorPtr1);
			break;
		}
		break;

	case HEADER_FLAG_INVALID:
		switch (sector2Flag)
		{
		// Sector 1 invalid, sector 2 empty.
		case HEADER_FLAG_EMPTY:
			// Swap sectors 1 -> 2.
			status = SwapSectors(sectorPtr1, sectorPtr2);
			// Use sector 2.
			validSectorPtr = sectorPtr2;
			validFreeOffset = GetNextFreeOffset(sectorPtr2);
			break;

		// Sector 1 invalid, sector 2 initializing.
		case HEADER_FLAG_INIT:
			// Erase sector 1.
			status = EraseSector(sectorPtr1);
			// Set sector 2 as valid.
			if (status == true)
				SetSectorFlags(sectorPtr2, HEADER_FLAG_VALID);
			// Use sector 2.
			validSectorPtr = sectorPtr2;
			validFreeOffset = sizeof(flashHeader_t);
			break;

		// Sector 1 invalid, sector 2 valid.
		case HEADER_FLAG_VALID:
			// Erase sector 1.
			status = EraseSector(sectorPtr1);
			// Use sector 2.
			validSectorPtr = sectorPtr2;
			validFreeOffset = GetNextFreeOffset(sectorPtr2);
			break;

		// Sector 1 invalid, sector 2 invalid.
		case HEADER_FLAG_INVALID:
			// Both sectors invalid so erase both.
			status = EraseSector(sectorPtr1);
			if (status == true)
				status = EraseSector(sectorPtr2);
			// Set sector 1 as valid.
			if (status == true)
				status = SetSectorFlags(sectorPtr1, HEADER_FLAG_VALID);
			// Use sector 1.
			validSectorPtr = sectorPtr1;
			validFreeOffset = sizeof(flashHeader_t);
			break;
		}
		break;
	}
  
	// generate lookup table
	ConstructLookupTable();

	return status;
}

/**
 * @brief	Get the value of a variable.
 * @param	id - variable to act upon
 * @param	value - data fetched from variable
 * @param	size - size of variable [bytes]
 * @returns	true for success; false otherwise
 */
bool FlashManGetVariable(uint16_t id, uint8_t *value, uint16_t size)
{
	uint32_t offset;					// address of data to read from flash
	flashData_t *data;					// data copied from flash
	uint16_t i;							// counter
	bool status = true;					// optimistic return value :)

	// Check for valid size.
	if (size > MAX_VARIABLE_SIZE)
	{
		status = false;
	}

	// Find offset for variable.
	if (status == true)
	{
		status = GetVariableOffset(id, &offset);
	}

	// Continue if we had success.
	if (status == true)
	{
		// Get variable record.
		data = (flashData_t *)(validSectorPtr + offset);

		// Copy data.
		for (i = 0; i < size; i++)
		{
			value[i] = data->data[i];
		}
	}

	return status;
}

/**
 * @brief	Set the value of a variable.
 * @param	id - variable to act upon
 * @param	value - data to store in variable
 * @param	size - size of data [bytes]
 * @returns	true for success; false otherwise
 */
bool FlashManSetVariable(uint16_t id, uint8_t *value, uint16_t size)
{
	uint16_t i;							// counter
	flashData_t flashData;				// structure to be stored in flash
	uint8_t oldData[MAX_VARIABLE_SIZE];	// previous value of the data
	bool preexisting;					// is the variable already in flash?
	bool status = true;					// optimistic return value :)

	// Check for valid size.
	if (size > MAX_VARIABLE_SIZE)
	{
		status = false;
	}

	// Get current value for this variable, if one exists.
	else
	{
		preexisting = FlashManGetVariable(id, oldData, size);

		// If the variable already exists...
		if (preexisting == true)
		{
			// Compare current value with new value.
			for (i = 0; i < size; i++)
			{
				// If new value is different from the old value then we haven't
				// succeeded yet.  If the two values are the same, then we're done.
				if (value[i] != oldData[i])
				{
					preexisting = false;
					break;
				}
			}
		}

		// Do nothing if we've already suceeded (new data is already in memory).
		// If the data is not in memory (preexisting = false) then store it.
		if (preexisting == false)
		{
			// If sector is full...
			if (validFreeOffset >= SECTOR_SIZE)
			{
				// Swap sectors.
				if (validSectorPtr == sectorPtr1)
				{
					status = SwapSectors(sectorPtr1, sectorPtr2);
				}
				else if (validSectorPtr == sectorPtr2)
				{
					status = SwapSectors(sectorPtr2, sectorPtr1);
				}

				// If no space in new sector then no room for more variables.
				if ((status == true) && (validFreeOffset >= SECTOR_SIZE))
				{
					status = false;
				}
			}

			if (status == true)
			{
				// Assemble variable record.
				flashData.flag = DATA_FLAG_VALID;
				flashData.id = id;

	#ifdef FLASH_USE_CHECKSUM
				flashData.checksum = id & 0xFF;
				flashData.checksum += (id >> 8) & 0xFF;
	#endif

				for (i = 0; i < MAX_VARIABLE_SIZE; i++)
				{
					if (i < size)
						flashData.data[i] = value[i];
					else
						flashData.data[i] = 0x00;
	#ifdef FLASH_USE_CHECKSUM
					flashData.checksum += flashData.data[i];
	#endif
				}
	#ifdef FLASH_USE_CHECKSUM
				flashData.checksum = 0x100 - flashData.checksum;
	#endif

				// Store record in sector.
				status = SetVariableRecord(&flashData, validSectorPtr, validFreeOffset);

				if (status == true)
				{
					// Get offset of next free location.
					validFreeOffset += sizeof(flashData_t);

					// Add new variable record to lookup table.
					ConstructLookupTable();
				}
			}
		}
	}

	return status;
}

/**
 * @brief	Get the number of variables Flash will hold.
 * @returns	max number of variables that can be stored by Flash.
 */
uint32_t FlashManGetMaxVariables(void)
{
	return MAX_VARIABLES;
}
