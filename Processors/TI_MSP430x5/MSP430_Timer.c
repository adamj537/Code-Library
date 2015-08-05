//*****************************************************************************
//
// TimerAB.c - Driver for the Timer A & B Modules.
//
//*****************************************************************************

#include <msp430.h>						// required by development platform
#include "Datatypes.h"					// compiler-specific data types
#include "Bitlogic.h"					// handy macros that make life easier
#include "TimerAB.h"					// header for this module

// This macro casts registers appropriately.
#ifndef HWREG16
#define HWREG16(x)	(*((volatile uint16_t *)((uint16_t)x)))
#endif

/* MSP430s differ in the number of timers they have.  This array contains
 * the base address of the registers of all available timers on the device in
 * use.  It is generated at compile-time, and used below to find the registers
 * of the timers in use.  Note that there are three types of timers (A, B, D),
 * and there can be multiple of each type (A0, A1, A2).  Timers of the same
 * type may not have the same number of capture/compare registers (i.e. Timer
 * A0 may have either three or five capture/compare registers, hence the
 * multiple defines for Timer A0).
 */

static const uint16_t timerToBaseAddress[] = {
#if defined(__MSP430_HAS_T0A3__)		// Timer A0
	__MSP430_BASEADDRESS_T0A3__,
#elif defined(__MSP430_HAS_T0A5__)
	__MSP430_BASEADDRESS_T0A5__,
#else
	0xFFFF,
#endif

#if defined(__MSP430_HAS_T1A2__)		// Timer A1
	__MSP430_BASEADDRESS_T1A2__,
#elif defined(__MSP430_HAS_T1A3__)
	__MSP430_BASEADDRESS_T1A3__,
#else
	0xFFFF,
#endif

#if defined(__MSP430_HAS_T2A2__)		// Timer A2
	__MSP430_BASEADDRESS_T2A2__,
#elif defined(__MSP430_HAS_T2A3__)
	__MSP430_BASEADDRESS_T2A3__,
#else
	0xFFFF,
#endif

#ifdef __MSP430_HAS_T3A2__				// Timer A3
	__MSP430_BASEADDRESS_T3A2__,
#else
	0xFFFF,
#endif

#if defined(__MSP430_HAS_T0B3__)		// Timer B0
	__MSP430_BASEADDRESS_T0B3__,
#elif defined(__MSP430_HAS_T0B7__)
	__MSP430_BASEADDRESS_T0B7__,
#else
	0xFFFF,
#endif

#if defined(__MSP430_HAS_T1B3__)		// Timer B1
	__MSP430_BASEADDRESS_T1B3__,
#else
	0xFFFF,
#endif

#ifdef __MSP430_HAS_T2B3__				// Timer B2
	__MSP430_BASEADDRESS_T2B3__,
#else
	0xFFFF,
#endif

#ifdef __MSP430_HAS_T0D3__				// Timer D0
	__MSP430_BASEADDRESS_T0D3__,
#else
	0xFFFF,
#endif

#ifdef __MSP430_HAS_T1D3__				// Timer D1
	__MSP430_BASEADDRESS_T1D3__
#else
	0xFFFF,
#endif
};

/* Timers in the MSP430 can have one of three "types."  This enumeration allows
 * software to determine which timer has which type, so it can call the
 * appropriate configuration functions.
 */
typedef enum {
	TIMER_TYPE_A,
	TIMER_TYPE_B,
	TIMER_TYPE_D,
} timerType_t;

/* This array allows software to figure out what timer has what architecture,
 * which determines how the timer is configured and what its capabilities are.
 */
static const timerType_t timerTypeArray[] = {
	TIMER_TYPE_A,	// Timer A0
	TIMER_TYPE_A,	// Timer A1
	TIMER_TYPE_A,	// Timer A2
	TIMER_TYPE_A,	// Timer A3
	TIMER_TYPE_B,	// Timer B0
	TIMER_TYPE_B,	// Timer B1
	TIMER_TYPE_B,	// Timer B2
	TIMER_TYPE_D,	// Timer D0
	TIMER_TYPE_D,	// Timer D1
};

/**************************************************************************//**
 *
 * \brief Starts timer counter
 *
 * This function assumes that the timer has been previously configured using
 * one of the init functions.  This function does not need to be used if the
 * user has
 *
 * \param baseAddress is the base address of the timer module.
 * \param timerMode mode to put the timer in
 *
 * Modified bits of \b TxCTL register.
 *
 * \return None
 *
 *****************************************************************************/

void TimerAB_StartCounter (uint8_t timer, tmrMode_t timerMode)
{
	uint16_t baseAddress = timerToBaseAddress[timer];

	// Start the timer (by setting it to the desired mode).
	SETMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), timerMode);
}

/**************************************************************************//**
 *
 * \brief Configures timer in continuous mode.
 *
 * This API does not start the timer. Timer needs to be started when required
 * using the TimerStartCounter API.
 *
 * \param baseAddress is the base address of the timer module.
 * \param config is the pointer to struct for continuous mode initialization.
 *
 * Modified bits of \b TxCTL register.
 *
 * \return None
 *
 *****************************************************************************/

void TimerAB_InitContinuousMode(uint8_t timer, sTmrInitContinuous_t *config)
{
	uint16_t baseAddress = timerToBaseAddress[timer];

	// Clear settings in control register.
	HWREG16(baseAddress + ABTIMER_REG_CTL) = 0x00;

	// Clear the input divider extension register (TAIDEXn = TBIDEXn).
	CLEARMASK(HWREG16(baseAddress + ABTIMER_REG_EX0),
			(TAIDEX0 | TAIDEX1 | TAIDEX2));

	// Set the input divider extension register bits.
	SETMASK(HWREG16(baseAddress + ABTIMER_REG_EX0), config->divider & 0x7);

	// Set the clock source, reset timer (if requested) , enable interrupt (if
	// requested), and set the rest of the clock divider bits.
	SETMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), (config->clock +
			config->clear + config->TIE + ((config->divider >> 3) << 6)));

	// If requested, start the timer.
	if (config->start)
	{
		SETMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), TIMER_MODE_CONTINUOUS);
	}
}

/**************************************************************************//**
 *
 * \brief Configures timer in up mode.
 *
 * This API does not start the timer. Timer needs to be started when required
 * using the TimerStartCounter API.
 *
 * \param baseAddress is the base address of the timer module.
 * \param config is the pointer to struct for up mode initialization.
 *
 * Modified bits of \b TxCTL register, bits of \b TxCCTL0 register and bits
 * of \b TxCCR0 register.
 *
 * \return None
 *
 *****************************************************************************/

void TimerAB_InitUpMode (uint8_t timer, sTmrInitUp_t *config)
{
	uint16_t baseAddress = timerToBaseAddress[timer];

	// Clear settings in control register.
	HWREG16(baseAddress + ABTIMER_REG_CTL) = 0x00;

	// Clear the input divider extension register (TAIDEXn = TBIDEXn).
	CLEARMASK(HWREG16(baseAddress + ABTIMER_REG_EX0),
			(TAIDEX0 | TAIDEX1 | TAIDEX2));

	// Set the input divider extension register bits.
	SETMASK(HWREG16(baseAddress + ABTIMER_REG_EX0), config->divider & 0x7);

	// Set the clock source, reset timer (if requested) , enable interrupt (if
	// requested), and set the rest of the clock divider bits.
	SETMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), (config->clock +
			config->clear + config->TIE + ((config->divider >> 3) << 6)));

	// If requested, start the timer.
	if (config->start)
	{
		SETMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), TIMER_MODE_UP);
	}

	// If requested, enable the capture/compare interrupt.
	if (config->CCR0_CCIE == TRUE)
	{
		SETMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), CCIE);
	}
	else
	{
		CLEARMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), CCIE);
	}

	// Set the period (by setting CCR0 register).
	HWREG16(baseAddress + ABTIMER_REG_CCR0) = config->period;
}

/**************************************************************************//**
 *
 * \brief Configures timer in up/down mode.
 *
 * This API does not start the timer. Timer needs to be started when required
 * using the TimerStartCounter API.
 *
 * \param baseAddress is the base address of the timer module.
 * \param config is the pointer to struct for up-down mode initialization.
 *
 * Modified bits of \b TxCTL register, bits of \b TxCCTL0 register and bits
 * of \b TxCCR0 register.
 *
 * \return None
 *
 *****************************************************************************/

void TimerAB_InitUpDownMode(uint8_t timer, sTmrInitUp_t *config)
{
	uint16_t baseAddress = timerToBaseAddress[timer];

	// Clear settings in control register.
	HWREG16(baseAddress + ABTIMER_REG_CTL) = 0x00;

	// Clear the input divider extension register (TAIDEXn = TBIDEXn).
	CLEARMASK(HWREG16(baseAddress + ABTIMER_REG_EX0),
			(TAIDEX0 | TAIDEX1 | TAIDEX2));

	// Set the input divider extension register bits.
	SETMASK(HWREG16(baseAddress + ABTIMER_REG_EX0), config->divider & 0x7);

	// Set the clock source, reset timer (if requested) , enable interrupt (if
	// requested), and set the rest of the clock divider bits.
	SETMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), (config->clock +
			config->clear +	config->TIE + ((config->divider >> 3) << 6)));

	// If requested, start the timer.
	if (config->start)
	{
		SETMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), TIMER_MODE_UPDOWN);
	}

	// If requested, enable the capture/compare interrupt.
	if (config->CCR0_CCIE == TRUE)
	{
		SETMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), CCIE);
	}
	else
	{
		CLEARMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), CCIE);
	}

	// Set the period (by setting CCR0 register).
	HWREG16(baseAddress + ABTIMER_REG_CCR0) = config->period;
}

/**************************************************************************//**
 *
 * \brief Generate a PWM with timer running in up mode
 *
 * \param baseAddress is the base address of the timer module.
 * \param param is the pointer to struct for PWM configuration.
 *
 * Modified bits of \b TxCTL register, bits of \b TxCCTL0 register, bits of
 * \b TxCCR0 register and bits of \b TxCCTLn register.
 *
 * \return None
 *
 *****************************************************************************/

void TimerAB_OutputPWM(uint8_t timer, sTmrInitPWM_t *config)
{
	uint16_t baseAddress = timerToBaseAddress[timer];

	// Clear settings in control register.
	HWREG16(baseAddress + ABTIMER_REG_CTL) = 0x00;

	// Clear the input divider extension register (TAIDEXn = TBIDEXn).
	CLEARMASK(HWREG16(baseAddress + ABTIMER_REG_EX0),
			(TAIDEX0 | TAIDEX1 | TAIDEX2));

	// Set clock divider.
	SETMASK(HWREG16(baseAddress + ABTIMER_REG_EX0), config->divider & 0x7);

	// Set the clock source, reset timer, and set the other clock divider bits.
	// Also, start the timer in "Up Mode".
	SETMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), (config->clock +
			TIMER_MODE_UP + TACLR + ((config->divider >> 3) << 6)));

	// Set the period (by setting CCR0 register).
	HWREG16(baseAddress + ABTIMER_REG_CCR0) = config->period;

	// Disable capture/compare interrupt; clear old output mode setting.
	CLEARMASK(HWREG16(baseAddress + ABTIMER_REG_CCTL0),
			(CCIE | OUTMOD2 | OUTMOD1 | OUTMOD0));

	// Set the desired output mode.
	SETMASK(HWREG16(baseAddress + config->ccRegister), config->outputMode);

	// Set the desired duty cycle (may also be set by another function later).
	HWREG16(baseAddress + ABTIMER_REG_R + config->ccRegister) = config->dutyCycle;
}

/**************************************************************************//**
 *
 * \brief Initializes Capture Mode
 *
 * \param baseAddress is the base address of the timer module.
 * \param config is the pointer to struct for capture mode initialization.
 *
 * Modified bits of \b TxCCTLn register.
 *
 * \return None
 *
 *****************************************************************************/

void TimerAB_InitCaptureMode(uint8_t timer, sTmrInitCapture_t *config)
{
	uint16_t baseAddress = timerToBaseAddress[timer];

	// Select capture mode (clearing this bit sets compare mode).
	SETMASK(HWREG16(baseAddress + config->ccRegister), CAP);

	// Clear old settings.  Don't clear capture mode (which you just set!).
	CLEARMASK(HWREG16(baseAddress + config->ccRegister),
			(CM1 | CM0 | CCIS1 | CCIS0 | SCS));

	// Set capture/compare control register bits.
	SETMASK(HWREG16(baseAddress + config->ccRegister), (config->mode +
			config->inputSelect + config->synchronizeSource +
			config->outputMode + config->ccInterruptEnable));
}

/**************************************************************************//**
 *
 * \brief Initializes Compare Mode
 *
 * \param baseAddress is the base address of the timer module.
 * \param config is the pointer to struct for compare mode initialization.
 *
 * Modified bits of \b TxCCTLn register and bits of \b TxCCRn register.
 *
 * \return None
 *
 *****************************************************************************/

void TimerAB_InitCompareMode(uint8_t timer, sTmrInitCompare_t *config)
{
	uint16_t baseAddress = timerToBaseAddress[timer];

	// Select compare mode (setting this bit sets capture mode).
	CLEARMASK(HWREG16(baseAddress + config->ccRegister), CAP);

	// Clear old settings for output mode, capture/compare interrupt.
	CLEARMASK(HWREG16(baseAddress + config->ccRegister),
			(OUTMOD2 | OUTMOD1 | OUTMOD0 | CCIE));

	// Set output mode and enable interrupt if desired.
	SETMASK(HWREG16(baseAddress + config->ccRegister),
			(config->ccInterruptEnable + config->outputMode));

	// Set the compare value.
	HWREG16(baseAddress + ABTIMER_REG_R + config->ccRegister) = config->compareValue;
}

/******************************************************************************
 *! \brief Set the timer's output mode
 *!
 *! This function modifies part of the CCTLn registers. Modes Toggle/reset,
 *! Set/reset, and Reset/set can't be used with CCR0.
 *!
 *! \param timer - index of the timer of interest
 *! \param ccRegister - the capture/compare register to modify
 *! \param outputMode - the desired timer output mode
 *!
 *! \return None
 *!
 *****************************************************************************/
void TimerAB_SetOutputMode(uint8_t timer, tmrCCR_t ccRegister, tmrOutMode_t outputMode)
{
	uint16_t baseAddress = timerToBaseAddress[timer];

	SETMASK(HWREG16(baseAddress + ccRegister), outputMode);
}

//*****************************************************************************
//
//! \brief Enable timer interrupt
//!
//! Does not clear interrupt flags.
//!
//! \param baseAddress is the base address of the timer module.
//!
//! Modified bits of \b TxCTL register.
//!
//! \return None
//
//*****************************************************************************

void TimerAB_EnableInterrupt (uint8_t timer)
{
	uint16_t baseAddress = timerToBaseAddress[timer];

	SETMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), TAIE);	// TAIE = TBIE = (0x0002)
}

//*****************************************************************************
//
//! \brief Disable timer interrupt
//!
//! \param baseAddress is the base address of the timer module.
//!
//! Modified bits of \b TxCTL register.
//!
//! \return None
//
//*****************************************************************************

void TimerAB_DisableInterrupt (uint8_t timer)
{
	uint16_t baseAddress = timerToBaseAddress[timer];

	CLEARMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), TAIE);	// TAIE = TBIE = (0x0002)
}

//*****************************************************************************
//
//! \brief Get timer interrupt status (from CTL register)
//!
//! \param baseAddress is the base address of the timer module.
//!
//! \return TRUE or FALSE
//
//*****************************************************************************

BOOL TimerAB_IsInterruptPending (uint8_t timer)
{
	uint16_t baseAddress = timerToBaseAddress[timer];
	BOOL status = FALSE;

	if ISMASKSET(HWREG16(baseAddress + ABTIMER_REG_CTL), TAIFG)	// TAIFG = TBIFG = (0x0001)
	{
		status = TRUE;
	}

	return status;
}

//*****************************************************************************
//
//! \brief Reset/Clear the timer clock divider, count direction, count
//!
//! \param baseAddress is the base address of the timer module.
//!
//! Modified bits of \b TxCTL register.
//!
//! \return None
//
//*****************************************************************************

void TimerAB_Clear (uint8_t timer)
{
	uint16_t baseAddress = timerToBaseAddress[timer];

	SETMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), TACLR);	// TACLR = TBCLR = (0x0004)
}

//*****************************************************************************
//
//! \brief Enable capture compare interrupt
//!
//! Does not clear interrupt flags
//!
//! \param baseAddress is the base address of the timer module.
//! \param captureCompareRegister selects the capture compare register being
//!        used. Refer to datasheet to ensure the device has the capture
//!        compare register being used.
//!
//! Modified bits of \b TxCCTLn register.
//!
//! \return None
//
//*****************************************************************************

void TimerAB_EnableCCInterrupt (uint8_t timer, tmrCCR_t ccRegister)
{
	uint16_t baseAddress = timerToBaseAddress[timer];

	SETMASK(HWREG16(baseAddress + ccRegister), CCIE);
}

//*****************************************************************************
//
//! \brief Disable capture compare interrupt
//!
//! \param baseAddress is the base address of the timer module.
//! \param captureCompareRegister selects the capture compare register being
//!        used. Refer to datasheet to ensure the device has the capture
//!        compare register being used.
//!
//! Modified bits of \b TxCCTLn register.
//!
//! \return None
//
//*****************************************************************************

void TimerAB_DisableCCInterrupt (uint8_t timer, tmrCCR_t ccRegister)
{
	uint16_t baseAddress = timerToBaseAddress[timer];

	CLEARMASK(HWREG16(baseAddress + ccRegister), CCIE);
}

//*****************************************************************************
//
//! \brief Get synchronized capture/compare input
//!
//! \param baseAddress is the base address of the timer module.
//! \param captureCompareRegister selects the capture compare register being
//!        used. Refer to datasheet to ensure the device has the capture
//!        compare register being used.
//! \param synchronized selects the type of capture compare input to read
//!        (input bit, output bit, or synchronized input bit)
//!
//! \return TRUE or FALSE
//
//*****************************************************************************

BOOL TimerAB_GetCCBitValue (uint8_t timer, tmrCCR_t ccRegister, tmrCCBit_t bit)
{
	uint16_t baseAddress = timerToBaseAddress[timer];
	BOOL bitValue = FALSE;

	if (ISMASKSET(HWREG16(baseAddress + ccRegister), bit))
	{
		bitValue = TRUE;
	}

	return bitValue;
}

//*****************************************************************************
//
//! \brief Get current capturecompare count
//!
//! \param baseAddress is the base address of the timer module.
//! \param captureCompareRegister selects the capture compare register being
//!        used. Refer to datasheet to ensure the device has the capture
//!        compare register being used.
//!
//! \return Current count as uint16_t
//
//*****************************************************************************

uint16_t TimerAB_GetCCCount (uint8_t timer, tmrCCR_t ccRegister)
{
	uint16_t baseAddress = timerToBaseAddress[timer];

	return  (HWREG16(baseAddress + ABTIMER_REG_R + ccRegister));
}

//*****************************************************************************
//
//! \brief Set output bit for output mode
//!
//! \param baseAddress is the base address of the timer module.
//! \param captureCompareRegister selects the capture compare register being
//!        used. Refer to datasheet to ensure the device has the capture
//!        compare register being used.
//! \param outputModeOutBitValue the value to be set for out bit
//!
//! Modified bits of \b TxCCTLn register.
//!
//! \return None
//
//*****************************************************************************

void TimerAB_SetCCOutBitValue (uint8_t timer, tmrCCR_t ccRegister, BOOL bitValue)
{
	uint16_t baseAddress = timerToBaseAddress[timer];

	CLEARMASK(HWREG16(baseAddress + ccRegister), OUT);

	if (bitValue == FALSE)
		CLEARMASK(HWREG16(baseAddress + ccRegister), OUT);
	else
		SETMASK(HWREG16(baseAddress + ccRegister), OUT);
}

/**************************************************************************//**
 *
 * \brief Stops the timer
 *
 * \param baseAddress is the base address of the timer module.
 *
 * Modified bits of \b TxCTL register.
 *
 * \return None
 *
 *****************************************************************************/

void TimerAB_Stop (uint8_t timer)
{
	uint16_t baseAddress = timerToBaseAddress[timer];

	// Clear the setting bits; seems unnecessary to me, but sample code did it.
	CLEARMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), (MC1 | MC0));

	// Stop the timer by setting it to "Stop Mode".
	SETMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), MC_0);
}

//*****************************************************************************
//
//! \brief Sets the value of the capture-compare register
//!
//! \param baseAddress is the base address of the timer module.
//! \param compareRegister selects the capture compare register being used.
//!			Refer to datasheet to ensure the device has the capture compare
//!			register being used.
//! \param compareValue is the count to be compared with in compare mode
//!
//! Modified bits of \b TxCCRn register.
//!
//! \return None
//
//*****************************************************************************

void TimerAB_SetCompareValue (uint8_t timer, tmrCCR_t ccRegister, uint16_t value)
{
	uint16_t baseAddress = timerToBaseAddress[timer];

	HWREG16(baseAddress + ABTIMER_REG_R + ccRegister) = value;
}

/**************************************************************************//**
 *
 * \brief Clears the timer interrupt flag
 *
 * \param baseAddress is the base address of the timer module.
 *
 * Modified bits are \b TIFG of \b TxCTL register.
 *
 * \return None
 *
 *****************************************************************************/

void TimerAB_ClearTimerInterruptFlag (uint8_t timer)
{
	uint16_t baseAddress = timerToBaseAddress[timer];

	// Clear the flag (TAIFG = TBIFG).
	CLEARMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), TAIFG);
}

//*****************************************************************************
//
//! \brief Clears the capture-compare interrupt flag
//!
//! \param baseAddress is the base address of the timer module.
//! \param captureCompareRegister selects the capture compare register being
//!        used. Refer to datasheet to ensure the device has the capture
//!        compare register being used.
//!
//! Modified bits are \b CCIFG of \b TxCCTLn register.
//!
//! \return None
//
//*****************************************************************************

void TimerAB_ClearCCInterruptFlag (uint8_t timer, tmrCCR_t ccRegister)
{
	uint16_t baseAddress = timerToBaseAddress[timer];

	CLEARMASK(HWREG16(baseAddress + ccRegister), CCIFG);
}

/**************************************************************************//**
 *
 * \brief Selects Timer_B counter length
 *
 * \param baseAddress is the base address of the TIMER_B module.
 * \param counterLength selects the value of counter length.
 *
 * Modified bits are \b CNTL of \b TBxCTL register.
 *
 * \return None
 *
 *****************************************************************************/

void TimerB_SelectCounterLength (uint8_t timer, tmrBCtrBitsize_t counterLength)
{
	uint16_t baseAddress = timerToBaseAddress[timer];
	timerType_t timerType = timerTypeArray[timer];

	// Do nothing if requested for type A timer (which doesn't support this).
	if (timerType == TIMER_TYPE_B)
	{
		// Clear bits containing previous setting.
		CLEARMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), (CNTL1 | CNTL0));

		// Set new setting.
		SETMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), counterLength);
	}
}

/**************************************************************************//**
 *
 * \brief Selects Timer_B Latching Group
 *
 * \param baseAddress is the base address of the TIMER_B module.
 * \param groupLatch selects the latching group.
 *
 * Modified bits are \b TBCLGRP of \b TBxCTL register.
 *
 * \return None
 *
 *****************************************************************************/

void TimerB_SelectLatchingGroup(uint8_t timer, tmrBGroup_t groupLatch)
{
	uint16_t baseAddress = timerToBaseAddress[timer];
	timerType_t timerType = timerTypeArray[timer];

	// Do nothing if requested for type A timer (which doesn't support this).
	if (timerType == TIMER_TYPE_B)
	{
		// Clear bits containing previous setting.
		CLEARMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), (TBCLGRP1 | TBCLGRP0));

		// Set new setting.
		SETMASK(HWREG16(baseAddress + ABTIMER_REG_CTL), groupLatch);
	}
}

//*****************************************************************************
//
//! \brief Selects Compare Latch Load Event
//!
//! \param baseAddress is the base address of the TIMER_B module.
//! \param compareReg selects the compare register being used. Refer to
//!        datasheet to ensure the device has the compare register being used.
//! \param compareLatchLoadEvent selects the latch load event
//!
//! Modified bits are \b CLLD of \b TBxCCTLn register.
//!
//! \return None
//
//*****************************************************************************

void TimerB_InitCompareLatchLoadEvent(uint8_t timer, uint16_t compareReg,
		uint16_t event)
{
	uint16_t baseAddress = timerToBaseAddress[timer];
	timerType_t timerType = timerTypeArray[timer];

	// Do nothing if requested for type A timer (which doesn't support this).
	if (timerType == TIMER_TYPE_B)
	{
		// Clear bits containing previous setting.
		CLEARMASK(HWREG16(baseAddress + compareReg), (CLLD1 | CLLD0));

		// Set new setting.
		SETMASK(HWREG16(baseAddress + compareReg), event);
	}
}

//*****************************************************************************
//
//! \brief Reads the current timer count value
//!
//! Reads the current count value of the timer. There is a majority vote system
//! in place to confirm an accurate value is returned. The TIMER_THRESHOLD
//! #define in the associated header file can be modified so that the votes
//! must be closer together for a consensus to occur.
//!
//! \param baseAddress is the base address of the timer module.
//!
//! \return Majority vote of timer count value
//
//*****************************************************************************

uint16_t TimerAB_GetCounterValue (uint8_t timer)
{
	uint16_t baseAddress = timerToBaseAddress[timer];
	uint16_t voteOne;
	uint16_t voteTwo;
	uint16_t res;

	voteTwo = HWREG16(baseAddress + ABTIMER_REG_R);

	do
	{
		voteOne = voteTwo;
		voteTwo = HWREG16(baseAddress + ABTIMER_REG_R);

		if(voteTwo > voteOne)
		{
			res = voteTwo - voteOne;
		}
		else if (voteOne > voteTwo)
		{
			res = voteOne - voteTwo;
		}
		else
		{
			res = 0;
		}

	} while (res > TIMER_THRESHOLD);

	return voteTwo;
}

//*****************************************************************************
//
//! Close the doxygen group for timer_api
//! @}
//
//*****************************************************************************
