#include <stdint.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_ints.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "i2c.h"

#include "CC3200_I2C.h"

static tBoolean bDone;
static unsigned char *g_pucWriteBuff;
static unsigned char *g_pucReadBuff;
static unsigned long g_ulWriteSize;
static unsigned long g_ulReadSize;
static unsigned long g_PrevState;
static unsigned long g_CurState;
static unsigned char g_ucDevAddr;
static unsigned long g_ulDebugReadCount;
static unsigned long g_ulDebugWriteCount;

#define I2C_RW_FLAG_SEND_START          1
#define I2C_RW_FLAG_SEND_STOP           2

#define I2C_STATE_WRITE         0
#define I2C_STATE_READ          1

//*****************************************************************************
// Int handler for I2C
//*****************************************************************************
void I2CIntHandler()
{
	unsigned long ulIntStatus;

	// Get the interrupt status
	ulIntStatus = MAP_I2CMasterIntStatusEx(I2CA0_BASE,true);

	// Clear the interrupt flag(s) - early in the ISR, because it can take a
	// few clock cycles to clear the flag.
	MAP_I2CMasterIntClearEx(I2CA0_BASE, ulIntStatus);

	// See if fifo fill request
	if ((ulIntStatus & I2C_MASTER_INT_TX_FIFO_REQ) &&  (0 != g_ulWriteSize))
	{
		// Put more data into the FIFO.
		MAP_I2CFIFODataPutNonBlocking(I2CA0_BASE, (uint8_t)(*g_pucWriteBuff));
		
		// Decrease the message size.
		g_ulWriteSize--;
		
		// Increment the index.
		g_pucWriteBuff++;
		
		g_ulDebugWriteCount++;
		
		// Set "we're done" flag after the whole message is transmitted.
		if (0 == g_ulWriteSize && 0 != g_ulReadSize)
			g_CurState = I2C_STATE_READ;
	}
	
	else if (ulIntStatus & I2C_MASTER_INT_TX_FIFO_EMPTY)
	{
		if ((g_CurState == I2C_STATE_READ) && (g_PrevState == I2C_STATE_WRITE))
		{
			// Set the Master Address
			MAP_I2CMasterSlaveAddrSet(I2CA0_BASE, g_ucDevAddr, true);

			// Set the burst length
			MAP_I2CMasterBurstLengthSet(I2CA0_BASE, (uint8_t)g_ulReadSize);

			g_PrevState = I2C_STATE_READ;

			// Issue the command
			MAP_I2CMasterControl(I2CA0_BASE, I2C_MASTER_CMD_FIFO_SINGLE_RECEIVE);
		}
	}
	
	else if ((ulIntStatus & I2C_MASTER_INT_RX_FIFO_REQ) && (0 != g_ulReadSize))
	{
		// Fetch data from the FIFO.
		MAP_I2CFIFODataGetNonBlocking(I2CA0_BASE, (uint8_t *)g_pucReadBuff);
		
		// Decrease the message size.
		g_ulReadSize--;
		
		// Increment the index.
		g_pucReadBuff++;
		
		g_ulDebugReadCount++;
	}

	if (ulIntStatus & I2C_MASTER_INT_STOP)
	{
		bDone = true;
		MAP_I2CMasterControl(I2CA0_BASE, 0);
	}

	// Clear the interrupt
	// Moved this call to the top of the interrupt routine, because it can take
	// a few clock cycles to clear the flag.
//	MAP_I2CMasterIntClearEx(I2CA0_BASE, ulIntStatus);
}

void I2C_Init()
{
	// Enable the I2C module.
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
int I2C_Transfer( unsigned char ucDevAddr, unsigned char *ucWriteBuffer,
                  unsigned char *ucReadBuffer, unsigned long ulWriteSize,
                  unsigned long ulReadSize)
{
	unsigned long ulCmd;

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
