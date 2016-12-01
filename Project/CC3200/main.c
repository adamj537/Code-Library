#include <stdint.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_ints.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "pin.h"
#include "utils.h"
#include "CC3200_I2C.h"

#define I2C_ADDR	0x5A

static unsigned char ucWriteBuff[256];
static unsigned char ucReadBuff[256];

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

static void BoardInit(void)
{
    // In case of TI-RTOS vector table is initialize by OS itself
#ifndef USE_TIRTOS
    // Set vector table base
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    // Enable global interrupts.
    MAP_IntMasterEnable();

    // Enable fault interrupt.
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

int main(void)
{
	MAP_PinTypeI2C(PIN_01,PIN_MODE_1);
	MAP_PinTypeI2C(PIN_02,PIN_MODE_1);

	BoardInit();

	I2C_Init();
	
	ucWriteBuff[0] = 0x00;

	// Repeat test forever.
	while(1)
	{
		// Do Write Only
		I2C_Transfer(I2C_ADDR, &ucWriteBuff[0], ucReadBuff, 1, 0);

		// Wait for transaction to finish.
		while (I2C_IsBusy());

		// Wait 1 ms.
		UtilsDelay(26666);

		// Do Read Only
		I2C_Transfer(I2C_ADDR, &ucWriteBuff[0], ucReadBuff, 0, 32);

		// Wait for transaction to finish.
		while (I2C_IsBusy());

		// Do Write-then-Read transfer
		I2C_Transfer(I2C_ADDR, &ucWriteBuff[0], ucReadBuff, 1, 32);

		// Wait for transaction to finish.
		while (I2C_IsBusy());
	}
}
