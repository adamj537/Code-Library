/*****************************************************************************
 *
 * 	Filename:		CC3200_ADC.c
 *
 * 	Author:			Adam Johnson
 *
 * 	Description:	ADC library for CC3200.  Tested on CC3200 LaunchXL board.
 *
 *	Terms of Use:	MIT License
 *
 *****************************************************************************/

#include "rom.h"						// macros to call ROM DriverLib
#include "rom_map.h"					// macros choose ROM or Flash DriverLib
#include "hw_memmap.h"					// defines base address of peripherals
#include "hw_types.h"					// data types required by DriverLib
#include <stdbool.h>					// needed by adc.h, prcm.h
#include <stdint.h>						// needed by adc.h, prcm.h
#include <stddef.h>						// defines "NULL"
#include "prcm.h"						// DriverLib - power reset clock manager
#include "adc.h"						// DriverLib - ADC
#include "project.h"					// global project settings
#include "ADC.h"						// header for this module

#define ADC_NUM_CHANNELS	4			// number of channels in the processor
#define MAX_ADC_INTERRUPTS	2			// number of items in adcCbType_t

// The callback function pointer table for callback access
static adcCallback_t CallbackArray[MAX_ADC_INTERRUPTS];

// remembers ADC channel currently in use
static volatile uint32_t adcActiveChannelCode;
static volatile adcChannel_t adcActiveChannel;

static volatile uint32_t adcNumSamplesDesired;
static volatile uint32_t adcNumSamplesAcquired;
static volatile adcSample_t *adcSamplesArray;

/******************************************************************************
 *
 *	Function:		AdcISR
 *
 *	Description:	This is the interrupt service routine registered to the
 *					CC3200's ADC peripheral.
 *
 *****************************************************************************/

void AdcISR (void)
{
	uint32_t isrSourceMask;		// bit-flags for interrupt source
	uint32_t rawData;
//	uint32_t timestamp;
//	uint32_t voltage;

	// Identify the source of the interrupt by reading Interrupt Status.
	isrSourceMask = MAP_ADCIntStatus(ADC_BASE, adcActiveChannelCode);

	// Clear the interrupt flag.
	MAP_ADCIntClear(ADC_BASE, adcActiveChannelCode, isrSourceMask);

	if ((isrSourceMask & ADC_FIFO_OVERFLOW) ||
		(isrSourceMask & ADC_FIFO_FULL))
	{
		// Call callback when conversions are complete.
		if (adcNumSamplesAcquired >= adcNumSamplesDesired)
		{
			// Disable all ADC channels.
			MAP_ADCChannelDisable(ADC_BASE, adcActiveChannelCode);

			// Disable the ADC's internal timer.
			MAP_ADCTimerDisable(ADC_BASE);

			// Disable the ADC.
			MAP_ADCDisable(ADC_BASE);

			// Unregister interrupt with the timer.
			MAP_ADCIntUnregister(ADC_BASE, adcActiveChannelCode);

			// Call a callback if requested.
			if (CallbackArray[ADC_CB_DONE] != NULL)
			{
				CallbackArray[ADC_CB_DONE](adcActiveChannel);
			}
		}

		// Read samples if conversion is not complete.
		else if (MAP_ADCFIFOLvlGet(ADC_BASE, adcActiveChannelCode))
		{
			rawData = MAP_ADCFIFORead(ADC_BASE, adcActiveChannelCode);

			// Extract the timestamp from the ADC data.
//			timestamp = (rawData >> 14) & 0x1FFFF;

			// Extract the sample from the ADC data.
			adcSamplesArray[adcNumSamplesAcquired++] = (rawData >> 2) & 0xFFF;
		}
	}

	else if ((isrSourceMask & ADC_FIFO_UNDERFLOW) ||
			 (isrSourceMask & ADC_FIFO_EMPTY))
	{
		// Call ADC overrun callback if requested.
		if (CallbackArray[ADC_CB_ERROR] != NULL)
		{
			CallbackArray[ADC_CB_ERROR](adcActiveChannel);
		}
	}
}

/*******************************************************************************
 *
 *	Description:	Initialize the specified ADC with the provided settings.
 *
 *	Parameters:		configPtr - pointer to a structure with the desired
 *						ADC configuration parameters
 *
 *	Return value:	ADC_RESULT_OK on success; other on failure
 *
 ******************************************************************************/

adcResult_t AdcInit(adcConfig_t *configPtr)
{
	adcResult_t result = ADC_RESULT_OK;	// an optimistic return value :)

	//*************************************************************************
	// Check all settings.  If they are invalid or unsupported, set the result
	// to invalid.
	//*************************************************************************

	// Check for invalid channel.
	if (configPtr->channel >= ADC_NUM_CHANNELS)
		result = ADC_RESULT_INVALID_SELECTION;
	
	// Differential mode is not supported.
	else if (configPtr->differential != false)
		result = ADC_RESULT_INVALID_SELECTION;

	// Left-adjusted data is not supported.
	else if (configPtr->leftAdjust != false)
		result = ADC_RESULT_INVALID_SELECTION;

	// Only gain 1 is supported.
	else if (configPtr->gain != ADC_GAIN_1)
		result = ADC_RESULT_INVALID_SELECTION;

	// The only "reference" supported is 1.5V, since that's the tolerance of
	// the ADC pins.
	else if (configPtr->reference != ADC_REF_1_5V)
		result = ADC_RESULT_INVALID_SELECTION;

	// Continuous mode only.
	else if (configPtr->mode != ADC_MODE_CONTINUOUS)
		result = ADC_RESULT_INVALID_SELECTION;

	// Must be 12-bit resolution.
	else if (configPtr->resolution != ADC_RES_12)
		result = ADC_RESULT_INVALID_SELECTION;

	//*************************************************************************
	// Don't do anything else if the result is not "OK".
	//*************************************************************************
	if (result == ADC_RESULT_OK)
	{
		// Enable clock to ADC.
		MAP_PRCMPeripheralClkEnable(PRCM_ADC, PRCM_RUN_MODE_CLK);
	}
	
	return result;
}

/******************************************************************************
 *
 *	Description:	(Un)Register a callback function.
 *
 *	Parameters:		channel - ADC channel to act upon
 *					type - type of callback to (un)register
 *					Callback - function to call when the specified event; use
 *					NULL to disable the callback
 *
 *	Return value:	ADC_RESULT_OK on success; other on failure
 *
 *****************************************************************************/

adcResult_t AdcSetCallback(adcChannel_t channel, adcCbType_t type,
	adcCallback_t Callback)
{
	adcResult_t result = ADC_RESULT_OK;	// an optimistic return value :)

	// Check for invalid channel.
	if (channel >= ADC_NUM_CHANNELS)
		result = ADC_RESULT_INVALID_SELECTION;
	
	else
	{
		// Register user callback.
		// If the callback is NULL, that's how we unregister.
		CallbackArray[type] = Callback;
	}
	
	return result;
}

/******************************************************************************
 *
 *	Description:	Start reading from the ADC and store in the provided buffer.
 *
 *	Parameters:		channel - ADC channel to act upon
 *					sampleArray - pointer to buffer to store the ADC readings
 *					numSamples - number of ADC samples to take
 *
 *	Return value:	ADC_RESULT_OK on success; other on failure
 *
 *****************************************************************************/

adcResult_t AdcReadSamples(adcChannel_t channel, adcSample_t *sampleArray, uint32_t numSamples)
{
	adcResult_t result = ADC_RESULT_OK;	// an optimistic return value :)
	
	// Remember the channel setting for later.
	adcActiveChannel = channel;
	switch(channel)
	{
		case ADC0:
			adcActiveChannelCode = ADC_CH_0;
			break;

		case ADC1:
			adcActiveChannelCode = ADC_CH_1;
			break;

		case ADC2:
			adcActiveChannelCode = ADC_CH_2;
			break;

		case ADC3:
			adcActiveChannelCode = ADC_CH_3;
			break;
		
		default:
			result = ADC_RESULT_INVALID_SELECTION;
		break;
	}

	//*************************************************************************
	// Don't do anything else if the result is not "OK".
	//*************************************************************************
	if (result == ADC_RESULT_OK)
	{
			// Set up global variables.
			adcNumSamplesDesired = numSamples;
			adcNumSamplesAcquired = 0;
			
			// Store pointer so that samples are stored to the right location.
			adcSamplesArray = sampleArray;

			// Enable the desired channel.
			MAP_ADCChannelEnable(ADC_BASE, adcActiveChannelCode);

			// Set the wrap-around value of the ADC's timer, a 17 bit
			// internal timer used to timestamp the ADC data samples internally.
			// Users can read the timestamp along with the sample from the FIFO
			// register(s).  Each sample in the FIFO contains 14 bit actual data
			// and 18 bit timestamp.  2^17 is the largest possible value.
			MAP_ADCTimerConfig(ADC_BASE, 2^17);

			// Enable the ADC's internal timer.
			MAP_ADCTimerEnable(ADC_BASE);

			// Enable the ADC.
			MAP_ADCEnable(ADC_BASE);

			// Start conversions.
			MAP_ADCIntEnable(ADC_BASE, adcActiveChannelCode, ADC_FIFO_UNDERFLOW | ADC_FIFO_FULL);
			MAP_ADCIntRegister(ADC_BASE, adcActiveChannelCode, AdcISR);
	}
		
	return result;
}

/******************************************************************************
 *
 *	Description:	Retrieves the last conversion value from the ADC
 *
 *	Parameters:		channel - ADC channel to act upon
 *					sample - ADC reading
 *
 *	Return value:	ADC_RESULT_OK on success; other on failure
 *
 *****************************************************************************/

adcResult_t AdcReadSample(adcChannel_t channel, adcSample_t *sample)
{
	return AdcReadSamples(channel, sample, 1);
}

/******************************************************************************
 *	ADC Test Functions
 *****************************************************************************/
#ifdef INCLUDE_TEST

#define TEST_ADC_NUM_SAMPLES		16

static volatile uint8_t testCompleteFlag;

void AdcTestCallback(adcChannel_t channel)
{
	testCompleteFlag = true;
}

/******************************************************************************
 *
 *	Description:	Test the ADC driver.
 *
 *	Return Value:	ADCResultOK on success, other on failure.
 *
 *****************************************************************************/

adcResult_t AdcTest(adcChannel_t channel)
{
	adcResult_t result = ADC_RESULT_OK;	// an optimistic return value :)
	adcConfig_t config;
	adcSample_t sampleArray[TEST_ADC_NUM_SAMPLES];
	
	config.channel = channel;
	config.gain = ADC_GAIN_1;
	config.reference = ADC_REF_1_5V;
	config.resolution = ADC_RES_12;
	config.mode = ADC_MODE_CONTINUOUS;
	config.differential = false;
	config.leftAdjust = false;
	
	do
	{
		// Initialize the ADC.
		result = AdcInit(&config);
		if (result != ADC_RESULT_OK)
			break;
			
		// Set a read complete callback.
		result = AdcSetCallback(ADC1, ADC_CB_DONE, AdcTestCallback);
		if (result != ADC_RESULT_OK)
			break;
		
		// Read samples from the ADC.
		testCompleteFlag = false;
		result = AdcReadSamples(ADC1, sampleArray, TEST_ADC_NUM_SAMPLES);
		if (result != ADC_RESULT_OK)
			break;
			
		// Wait for the ADC to complete.
		while (testCompleteFlag == false){}
			
		// Unregister the read complete callback.
		result = AdcSetCallback(ADC1, ADC_CB_DONE, NULL);
		if (result != ADC_RESULT_OK)
			break;

	} while (false);
	
	return result;
}

#endif // INCLUDE_ADC_TEST_FUNCTION
