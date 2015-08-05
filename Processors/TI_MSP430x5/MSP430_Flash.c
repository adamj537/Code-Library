//*****************************************************************************
//
// FlashMem.c - Driver for the flashctl Module.
//
//*****************************************************************************

#include <msp430.h>						// required by development platform
#include "Datatypes.h"					// compiler-specific data types
#include "Bitlogic.h"					// handy defines that make life easier

#ifdef __MSP430_HAS_FLASH__
#include "FlashMem.h"

//*****************************************************************************
//
//! \brief Erase a single segment of the flash memory.
//!
//! For devices like MSP430i204x, if the specified segment is the information
//! flash segment, the FLASH_unlockInfo API must be called prior to calling
//! this API.
//!
//! \param flash_ptr is the pointer into the flash segment to be erased
//!
//! \return None
//
//*****************************************************************************

void FlashMemSegmentErase (uint8_t *flash_ptr)
{
	FCTL3 = FWKEY;						// Clear Lock bit.

	FCTL1 = FWKEY | ERASE;				// Set Erase bit.

	*flash_ptr = 0;						// Dummy write to erase flash seg.

	while (FCTL3 & BUSY);				// Wait for BUSY bit to clear.

	FCTL1 = FWKEY;						// Clear ERASE bit.

	FCTL3 = FWKEY + LOCK;				// Set LOCK bit.
}

//*****************************************************************************
//
//! \brief Erase a single bank of the flash memory.
//!
//! This function erases a single bank of the flash memory.  This API will
//! erase the entire flash if device contains only one flash bank.
//!
//! \param flash_ptr is a pointer into the bank to be erased
//!
//! \return None
//
//*****************************************************************************

void FlashMemBankErase (uint8_t *flash_ptr)
{
	FCTL3 = FWKEY;						// Clear Lock bit.
    
	while (FCTL3 & BUSY);				// Wait for BUSY bit to clear.

	FCTL1 = FWKEY | MERAS;				// Set MERAS bit.

	*flash_ptr = 0;						// Dummy write to erase flash seg.

	while (FCTL3 & BUSY);				// Wait for BUSY bit to clear.

	FCTL1 = FWKEY;						// Clear MERAS bit.

	FCTL3 = FWKEY | LOCK;				// Set LOCK bit.
}

//*****************************************************************************
//
//! \brief Erase all flash memory.
//!
//! This function erases all the flash memory banks. For devices like
//! MSP430i204x, this API erases main memory and information flash memory if
//! the FLASH_unlockInfo API was previously executed (otherwise the information
//! flash is not erased). Also note that erasing information flash memory in
//! the MSP430i204x impacts the TLV calibration constants located at the
//! information memory.
//!
//! \param flash_ptr is a pointer into the bank to be erased
//!
//! \return None
//
//*****************************************************************************

void FlashMemMassErase (uint8_t *flash_ptr)
{
	FCTL3 = FWKEY;						// Clear Lock bit.

	while (FCTL3 & BUSY);				// Wait for BUSY bit to clear.

	FCTL1 = FWKEY | MERAS | ERASE;		// Set MERAS bit.

	*flash_ptr = 0;						// Dummy write to erase Flash seg.

	while (FCTL3 & BUSY);				// Wait for BUSY bit to clear.

	FCTL1 = FWKEY;						// Clear MERAS bit.

	FCTL3 = FWKEY | LOCK;				// Set LOCK bit.
}

//*****************************************************************************
//
//! \brief Erase check of the flash memory
//!
//! This function checks bytes in flash memory to make sure that they are in an
//! erased state (are set to 0xFF).
//!
//! \param flash_ptr is the pointer to the starting location of the erase check
//! \param numberOfBytes is the number of bytes to be checked
//!
//! \return TRUE if bytes are erased; FALSE otherwise
//
//*****************************************************************************

BOOL FlashMemEraseCheck (uint8_t *flash_ptr, uint16_t numberOfBytes)
{
	uint16_t i;							// counter
	BOOL result = TRUE;					// return value

	for (i = 0; i < numberOfBytes; i++)	// Loop through each byte.
	{
		if ((*(flash_ptr + i)) != 0xFF)	// If the byte isn't 0xFF (erased)...
		{
			result = FALSE;				// ...then we fail.
		}
	}

	return result;						// Return result.
}

//*****************************************************************************
//
//! \brief Write data into the flash memory in byte format, pass by reference
//!
//! This function writes a byte array of size count into flash memory. Assumes
//! the flash memory is already erased and unlocked. FlashCtl_segmentErase can
//! be used to erase a segment.
//!
//! \param data_ptr is the pointer to the data to be written
//! \param flash_ptr is the pointer into which to write the data
//! \param count number of times to write the value
//!
//! \return None
//
//*****************************************************************************

void FlashMemWrite8 (uint8_t *data_ptr, uint8_t *flash_ptr, uint16_t count)
{
	FCTL3 = FWKEY;						// Clear Lock bit.

	FCTL1 = FWKEY | WRT;				// Enable byte/word write mode.

	while (count > 0)					// For each byte...
	{
		while (FCTL3 & BUSY);			// Wait for BUSY bit to clear.

		*flash_ptr++ = *data_ptr++;		// Write to flash.

		count--;						// Decrement the counter.
	}

	FCTL1 = FWKEY;						// Clear WRT bit.

	FCTL3 = FWKEY | LOCK;				// Set LOCK bit.
}

//*****************************************************************************
//
//! \brief Write data into the flash memory in 16-bit word format, pass by
//! reference
//!
//! This function writes a 16-bit word array of size count into flash memory.
//! Assumes the flash memory is already erased and unlocked.
//! FlashCtl_segmentErase can be used to erase a segment.
//!
//! \param data_ptr is the pointer to the data to be written
//! \param flash_ptr is the pointer into which to write the data
//! \param count number of times to write the value
//!
//! \return None
//
//*****************************************************************************

void FlashMemWrite16 (uint16_t *data_ptr, uint16_t *flash_ptr, uint16_t count)
{
	FCTL3 = FWKEY;						// Clear Lock bit.

	FCTL1 = FWKEY | WRT;				// Enable byte/word write mode.

	while (count > 0)					// For each byte...
	{
		while (FCTL3 & BUSY);			// Wait for BUSY bit to clear.

		*flash_ptr++ = *data_ptr++;		// Write to flash.

		count--;						// Decrement the counter.
	}

	FCTL1 = FWKEY;						// Clear WRT bit.

	FCTL3 = FWKEY | LOCK;				// Set LOCK bit.
}

//*****************************************************************************
//
//! \brief Write data into the flash memory in 32-bit word format, pass by
//! reference
//!
//! This function writes a 32-bit array of size count into flash memory.
//! Assumes the flash memory is already erased and unlocked.
//! FlashCtl_segmentErase can be used to erase a segment.
//!
//! \param data_ptr is the pointer to the data to be written
//! \param flash_ptr is the pointer into which to write the data
//! \param count number of times to write the value
//!
//! \return None
//
//*****************************************************************************

void FlashMemWrite32 (uint32_t *data_ptr, uint32_t *flash_ptr, uint16_t count)
{
	FCTL3 = FWKEY;						// Clear Lock bit.

	FCTL1 = FWKEY | BLKWRT;				// Enable long-word write.

	while (count > 0)					// For each byte...
	{
		while (FCTL3 & BUSY);			// Wait for BUSY bit to clear.

		*flash_ptr++ = *data_ptr++;		// Write to Flash.

		count--;						// Decrement the counter.
	}

	FCTL1 = FWKEY;						// Clear BLKWRT bit.

	FCTL3 = FWKEY | LOCK;				// Set LOCK bit.
}

//*****************************************************************************
//
//! \brief Check FlashCtl status to see if it is currently busy erasing or
//! programming
//!
//! This function checks the status register to determine if the flash memory
//! is ready for writing.
//!
//! \param mask FLASHCTL status to read
//!        Mask value is the logical OR of any of the following:
//!        - \b FLASHCTL_READY_FOR_NEXT_WRITE
//!        - \b FLASHCTL_ACCESS_VIOLATION_INTERRUPT_FLAG
//!        - \b FLASHCTL_PASSWORD_WRITTEN_INCORRECTLY
//!        - \b FLASHCTL_BUSY
//!
//! \return Logical OR of any of the following:
//!         - \b FlashCtl_READY_FOR_NEXT_WRITE
//!         - \b FlashCtl_ACCESS_VIOLATION_INTERRUPT_FLAG
//!         - \b FlashCtl_PASSWORD_WRITTEN_INCORRECTLY
//!         - \b FlashCtl_BUSY
//!         \n indicating the status of the FlashCtl
//
//*****************************************************************************

uint8_t FlashMemStatus (uint8_t mask)
{
    return (FCTL3 & mask);
}

//*****************************************************************************
//
//! \brief Locks the information flash memory segment A
//!
//! This function is typically called after an erase or write operation on the
//! information flash segment is performed by any of the other API functions in
//! order to re-lock the information flash segment.
//!
//! \return None
//
//*****************************************************************************

void FlashMemLockInfoA (void)
{
	//Disable global interrupts while doing RMW operation on LOCKA bit.
	uint16_t gieStatus;
	gieStatus = __get_SR_register() & GIE;          //Store current SR register
	__disable_interrupt();                          //Disable global interrupt

	//Set the LOCKA bit in FCTL3.
	//Since LOCKA toggles when you write a 1 (and writing 0 has no effect),
	//read the register, XOR with LOCKA mask, mask the lower byte
	//and write it back.
	FCTL3 = FWKEY + ((FCTL3 ^ LOCKA) & 0xFF);

	//Reinstate SR register to restore global interrupt enable status
	__bis_SR_register(gieStatus);
}

//*****************************************************************************
//
//! \brief Unlocks the information flash memory segment A
//!
//! This function must be called before an erase or write operation on the
//! information flash segment A is performed by any of the other API functions.
//!
//! \return None
//
//*****************************************************************************

void FlashMemUnlockInfoA (void)
{
	//Disable global interrupts while doing RMW operation on LOCKA bit
	uint16_t gieStatus;
	gieStatus = __get_SR_register() & GIE;          //Store current SR register
	__disable_interrupt();                          //Disable global interrupt

	//Clear the LOCKA bit in FCTL3.
	//Since LOCKA toggles when you write a 1 (and writing 0 has no effect),
	//read the register, mask the lower byte, and write it back.
	FCTL3 = FWKEY + (FCTL3 & 0xFF);

	//Reinstate SR register to restore global interrupt enable status
	__bis_SR_register(gieStatus);
}

#endif
