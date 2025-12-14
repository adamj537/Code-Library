/******************************************************************************
 *
 *	Filename:		ASCII-Based Protocol.c
 *
 *	Description:	This is a relatively simple ASCII-based communication
 *					protocol.  It's nice because it's human-readable.  It could
 *					also be expanded into an SCPI library.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *	Function:		ReceiveHexDigit
 *
 *	Description:	Converts a ASCII hex digit to a binary value.  
 *
 *	Parameters:		character - ASCII hex digit to convert; case insensitive.
 *					data - pass a pointer to the binary value; only the four
 *						least significant four bits are altered.
 *
 *	Return value:	1 if a valid hex digit; 0 if not a hex digit
 *
 *****************************************************************************/

static uint8_t ReceiveHexDigit (uint8_t character, uint16_t *data)
{
	// Get rid of the highest 4 bits of data.
    *data <<= 4;

	// If the character is a number...
    if (character >= '0' && character <= '9')
    {
		// Convert to ASCII.
        *data |= character - '0';
		
		// A number if a valid hex digit.
        return 1;
    }

	// If the character is an upper-case letter...
    if (character >= 'A' && character <= 'F')
    {
		// Convert to ASCII.
        *data |= character - 'A' + 10;
		
		// An upper-case letter is a valid hex digit.
        return 1;
    }

	// If the character is a lower-case letter...
    if (character >= 'a' && character <= 'f')
    {
		// Convert to ASCII.
        *data |= character - 'a' + 10;
		
		// A lower-case letter is a valid hex digit.
        return 1;
    }

	// Any other value is not a valid hex digit.
    return 0;
}

/******************************************************************************
 *
 *	Function:		TransmitHexDigit
 *
 *	Description:	Converts a binary value to a single ASCII hex digit (0-F).
 *
 *	Parameters:		data - a pointer to the binary value; only the four least
 *						significant bits are used.
 *
 *	Return value:	the ASCII hex digit
 *
 *****************************************************************************/

static uint8_t TransmitHexDigit (uint16_t *data)
{
    uint8_t character;					// temporary variable and return value
    
	// Set all but the four least significant bits of the data to zero.
    character = *data & 0x000F;
    
	// If the data is a digit...
    if (character < 10)
    {
		// Convert to ASCII digit.
        character += '0';
    }
	
	// Otherwise, it must be a letter...
    else
    {
		// Convert to ASCII letter.
        character += 'A' - 10;
    }
    
	// Shift the data right by four bits.
    *data >>= 4;

	// Return the ASCII hex digit.
    return character;
}

/******************************************************************************
 *
 *	Function:		CommProcess
 *
 *	Description:	Handles the communications link.  Transfers data to and
 *					from the EEPROM and status files.
 *
 *****************************************************************************/

void CommProcess (void)
{
	static int8_t count;				// size of a message (persists in memory)
	static uint8_t buffer[10];			// array for a message (persists in memory)
	static uint8_t transmit;			// state machine variable (persists in memory)
	uint8_t character;
	uint16_t address;
	uint16_t data;

	if (transmit)
	{
		character = buffer[count];		// Find the next character to send.
		if (!UARTSend(character))		// Send a character to the UART.
			return;						// If there was an error, stop.
		count++;
		if (character != '\n')			// If the character is a newline...
			return;
		count = 0;						// Reset the message counter.
		transmit = 0;					// Go to "receive" state.
	}
	else
	{
		// Read a character from the UART.
		if (!UARTRead (&character))
			// If there's an error, stop.
			return;
		if (count < 10)
			buffer[count++] = character;
		if (character != '\n')
			return;
		count = 0;
		transmit = 1;
		switch (buffer[0])
		{
		case 'S':
		case 's':
			// Read from the status file
			address = 0x00;
			if (!ReceiveHexDigit (buffer[1], &address)) break;
			if (!ReceiveHexDigit (buffer[2], &address)) break;
			if (buffer[3] != '\r') break;
			if (buffer[4] != '\n') break;
			if (!StatusGet (address, &data)) break;
			buffer[3] = TransmitHexDigit (&data);
			buffer[2] = TransmitHexDigit (&data);
			buffer[1] = TransmitHexDigit (&data);
			buffer[0] = TransmitHexDigit (&data);
			buffer[4] = '\r';
			buffer[5] = '\n';
			return;

		// Read from the EEPROM file.
		case 'R':
		case 'r':
			address = 0x00;
			if (!ReceiveHexDigit (buffer[1], &address)) break;
			if (!ReceiveHexDigit (buffer[2], &address)) break;
			if (buffer[3] != '\r') break;
			if (buffer[4] != '\n') break;
			if (!ConfigGet (address, &data)) break;
			buffer[3] = TransmitHexDigit (&data);
			buffer[2] = TransmitHexDigit (&data);
			buffer[1] = TransmitHexDigit (&data);
			buffer[0] = TransmitHexDigit (&data);
			buffer[4] = '\r';
			buffer[5] = '\n';
			return;

		// Write to the EEPROM file.
		case 'W':
		case 'w':
			address = 0x00;
			if (!ReceiveHexDigit (buffer[1], &address)) break;
			if (!ReceiveHexDigit (buffer[2], &address)) break;
			data = 0x00;
			if (!ReceiveHexDigit (buffer[3], &data)) break;
			if (!ReceiveHexDigit (buffer[4], &data)) break;
			if (!ReceiveHexDigit (buffer[5], &data)) break;
			if (!ReceiveHexDigit (buffer[6], &data)) break;
			if (buffer[7] != '\r') break;
			if (buffer[8] != '\n') break;
			if (!ConfigPut (address, data)) break;
			buffer[0] = 'O';
			buffer[1] = 'K';
			buffer[2] = '\r';
			buffer[3] = '\n';
			return;    
		}
		
		buffer[0] = '?';
		buffer[1] = '\r';
		buffer[2] = '\n';
    }
}