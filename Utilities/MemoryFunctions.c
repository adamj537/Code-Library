/******************************************************************************
 * Functions to store, search, and recall memory from EEPROM.
 * EEPROM is divided into two sections, called "Memory Values" and "Indexes".
 * There is one index for each memory value.
 * When a new value is saved, it goes into the next free Memory Value slot.
 * The corresponding index slot saves an ID value.
 * To search for a value by ID, search for the desired index, and then read the
 * memory value with the same offset.
 * In this way, values can be added, deleted, and read arbitrarily.
 *****************************************************************************/

#define NUM_MEMORY				40		// number of stored readings allowed

// User saved measurements
#define MEM_SAVED_ADDR			40		// 2 bytes - uint16 - number of readings saved
#define MEM_ADDR				42		// 160 bytes - array of floats (4 bytes each)*
#define MEM_INDEX_ADDR			202		// 40 bytes - uint8 - index of each reading

float MemFetch(uint16_t index)
{
	float value;						// value fetched from EEPROM
	uint8_t addr;						// address counter
	volatile uint8_t i;					// index of value in memory
	uint8_t foundIndex = 0xFF;			// desired index or the next higher one
	uint8_t foundAddr;					// data value corresponding to foundIndex
	uint8_t lowestIndex = 0xFF;			// lowest valid index in memory
	uint8_t lowestAddr;					// data value corresponding to lowestIndex

	// Find either the desired index or the first full location after that.
	for (addr = 0; addr < NUM_MEMORY; addr++)
	{
		i = EEPROMreadByte(MEM_INDEX_ADDR + addr);

		// Find either the desired index or the next highest one.
		if ((index <= i) && (i < foundIndex))
		{
			foundIndex = i;
			foundAddr = addr;
		}

		// Also find the lowest index overall.
		if (i < lowestIndex)
		{
			lowestIndex = i;
			lowestAddr = addr;
		}
	}

	// If there was nothing at or above the desired index, use lowest index.
	// If the lowest index is invalid, memory is empty.
	if (foundIndex == 0xFF)
	{
		foundIndex = lowestIndex;
		foundAddr = lowestAddr;
	}

	if (foundIndex != 0xFF)
	{
		// Fetch the memory value.
		value = EEPROMreadFloat(MEM_ADDR + (4 * foundAddr));
	}
	else
	{
		// Alert the user that memory is empty.
		value = NaN;
	}

	return value;
}

uint8_t AppMemUp(uint8_t currentIndex)
{
	uint8_t addr;						// address counter
	volatile uint8_t index;				// index of value in memory
	uint8_t foundIndex = 0xFF;			// greatest index <= desired index
	uint8_t lowestIndex = 0xFF;			// greatest index overall
	uint8_t nextIndex;					// return value

	// Find either:
	//	(1) the desired index (memoryIndex + 1), or
	//	(2) the lowest index that's higher than the desired index, or
	//	(3) the lowest index overall.

	// Search through the saved readings.
	for (addr = 0; addr < NUM_MEMORY; addr++)
	{
		// Fetch the index.
		index = EEPROMreadByte(MEM_INDEX_ADDR + addr);

		// (1) and (2) can be done in a single if statement:
		// Look for the lowest index between 0xFF and the desired index.
		if (((currentIndex + 1) <= index) && (index < foundIndex))
		{
			foundIndex = index;
		}

		// Look for (3) the lowest index overall.
		if (index > lowestIndex)
		{
			lowestIndex = index;
		}
	}

	// If there was nothing at or above the desired index, use lowest index.
	// If the lowest index is invalid, memory is empty.
	if (foundIndex != 0xFF)
	{
		nextIndex = foundIndex;
	}
	else
	{
		nextIndex = lowestIndex;
	}

	return nextIndex;
}

uint8_t AppMemDown(uint8_t currentIndex)
{
	uint8_t addr;						// address counter
	volatile int8_t index;				// index of value in memory
	int8_t foundIndex = -1;				// greatest index <= desired index
	int8_t highestIndex = -1;			// greatest index overall
	uint8_t nextIndex;					// return value

	// Find either:
	//	(1) the desired index (memoryIndex - 1), or
	//	(2) the highest index that's lower than the desired index, or
	//	(3) the highest index overall.

	// Search through the saved readings.
	for (addr = 0; addr < NUM_MEMORY; addr++)
	{
		// Fetch the index.
		index = (int8)EEPROMreadByte(MEM_INDEX_ADDR + addr);

		// (1) and (2) can be done in a single if statement:
		// Look for the highest index between 0 and the desired index.
		if ((foundIndex < index) && (index <= (currentIndex - 1)))
		{
			foundIndex = index;
		}

		// Look for (3) the highest index overall.
		if (index > highestIndex)
		{
			highestIndex = index;
		}
	}

	// If there was nothing at or below the desired index, use greatest index.
	// If the greatest index is invalid, memory is empty.
	if (foundIndex != -1)
	{
		nextIndex = foundIndex;
	}
	else
	{
		nextIndex = highestIndex;
	}

	return nextIndex;
}

uint8_t AppSaveMem(void)
{
	mErr_t error;						// error code from manometer
	uint8_t addr;
	volatile uint8_t index;
	uint8_t emptyAddr = 0xFF;				// empty spot for new reading
	int8 newIndex = -1;					// index of new reading
	float value;						// value to be saved
	volatile uint8_t numSaved = 0;		// number of saved readings

	for (addr = 0; addr < NUM_MEMORY; addr++)
	{
		index = EEPROMreadByte(MEM_INDEX_ADDR + addr);

		// Find the first empty slot in EEPROM.
		if ((emptyAddr == 0xFF) && (index == 0xFF))	// 0xFF means "I'm empty."
		{
			emptyAddr = addr;
		}

		// Find the highest index.  This is the number of stored readings.
		// Casting index to signed integer makes 0xFF (empty) = -1.
		if ((int8)index >= newIndex)
		{
			newIndex = index + 1;
		}
	}

	// If memory is full...
	if (emptyAddr > NUM_MEMORY)
	{
		DisplayAlphaString("ERR", 3);	// ...tell the user.
		DisplayMainString("FULL", 4);
	}

	else	// If memory is available, fetch, then save reading.
	{
		// Show the user what reading we're saving.
		DisplayAlphaString("MEM", 3);
		DisplayMainInt(newIndex + 1);

		// Fetch the most recent process value.
		error = ManoGetProcessValue(&value);

		// If there's no error...
		if (error == MAN_OK)
		{
			// Save the reading.
			EEPROMwriteFloat(MEM_ADDR + (4 * emptyAddr), value);

			// Save the reading's index.
			EEPROMwriteByte(MEM_INDEX_ADDR + emptyAddr, newIndex);

			// Update the count of saved readings.
			numSaved = EEPROMreadByte(MEM_SAVED_ADDR);
			EEPROMwriteByte(MEM_SAVED_ADDR, ++numSaved);
//			EEPROMwriteByte(MEM_SAVED_ADDR, newIndex);
		}

		// If there was an error (overflow, underflow, sensor error), let
		// the user know.
		else
		{
			DisplayMainString("Err1", 4);
		}
	}

	TimerDelay(500);	// Pause for dramatic effect.

	return ST_HOME;		// Go to "Home" state.
}

uint8_t AppEraseMem(void)
{
	uint8_t addr;						// counter
	volatile uint16_t index;			// index of value in memory
	volatile uint16_t numSaved = 0;	// highest index saved in memory

	// Erase all saved readings.
	if (g_setting == SETTING_ALL)
	{
		// Reset the memory index.
		g_memoryIndex = 0;

		// Erase each reading.
		for (addr = 0; addr < NUM_MEMORY; addr++)
		{
			// Write 0xFF for "empty".
			EEPROMwriteByte(MEM_INDEX_ADDR + addr, 0xFF);
		}

		// Clear the count of saved readings.
		EEPROMwriteWord(MEM_SAVED_ADDR, 0);
	}

	// Erase one reading.
	else if (g_setting == SETTING_ONE_YES)
	{
		// Search for desired memory index.
		for (addr = 0; addr < NUM_MEMORY; addr++)
		{
			index = EEPROMreadByte(MEM_INDEX_ADDR + addr);
			if (index == g_memoryIndex)
			{
				// Change to 0xFF for "empty".
				index = 0xFF;

				// Save to EEPROM.
				EEPROMwriteByte(MEM_INDEX_ADDR + addr, index);

				// Decrement the memory index.
				g_memoryIndex--;
			}

			// Find the (new) number of saved readings.
			if ((int8)index >= 0)
			{
				numSaved++;
			}
		}

		// Update the number of saved readings.
		EEPROMwriteByte(MEM_SAVED_ADDR, numSaved);
	}

	return ST_MEM_VIEW;	// Go to the "view" state.
}