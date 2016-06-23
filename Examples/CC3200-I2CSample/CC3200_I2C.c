/******************************************************************************
 *
 *	Filename:		CC3200_I2C.c
 *
 *	Author:			Adam Johnson
 *
 *	Description:	Non-blocking I2C driver for CC3200.
 *
 *****************************************************************************/
 
#include <stdint.h>						// needed by i2c.h, prcm.h
#include <stdbool.h>					// needed by i2c.h, prcm.h
#include "hw_memmap.h"					// defines base address of peripherals
#include "hw_types.h"					// data types required by DriverLib
#include "hw_ints.h"
#include "rom.h"						// macros to call ROM DriverLib
#include "rom_map.h"					// macros choose ROM or Flash DriverLib
#include "interrupt.h"
#include "prcm.h"						// DriverLib - power reset clock manager
#include "i2c.h"						// DriverLib - I2C

#include "CC3200_I2C.h"					// header for this module

#define I2C_STATE_WRITE			0
#define I2C_STATE_READ			1

static volatile tBoolean bDone;			// flag to signal completed transaction
static uint8_t *g_pucWriteBuff;			// pointer to data to write to slave
static uint8_t *g_pucReadBuff;			// pointer to data read from slave
static uint32_t g_ulWriteSize;			// number of bytes to write to slave
static uint32_t g_ulReadSize;			// number of bytes to read from slave
static uint32_t g_PrevState;			// state machine variable
static uint32_t g_CurState;				// state machine variable
static uint8_t g_ucDevAddr;				// I2C address (used by master and slave configurations)

/******************************************************************************
 *	Description:	This is the interrupt service routine registered to the
 *					CC3200's I2C peripheral.
 *****************************************************************************/
static void I2CIntHandler()
{
	uint32_t ulIntStatus;

	// Identify the source of the interrupt by reading Masked Interrupt Status.
	// The second argument of this function allows us to specify if we want
	// to mask the results so only enabled interrupts are returned to us.
	ulIntStatus = MAP_I2CMasterIntStatusEx(I2CA0_BASE, true);

	// Clear the interrupt flag(s) - early in the ISR, because it can take a
	// few clock cycles to clear the flag.
	MAP_I2CMasterIntClearEx(I2CA0_BASE, ulIntStatus);

	// Search for a bit that is set, and deal with it.  Other bits will trigger
	// the ISR after this function returns, and will be dealt with later.  So
	// this block will set the precedence of the interrupt sources.

	// If the TX FIFO wants more data, and there's data to write...
	if ((ulIntStatus & I2C_MASTER_INT_TX_FIFO_REQ) &&  (0 != g_ulWriteSize))
	{
		// Put more data into the FIFO.
		MAP_I2CFIFODataPutNonBlocking(I2CA0_BASE, (uint8_t)(*g_pucWriteBuff));
		
		// Decrease the message size.
		g_ulWriteSize--;
		
		// Increment the index.
		g_pucWriteBuff++;
		
		// Deal with the special "Write-Then-Read" case.
		// If we're done writing, and there's still data to read...
		if (0 == g_ulWriteSize && 0 != g_ulReadSize)
			// Go to "we're reading" state.
			g_CurState = I2C_STATE_READ;
	}

	// If we're doing a "Write-Then-Read" and the transmit FIFO is empty...
	else if (ulIntStatus & I2C_MASTER_INT_TX_FIFO_EMPTY)
	{
		if ((g_CurState == I2C_STATE_READ) && (g_PrevState == I2C_STATE_WRITE))
		{
			// Set the Master Address
			MAP_I2CMasterSlaveAddrSet(I2CA0_BASE, g_ucDevAddr, true);

			// Set the burst length
			MAP_I2CMasterBurstLengthSet(I2CA0_BASE, (uint8_t)g_ulReadSize);

			// Update the previous state.
			g_PrevState = I2C_STATE_READ;

			// Issue the command
			MAP_I2CMasterControl(I2CA0_BASE, I2C_MASTER_CMD_FIFO_SINGLE_RECEIVE);
		}
	}

	// If the RX FIFO has data to give, and we want to read...
	else if ((ulIntStatus & I2C_MASTER_INT_RX_FIFO_REQ) && (0 != g_ulReadSize))
	{
		// Fetch data from the FIFO.
		MAP_I2CFIFODataGetNonBlocking(I2CA0_BASE, (uint8_t *)g_pucReadBuff);
		
		// Decrease the message size.
		g_ulReadSize--;
		
		// Increment the index.
		g_pucReadBuff++;
	}

	// If we reach the end of a message...
	if (ulIntStatus & I2C_MASTER_INT_STOP)
	{
		// Set the "we're done" flag.
		bDone = true;

		// Stop the I2C peripheral, because the silly thing will keep going.
		MAP_I2CMasterControl(I2CA0_BASE, 0);
	}
}

void I2C_Init()
{
	// Enable clock to I2C.
	MAP_PRCMPeripheralClkEnable(PRCM_I2CA0, PRCM_RUN_MODE_CLK);
	
	// Configure the clock.
	MAP_I2CMasterInitExpClk(I2CA0_BASE, 80000000, true);
	
	// Configure I2C interrupts.
	MAP_I2CIntRegister(I2CA0_BASE, I2CIntHandler);
	
	// Clear all interrupts.
	MAP_I2CMasterIntClearEx(I2CA0_BASE, 0xFFFF);

	// Enable the required interrupts.
	MAP_I2CMasterIntEnableEx(I2CA0_BASE, I2C_MASTER_INT_RX_FIFO_REQ |
		I2C_MASTER_INT_TX_FIFO_REQ | I2C_MASTER_INT_TX_FIFO_EMPTY |
		I2C_MASTER_INT_STOP);

	// Configure the I2C TX FIFO.
	MAP_I2CTxFIFOConfigSet(I2CA0_BASE,I2C_FIFO_CFG_TX_MASTER |
		I2C_FIFO_CFG_TX_TRIG_4);

	// Configure the I2C RX FIFO.
	MAP_I2CRxFIFOConfigSet(I2CA0_BASE,I2C_FIFO_CFG_RX_MASTER |
		I2C_FIFO_CFG_RX_TRIG_4);

	// Enable the I2C Master.
	MAP_I2CMasterEnable(I2CA0_BASE);
}

tBoolean I2C_IsBusy(void)
{
	return !bDone;
}

//*****************************************************************************
// Read/Write API
//*****************************************************************************
int I2C_Transfer( uint8_t ucDevAddr, uint8_t *ucWriteBuffer,
                  uint8_t *ucReadBuffer, uint32_t ulWriteSize,
                  uint32_t ulReadSize)
{
	uint32_t ulCmd;

	// Set the flag to false.
	bDone = false;

	// Set the device address.
	g_ucDevAddr =  ucDevAddr;

	// Set the transfer size
	if (0 != ulWriteSize || 0 != ulReadSize)
	{
		g_ulWriteSize = ulWriteSize;
		g_ulReadSize = ulReadSize;
	}
	else
	{
		// Error
		return -1;
	}

	// Set the buffer
	g_pucWriteBuff = ucWriteBuffer;
	g_pucReadBuff = ucReadBuffer;

	// Form the command
	if (0 != g_ulWriteSize)
	{
		// Set the Master Address
		MAP_I2CMasterSlaveAddrSet(I2CA0_BASE, ucDevAddr, false);

		// Set the burst length
		MAP_I2CMasterBurstLengthSet(I2CA0_BASE, (uint8_t)ulWriteSize);

		// Current state
		g_PrevState = I2C_STATE_WRITE;
		g_CurState = I2C_STATE_WRITE;

		// Set the start
		ulCmd = I2C_MASTER_CMD_FIFO_BURST_SEND_START;

		if (0 == ulReadSize)
			ulCmd = I2C_MASTER_CMD_FIFO_SINGLE_SEND;
	}
	else
	{
		// Set the Master Address
		MAP_I2CMasterSlaveAddrSet(I2CA0_BASE, ucDevAddr, true);

		// Set the burst length
		MAP_I2CMasterBurstLengthSet(I2CA0_BASE, (uint8_t)ulReadSize);

		g_PrevState = I2C_STATE_READ;
		g_CurState = I2C_STATE_READ;

		ulCmd = I2C_MASTER_CMD_FIFO_SINGLE_RECEIVE;
	}

	// Issue the command
	MAP_I2CMasterControl(I2CA0_BASE, ulCmd);

	// Success
	return 0;
}
