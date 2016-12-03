/******************************************************************************
 *	Filename:		CC3200_I2C.c
 *
 *	Author:			Adam Johnson
 *
 *	Description:	Master I2C library for CC3200.  Tested on CC3200 LaunchXL
 *					board.
 *
 *****************************************************************************/

#include "rom.h"						// macros to call ROM DriverLib
#include "rom_map.h"					// macros choose ROM or Flash DriverLib
#include "hw_memmap.h"					// defines base address of peripherals
#include "hw_types.h"					// data types required by DriverLib
#include "prcm.h"						// DriverLib - power reset clock manager
#include <stdbool.h>					// defines "bool", "true", "false"
#include <stdint.h>						// defines standard data types
#include <stddef.h>						// defines "NULL"
#include "i2c.h"						// DriverLib - I2C
#include "I2C.h"						// header for this module

#define I2C_CLOCK_FREQ_HZ 	80000000	// master clock frequency

#define I2C_READ			1			// address bit to read from I2C device
#define I2C_WRITE			0			// address bit to write to I2C device

#define I2C_TIMEOUT_VAL		0x7D		// I2C transaction time-out value
										// Set to value 0x7D
										// (20 ms @100KHzs, 5 ms @400Khz
										
#define I2C_NUM_CHANNELS	1			// how many I2C channels we have

/******************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

// The callback function pointer table for callback access
static i2cCallback_t CallbackArray[I2C_NUM_CHANNELS][I2C_NUM_CALLBACKS];

// holds messages that are being received or transmitted
volatile uint8_t *txDataPtr[I2C_NUM_CHANNELS];		// pointer to TX data
volatile uint32_t txByteCount[I2C_NUM_CHANNELS];	// num bytes to TX
volatile uint32_t txBytesDone[I2C_NUM_CHANNELS];	// num bytes sent

volatile uint8_t *rxDataPtr[I2C_NUM_CHANNELS];		// pointer to RX data
volatile uint32_t rxByteCount[I2C_NUM_CHANNELS];	// num bytes to RX
volatile uint32_t rxBytesDone[I2C_NUM_CHANNELS];	// num bytes received

// flags for enabling callbacks (for interrupts that are always enabled)
bool txCallbackEnable[I2C_NUM_CHANNELS];
bool rxCallbackEnable[I2C_NUM_CHANNELS];

// I2C address (used by master and slave configurations)
static uint8_t i2cAddress;

/******************************************************************************
 *
 * 	Function:		I2C_ISR
 *
 * 	Description:	This is the interrupt service routine registered to the
 *					CC3200's I2C peripheral.
 *
 *****************************************************************************/

static void I2C_ISR(void)
{
	uint32_t isrSourceMask;				// bit-flags for interrupt source

	// Identify the source of the interrupt by reading Masked Interrupt Status.
	// The second argument of this function allows us to specify if we want
	// to mask the results so only enabled interrupts are returned to us.
	isrSourceMask = MAP_I2CMasterIntStatusEx(I2CA0_BASE, true);

	// Search for a bit that is set, and deal with it.  Other bits will trigger
	// the ISR after this function returns, and will be dealt with later.  So
	// this block will set the precedence of the interrupt sources.
	// Also, we must clear the interrupt bit of anything we service, or it will
	// cause another interrupt once this function exits.
	if (isrSourceMask & I2C_MASTER_INT_RX_FIFO_REQ)
	{
		// Clear the interrupt flag.
		MAP_I2CMasterIntClearEx(I2CA0_BASE, I2C_MASTER_INT_RX_FIFO_REQ);

		// Process received data.
		while (!(MAP_I2CFIFOStatus(I2CA0_BASE) & I2C_FIFO_RX_EMPTY))
		{
			// Fetch data from the FIFO.
			MAP_I2CFIFODataGetNonBlocking(I2CA0_BASE,
				(uint8_t *)&rxDataPtr[I2C0][rxBytesDone[I2C0]]);

			// Increment the counter.
			if (rxByteCount[I2C0] > rxBytesDone[I2C0])
				rxBytesDone[I2C0]++;

			// If we're done reading...
			if (rxByteCount[I2C0] == rxBytesDone[I2C0])
			{
				// If we're done, send STOP.
				if (txByteCount[I2C0] == txBytesDone[I2C0])
					MAP_I2CMasterControl(I2CA0_BASE,
						I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

				// If we're doing a Read-Then-Write, configure the FIFO
				// and then send a re-START.
				else
				{
					// Set the I2C address + WRITE bit.
					MAP_I2CMasterSlaveAddrSet(I2CA0_BASE, i2cAddress, I2C_WRITE);

					// Set the length of the operation.
					MAP_I2CMasterBurstLengthSet(I2CA0_BASE, txByteCount[I2C0]);

					// Place the first byte into the FIFO.
					MAP_I2CFIFODataPutNonBlocking(I2CA0_BASE, txDataPtr[I2C0][0]);

					// Initiate I2C write operation.
					MAP_I2CMasterControl(I2CA0_BASE,
						I2C_MASTER_CMD_FIFO_BURST_SEND_START |
						I2C_MASTER_CMD_FIFO_BURST_SEND_ERROR_STOP);
				}

				// If the associated callback is enabled and exists, call it.
				if ((txCallbackEnable[I2C0] == true) &&
					(CallbackArray[I2C0][I2C_CB_RX] != NULL))
				CallbackArray[I2C0][I2C_CB_RX](NULL);
			}
		}
	}
	else if ((isrSourceMask & I2C_MASTER_INT_TX_FIFO_REQ) ||
			(isrSourceMask & I2C_MASTER_INT_TX_FIFO_EMPTY))
	{
		// Clear the interrupt flag(s).
		MAP_I2CMasterIntClearEx(I2CA0_BASE, I2C_MASTER_INT_TX_FIFO_REQ);
		MAP_I2CMasterIntClearEx(I2CA0_BASE, I2C_MASTER_INT_TX_FIFO_EMPTY);

		// If we're done with the write...
		if (txByteCount[I2C0] == txBytesDone[I2C0])
		{
			// If we're done, send STOP.
			if (rxByteCount[I2C0] == rxBytesDone[I2C0])
			{
				MAP_I2CMasterControl(I2CA0_BASE, I2C_MASTER_CMD_FIFO_BURST_SEND_ERROR_STOP);
			}

			// If we're doing a Write-Then-Read, configure the FIFO
			// and then send a re-START.
			else
			{
				// Set the I2C address + READ bit.
				MAP_I2CMasterSlaveAddrSet(I2CA0_BASE, i2cAddress, I2C_READ);

				// Set the length of the operation.
				MAP_I2CMasterBurstLengthSet(I2CA0_BASE, rxByteCount[I2C0]);

				// Initiate I2C read operation.
				MAP_I2CMasterControl(I2CA0_BASE,
						I2C_MASTER_CMD_FIFO_BURST_RECEIVE_START |
						I2C_MASTER_CMD_FIFO_BURST_RECEIVE_ERROR_STOP);
			}

			// If the associated callback exists, call it.
			if (CallbackArray[I2C0][I2C_CB_TX] != NULL)
				CallbackArray[I2C0][I2C_CB_TX](NULL);
		}

		// If there's more to send...
		else
		{
			// Put more data in the FIFO.
			MAP_I2CFIFODataPutNonBlocking(I2CA0_BASE,
					txDataPtr[I2C0][txBytesDone[I2C0]]);

			if (txByteCount[I2C0] > txBytesDone[I2C0])
				txBytesDone[I2C0]++;
		}
	}
	else if (isrSourceMask & I2C_MASTER_INT_RX_FIFO_FULL)
	{
		MAP_I2CMasterIntClearEx(I2CA0_BASE, I2C_MASTER_INT_RX_FIFO_FULL);
	}
	else if (isrSourceMask & I2C_MASTER_INT_ARB_LOST)
	{
		MAP_I2CMasterIntClearEx(I2CA0_BASE, I2C_MASTER_INT_ARB_LOST);

		// If the associated callback is enabled and exists, call it.
		if (CallbackArray[I2C0][I2C_CB_ARB_LOST] != NULL)
			CallbackArray[I2C0][I2C_CB_ARB_LOST](NULL);
	}
	else if (isrSourceMask & I2C_MASTER_INT_START)
	{
		MAP_I2CMasterIntClearEx(I2CA0_BASE, I2C_MASTER_INT_START);

		// If the associated callback is enabled and exists, call it.
		if (CallbackArray[I2C0][I2C_CB_START] != NULL)
			CallbackArray[I2C0][I2C_CB_START](NULL);
	}
	else if (isrSourceMask & I2C_MASTER_INT_STOP)
	{
		MAP_I2CMasterIntClearEx(I2CA0_BASE, I2C_MASTER_INT_STOP);

		// If the associated callback is enabled and exists, call it.
		if (CallbackArray[I2C0][I2C_CB_STOP] != NULL)
			CallbackArray[I2C0][I2C_CB_STOP](NULL);
	}
	else if (isrSourceMask & I2C_MASTER_INT_NACK)
	{
		MAP_I2CMasterIntClearEx(I2CA0_BASE, I2C_MASTER_INT_NACK);

		// If the associated callback is enabled and exists, call it.
		if (CallbackArray[I2C0][I2C_CB_NO_ACK] != NULL)
			CallbackArray[I2C0][I2C_CB_NO_ACK](NULL);
	}
	else if (isrSourceMask & I2C_MASTER_INT_TIMEOUT)
	{
		MAP_I2CMasterIntClearEx(I2CA0_BASE, I2C_MASTER_INT_TIMEOUT);
	}

	// This interrupt will fire every time there is a transfer of data.
	// I'm using it to check for errors, and then reset the bus if an error
	// occurs.
	else if (isrSourceMask & I2C_MASTER_INT_DATA)
	{
		// Clear the interrupt flag.
		MAP_I2CMasterIntClearEx(I2CA0_BASE, I2C_MASTER_INT_DATA);

		// Check for errors.
		if(MAP_I2CMasterErr(I2CA0_BASE) != I2C_MASTER_ERR_NONE)
		{
			// Send STOP to reset the bus.
			MAP_I2CMasterControl(I2CA0_BASE, I2C_MASTER_CMD_BURST_SEND_STOP);
		}
	}
}

i2cResult_t GetI2CBase(uint32_t channel, uint32_t *baseAddress)
{
	i2cResult_t result = I2C_RESULT_OK;	// an optimistic return value :)

	// Check for valid handle.
	switch (channel)
	{
#if defined(I2CA0_BASE)
	case I2C0:
		*baseAddress = I2CA0_BASE;
		break;
#endif

	default:
		result = I2C_RESULT_INVALID_SELECTION;
		break;
	}

	return result;
}

/******************************************************************************
 * EXPORTED FUNCTIONS
 *****************************************************************************/

/******************************************************************************
 *
 *	@brief		Initialize the specified I2C with the provided configuration.
 *				If the I2C was initialized previously, I2CDisable() must be
 *				called before re-initializing.
 *
 *	@param[in]	configPtr - pointer to an i2cConfig_t structure with the desired
 *				configuration parameters.
 *
 *	@return		An I2C handle that is used in other calls on success,
 *				I2CIndexInvalid on failure.
 *
 *****************************************************************************/

i2cResult_t I2CInit(i2cConfig_t *configPtr)
{
	i2cResult_t result = I2C_RESULT_OK;	// an optimistic return value :)
	uint32_t base;						// I2C registers' base address
	bool fastModeFlag;					// setting for fast mode

	//*************************************************************************
	// Check all settings.  If they are invalid or unsupported, set the result
	// to invalid.
	//*************************************************************************

	// Check for valid channel.
	result = GetI2CBase(configPtr->channel, &base);

	// Check for valid speed.
	if (configPtr->speed == 100000)
		fastModeFlag = false;
	else if (configPtr->speed == 400000)
		fastModeFlag = true;
	else
		result = I2C_RESULT_INVALID_SELECTION;

	//*************************************************************************
	// Don't do anything else if the result is not "OK".
	//*************************************************************************
	if (result == I2C_RESULT_OK)
	{
		// Enable clock to I2C.
		MAP_PRCMPeripheralClkEnable(PRCM_I2CA0, PRCM_RUN_MODE_CLK);

		// Reset the I2C peripheral.
		MAP_PRCMPeripheralReset(PRCM_I2CA0);

		// Initialize the I2C peripheral (also enables master mode).
		MAP_I2CMasterInitExpClk(base, I2C_CLOCK_FREQ_HZ, fastModeFlag);

		// Disable master mode (because Enable function will turn it on later).
		MAP_I2CMasterDisable(base);

		// Set the time-out. Not to be used with breakpoints.
		MAP_I2CMasterTimeoutSet(base, I2C_TIMEOUT_VAL);

		// Store address for later.
		i2cAddress = configPtr->slaveAddress;
	}

	return result;
}

/******************************************************************************
 *
 *	@brief		Enable the specified I2C peripheral. The I2C must have already
 *				been initialized with I2CInit().
 *
 *	@param[in]	channel The index of the I2C to enable
 *
 *	@return		I2C_RESULT_OK on success, other on failure
 *
 *****************************************************************************/

i2cResult_t I2CEnable(i2cChannel_t channel)
{
	uint32_t base;						// I2C registers' base address
	i2cResult_t result;					// return value

	// Check for valid index.
	result = GetI2CBase(channel, &base);
	
	// Stop here if we didn't find the correct registers.
	if (result == I2C_RESULT_OK)
	{
		// Enable the I2C Module.
		MAP_I2CMasterEnable(base);

		// Register software ISR with the DriverLib.
		MAP_I2CIntRegister(base, I2C_ISR);

		// Clear all interrupts.
		MAP_I2CMasterIntClearEx(base, MAP_I2CMasterIntStatusEx(base, false));

		// Enable interrupts.
		MAP_I2CMasterIntEnableEx(base,
			I2C_MASTER_INT_RX_FIFO_FULL |	// RX FIFO is full
			I2C_MASTER_INT_RX_FIFO_REQ |	// RX FIFO service required
			I2C_MASTER_INT_TX_FIFO_EMPTY |	// TX FIFO is empty
			I2C_MASTER_INT_TX_FIFO_REQ |	// TX FIFO service required
//			I2C_MASTER_INT_NACK |			// no acknowledgment received
//			I2C_MASTER_INT_START |			// START sequence received
//			I2C_MASTER_INT_STOP |			// STOP sequence received
			I2C_MASTER_INT_TIMEOUT |		// timeout
			I2C_MASTER_INT_DATA);			// data transaction complete

		// Flush FIFOs.
		MAP_I2CTxFIFOFlush(base);
		MAP_I2CRxFIFOFlush(base);

		// Enable FIFOs.
		MAP_I2CTxFIFOConfigSet(base, I2C_FIFO_CFG_TX_MASTER | I2C_FIFO_CFG_TX_TRIG_1);
		MAP_I2CRxFIFOConfigSet(base, I2C_FIFO_CFG_RX_MASTER | I2C_FIFO_CFG_RX_TRIG_1);
	}
		
	return result;
}

/******************************************************************************
 *
 *	@brief		Disable the specified I2C peripheral.
 *
 *	@param[in]	channel The index of the I2C to disable
 *
 *	@return		I2C_RESULT_OK on success, other on failure.
 *
 *****************************************************************************/

i2cResult_t I2CDisable(i2cChannel_t channel)
{
	uint32_t base;						// I2C registers' base address
	i2cResult_t result;					// return value

	// Check for valid index.
	result = GetI2CBase(channel, &base);
	
	// Stop here if we didn't find the correct registers.
	if (result == I2C_RESULT_OK)
	{
		// Disable all ISRs associated with I2C.
		MAP_I2CMasterIntDisable(base);

		// Unregister software ISRs with the DriverLib.
		MAP_I2CIntUnregister(base);

		// Disable the I2C Module.
		MAP_I2CMasterDisable(base);

	}

	return result;
}

/******************************************************************************
 *
 *	@brief		(Un)Register a callback function.
 *
 *	@param[in]	channel - index of the I2C to (un)register a callback with.
 *	@param[in]	type - which callback to (un)register
 *	@param[in]	callbackPtr - the function to call when the specified
 *				event occurs.
 *
 *	@return		I2C_RESULT_OK on success, other on failure
 *
 *****************************************************************************/

i2cResult_t I2CRegisterCallback(i2cChannel_t channel, i2cCbType_t type, i2cCallback_t callbackPtr)
{
	uint32_t base;						// dummy variable
	i2cResult_t result;					// return value

	// Check for valid index.
	result = GetI2CBase(channel, &base);
	
	// Stop here if we didn't find the correct registers.
	if (result == I2C_RESULT_OK)
	{
		// Register User callback.
		// If the callback is NULL, that's how we unregister.
		CallbackArray[channel][type] = callbackPtr;
	}
	
	return result;
}

/******************************************************************************
 *
 *	@brief		Enable the selected callback for the specified I2C peripheral
 *
 *	@param[in]	channel - index of the I2C to enable the callback for.
 *	@param[in]	type - which callback to enable.
 *
 *	@return		I2C_RESULT_OK on success, other on failure
 *
 *****************************************************************************/

i2cResult_t I2CEnableCallback(i2cChannel_t channel, i2cCbType_t type)
{
	uint32_t base;						// I2C registers' base address
	i2cResult_t result;				// return value

	// Check for valid index.
	result = GetI2CBase(channel, &base);

	// Stop here if we didn't find the correct registers.
	if (result != I2C_RESULT_OK)
	{
		// Do nothing!
	}
	
	// Enable the interrupt for the requested callback.
	else if (type == I2C_CB_RX)
	{
		// Enable "transaction received" callback.
		rxCallbackEnable[channel] = true;
	}
	else if (type == I2C_CB_TX)
	{
		// Enable "transmit complete" callback.
		txCallbackEnable[channel] = true;
	}
	else if (type == I2C_CB_ARB_LOST)
	{
		// Enable "arbitration lost" interrupt.
		MAP_I2CMasterIntEnableEx(base, I2C_MASTER_INT_ARB_LOST);
	}
	else if (type == I2C_CB_NO_ACK)
	{
		// Enable "no acknowledgement received" interrupt.
		MAP_I2CMasterIntEnableEx(base, I2C_MASTER_INT_NACK);
	}
	else if (type == I2C_CB_START)
	{
		// Enable "START sequence received" interrupt.
		MAP_I2CMasterIntEnableEx(base, I2C_MASTER_INT_START);
	}
	else if (type == I2C_CB_STOP)
	{
		// Enable "STOP sequence received" interrupt.
		MAP_I2CMasterIntEnableEx(base, I2C_MASTER_INT_STOP);
	}
	else
	{
		result = I2C_RESULT_NOT_IMPLEMENTED;
	}
	
	return result;
}

/******************************************************************************
 *
 *	@brief		Disable the selected callback for the specified I2C peripheral
 *
 *	@param[in]	channel - index of the I2C to disable the callback for.
 *	@param[in]	type - which callback to disable
 *
 *	@return		I2C_RESULT_OK on success, other on failure
 *
 *****************************************************************************/

i2cResult_t I2CDisableCallback(i2cChannel_t channel, i2cCbType_t type)
{
	uint32_t base;						// I2C registers' base address
	i2cResult_t result;				// return value

	// Check for valid index.
	result = GetI2CBase(channel, &base);

	// Stop here if we didn't find the correct registers.
	if (result != I2C_RESULT_OK)
	{
		// Do nothing!
	}

	// Enable the interrupt for the requested callback.
	else if (type == I2C_CB_RX)
	{
		// Disable "transaction received" callback.
		rxCallbackEnable[channel] = true;
	}
	else if (type == I2C_CB_TX)
	{
		// Disable "transmit complete" callback.
		txCallbackEnable[channel] = true;
	}
	else if (type == I2C_CB_ARB_LOST)
	{
		// Disable "arbitration lost" interrupt.
		MAP_I2CMasterIntDisableEx(base, I2C_MASTER_INT_ARB_LOST);
	}
	else if (type == I2C_CB_NO_ACK)
	{
		// Disable "no acknowledgement received" interrupt.
		MAP_I2CMasterIntDisableEx(base, I2C_MASTER_INT_NACK);
	}
	else if (type == I2C_CB_START)
	{
		// Disable "Start sequence received" interrupt.
		MAP_I2CMasterIntDisableEx(base, I2C_MASTER_INT_START);
	}
	else if (type == I2C_CB_STOP)
	{
		// Disable "Stop sequence received" interrupt.
		MAP_I2CMasterIntDisableEx(base, I2C_MASTER_INT_STOP);
	}
	else
	{
		result = I2C_RESULT_NOT_IMPLEMENTED;
	}

	return result;
}

/******************************************************************************
 *
 *	@brief		Queries the busy state of the I2C peripheral
 *
 *	@param[in]	channel - index of the I2C to configure the data map
 *
 *	@return		TRUE if the I2C is busy with a communication, FALSE otherwise
 *
 *****************************************************************************/

bool I2CIsBusy(i2cChannel_t channel)
{
	uint32_t base;						// I2C registers' base address
	i2cResult_t result;					// error indicator
	bool busyFlag = false;				// is the I2C peripheral busy?
	bool busBusyFlag = false;			// is the bus busy?

	// Check for valid index.
	result = GetI2CBase(channel, &base);

	// Stop here if we didn't find the correct registers.
	if (result == I2C_RESULT_OK)
	{
		// Check if the I2C bus is busy.
		busBusyFlag = MAP_I2CMasterBusBusy(base);
		busyFlag = MAP_I2CMasterBusy(base);
	}

	return (busyFlag || busBusyFlag);
}

/******************************************************************************
 *
 *	@brief		Perform a write transaction on the bus (as a master).
 *
 *	@param[in]	channel - index of the I2C peripheral to act upon
 *	@param[in]	dataPtr - array of bytes to write to device
 *	@param[in]	count - number of bytes to write
 *
 *****************************************************************************/

i2cResult_t I2CWrite(i2cChannel_t channel, uint8_t *dataPtr, uint8_t count)
{
	i2cResult_t result;				// return value
	uint32_t base;						// I2C registers' base address

	//*************************************************************************
	// Check all settings.  If they are invalid or unsupported, set an error.
	//*************************************************************************

	// Check for valid index.
	result = GetI2CBase(channel, &base);

	// Check for valid count.
	if (count == 0)
		result = I2C_RESULT_INVALID_SELECTION;

	//*************************************************************************
	// Continue only if settings are ok.
	//*************************************************************************
	if (result == I2C_RESULT_OK)
	{
		// Set TX array start address.
		txDataPtr[channel] = dataPtr;

		// Set TX byte count & reset count of bytes sent.
		// Note that before we finish this function, we'll have sent 1 byte.
		txByteCount[channel] = count;
		txBytesDone[channel] = 1;

		// Indicate that we're not reading.
		rxByteCount[channel] = 0;
		rxBytesDone[channel] = 0;

		// Set the I2C address + WRITE bit.
		MAP_I2CMasterSlaveAddrSet(base, i2cAddress, I2C_WRITE);

		// Set the length of the operation.
		MAP_I2CMasterBurstLengthSet(base, count);

		// Place the first byte into the FIFO.
		MAP_I2CFIFODataPutNonBlocking(base, txDataPtr[I2C0][0]);

		// Initiate I2C write operation.
		MAP_I2CMasterControl(base, I2C_MASTER_CMD_FIFO_BURST_SEND_START |
			I2C_MASTER_CMD_FIFO_BURST_SEND_ERROR_STOP);
	}

	return result;
}

/******************************************************************************
 *
 *	@brief		Perform a read transaction on the bus (as a master).
 *
 *	@param[in]	channel - index of the I2C peripheral to act upon.
 *	@param[out]	dataPtr - array to read data into
 *	@param[in]	count - number of bytes to read
 *
 *****************************************************************************/

i2cResult_t I2CRead(i2cChannel_t channel, uint8_t *dataPtr, uint8_t count)
{
	i2cResult_t result;					// return value
	uint32_t base;						// I2C registers' base address

	//*************************************************************************
	// Check all settings.  If they are invalid or unsupported, set an error.
	//*************************************************************************

	// Check for valid index.
	result = GetI2CBase(channel, &base);

	// Check for valid count.
	if (count == 0)
		result = I2C_RESULT_INVALID_SELECTION;

	//*************************************************************************
	// Continue only if settings are ok.
	//*************************************************************************
	if (result == I2C_RESULT_OK)
	{
		// Indicate that we're not writing.
		txByteCount[channel] = 0;
		txBytesDone[channel] = 0;

		// Set RX array start address.
		rxDataPtr[channel] = dataPtr;

		// Set RX byte count & reset count of bytes sent.
		rxByteCount[channel] = count;
		rxBytesDone[channel] = 0;

		// Set the I2C address + READ bit.
		MAP_I2CMasterSlaveAddrSet(base, i2cAddress, I2C_READ);

		// Set the length of the operation.
		MAP_I2CMasterBurstLengthSet(base, count);

		// Initiate I2C read operation.
		MAP_I2CMasterControl(base, I2C_MASTER_CMD_FIFO_BURST_RECEIVE_START |
			I2C_MASTER_CMD_FIFO_BURST_RECEIVE_ERROR_STOP);
	}

	return result;
}

/******************************************************************************
 *
 * @brief Perform a write-then-read transaction on the bus (as a master).
 *
 * @param[in] handle - the index of the I2C peripheral to act upon
 * @param[in] writeDataPtr - array of bytes to write to device
 * @param[in] writeCount - number of bytes to write
 * @param[out] readDataPtr - array to read data into
 * @param[in] readCount - number of bytes to read
 *
 *****************************************************************************/

i2cResult_t I2CWriteThenRead(i2cChannel_t channel, uint8_t *writeDataPtr,
	uint8_t writeCount, uint8_t *readDataPtr, uint8_t readCount)
{
	i2cResult_t result;					// return value
	uint32_t base;						// I2C registers' base address

	//*************************************************************************
	// Check all settings.  If they are invalid or unsupported, set an error.
	//*************************************************************************

	// Check for valid index.
	result = GetI2CBase(channel, &base);

	// Check for valid count.
	if ((writeCount == 0) || (readCount == 0))
		result = I2C_RESULT_INVALID_SELECTION;

	//*************************************************************************
	// Continue only if settings are ok.
	//*************************************************************************
	if (result == I2C_RESULT_OK)
	{
		// Set TX array start address.
		txDataPtr[channel] = writeDataPtr;

		// Set TX byte count & reset count of bytes sent.
		txByteCount[channel] = writeCount;
		txBytesDone[channel] = 1;

		// Set RX array start address.
		rxDataPtr[channel] = readDataPtr;

		// Set RX byte count & reset count of bytes sent.
		rxByteCount[channel] = readCount;
		rxBytesDone[channel] = 0;

		// Set the I2C address + WRITE bit.
		MAP_I2CMasterSlaveAddrSet(base, i2cAddress, I2C_WRITE);

		// Set the length of the write operation.
		MAP_I2CMasterBurstLengthSet(base, writeCount);

		// Place the first byte into the FIFO.
		MAP_I2CFIFODataPutNonBlocking(base, txDataPtr[I2C0][0]);

		// Initiate I2C write operation (but unlike in the I2CWrite function,
		// don't send a STOP condition.
		MAP_I2CMasterControl(base, I2C_MASTER_CMD_FIFO_BURST_SEND_START);
	}

	return result;
}

/******************************************************************************
 *
 * @brief Perform a read-then-write transaction on the bus (as a master) (rarely used!).
 *
 * @param[in] handle - the index of the I2C peripheral to act upon
 * @param[out] readDataPtr - array to read data into
 * @param[in] readCount - number of bytes to read
 * @param[in] writeDataPtr - array of bytes to write to device
 * @param[in] writeCount - number of bytes to write
 *
 *****************************************************************************/

i2cResult_t I2CReadThenWrite(i2cChannel_t channel, uint8_t *readDataPtr,
	uint8_t readCount, uint8_t *writeDataPtr, uint8_t writeCount)
{
	i2cResult_t result;					// return value
	uint32_t base;						// I2C registers' base address

	//*************************************************************************
	// Check all settings.  If they are invalid or unsupported, set an error.
	//*************************************************************************

	// Check for valid index.
	result = GetI2CBase(channel, &base);

	// Check for valid count.
	if ((writeCount == 0) || (readCount == 0))
		result = I2C_RESULT_INVALID_SELECTION;

	//*************************************************************************
	// Continue only if settings are ok.
	//*************************************************************************
	if (result == I2C_RESULT_OK)
	{
		// Set TX array start address.
		txDataPtr[channel] = writeDataPtr;

		// Set TX byte count & reset count of bytes sent.
		txByteCount[channel] = writeCount;
		txBytesDone[channel] = 0;

		// Set RX array start address.
		rxDataPtr[channel] = readDataPtr;

		// Set RX byte count & reset count of bytes sent.
		rxByteCount[channel] = readCount;
		rxBytesDone[channel] = 0;

		// Set the I2C address + READ bit.
		MAP_I2CMasterSlaveAddrSet(base, i2cAddress, I2C_READ);

		// Set the length of the read operation.
		MAP_I2CMasterBurstLengthSet(base, readCount);

		// Initiate I2C read operation.
		MAP_I2CMasterControl(base, I2C_MASTER_CMD_FIFO_BURST_RECEIVE_START |
			I2C_MASTER_CMD_FIFO_BURST_RECEIVE_ERROR_STOP);
	}
	return result;
}

/******************************************************************************
 *	Test Function
 *****************************************************************************/

#define I2C_ADDR	0x41			// I2C address of LaunchPad sensor
#define I2C_SPEED	100000			// 100 kHz

// First message:  Write to sensor.
const uint8_t writeMsg[] = {
	0xFE,	// register to access
};

// Second message:  Read from sensor.  This is for the "write-then-read" test.
const uint8_t readMsg[] = {
	0x00,	// register to read from
};

/******************************************************************************
 *
 *	@brief Test the I2C Master Driver
 *
 *	@return I2C_RESULT_OK on success, other on failure.
 *
 *****************************************************************************/

i2cResult_t I2CTest(i2cChannel_t channel, uint8_t repeatFlag)
{
	i2cResult_t result = I2C_RESULT_OK;	// an optimistic return value :)
	i2cConfig_t config;					// module config data
	volatile uint8_t response[2];		// bytes received from sensor

	do	// I'm afraid of goto statements, so I'll use this instead ;}
	{
		// Setup the I2C configuration.
		config.channel = channel;
		config.speed = I2C_SPEED;
		config.slaveAddress = I2C_ADDR;

		// Setup the I2C driver with the setup configuration.
		result = I2CInit(&config);
		if (result != I2C_RESULT_OK)
			break;

		// Enable the I2C peripheral.
		result = I2CEnable(channel);
		if (result != I2C_RESULT_OK)
			break;

		do	// This part can (optionally) be repeated.
		{
			// Try a write.
			result = I2CWrite(channel, (uint8_t *)writeMsg, sizeof(writeMsg));
			if (result != I2C_RESULT_OK)
				break;

			// Wait until the device is not busy from the previous operation.
			while (I2CIsBusy(channel));

			// Try a read.
			result = I2CRead(channel, (uint8_t *)response, sizeof(response));
			if (result != I2C_RESULT_OK)
				break;

			// Wait until the device is not busy from the previous operation.
			while (I2CIsBusy(channel));

			// Try a write-then-read.
			result = I2CWriteThenRead(channel, (uint8_t *)readMsg,
				sizeof(readMsg), (uint8_t *)response, sizeof(response));
			if (result != I2C_RESULT_OK)
				break;

			// Wait until the device is not busy from the previous operation.
			while (I2CIsBusy(channel));

		} while (repeatFlag);			// Repeat if requested.
	} while (false);					// Don't repeat the initialization.

	return result;
}

