/******************************************************************************
 *
 *	Filename:		dwyerHAL_ADC.h
 *
 *	Author:			Adam Johnson
 *
 *	Description:	ADC library for CC3200.  Tested on CC3200 LaunchXL board.
 *
 *****************************************************************************/


#ifndef CC3200_ADC_H
#define CC3200_ADC_H

typedef void (*adcCallback_t)(adcChannel_t);	// prototype for callback functions

typedef uint16_t adcSample_t;			// ADC result type

typedef enum							// enumeration for the ADC channels
{
	ADC0,
	ADC1,
	ADC2,
	ADC3,
	ADC_NUM_CHANNELS
} adcChannel_t;

typedef enum							// ADC gain settings
{
	ADC_GAIN_1
} adcGain_t;

typedef enum							// ADC resolution settings
{
	ADC_RES_12							// 12-bit
} adcRes_t;

typedef enum							// ADC mode settings
{
	ADC_MODE_SINGLE,					// Do a single conversion.
	ADC_MODE_CONTINUOUS,				// Convert continuously.
	ADC_MODE_BURST,
} adcMode_t;

typedef enum							// ADC reference settings
{
	ADC_REF_1_5V
} adcRef_t;

typedef enum							// error codes
{
	ADC_RESULT_OK = 0,					// All is well!
	ADC_RESULT_FAIL,					// It's the target's fault.
	ADC_RESULT_NOT_IMPLEMENTED,			// It's my fault.
	ADC_RESULT_INVALID_SELECTION		// It's your fault.
} adcResult_t;

typedef enum							// Callback assignments
{
	ADC_CB_DONE,						// conversion complete
	ADC_CB_ERROR						// error
} adcCbType_t;

typedef struct							// configuration structure
{
	adcChannel_t channel;				// The index of the peripheral to access
	adcGain_t gain;						// The amount of gain to apply
	adcRef_t reference;					// The analog reference source
	adcRes_t resolution;				// The resolution of the returned samples
	adcMode_t mode;						// The operation mode of ADC conversions
	bool differential;					// TRUE = differential input; FALSE = single ended input
	bool leftAdjust;					// TRUE = left justified results
} adcConfig_t;

adcResult_t AdcInit(adcConfig_t *configPtr);
adcResult_t AdcEnable(adcChannel_t channel);
adcResult_t AdcDisable(adcChannel_t channel);
adcResult_t AdcSetCallback(adcChannel_t channel, adcCbType_t type, adcCallback_t Callback);
adcResult_t AdcReadSamples(adcChannel_t channel, ADCSample_t *sampleArray, uint32_t numSamples);
adcResult_t AdcReadSample(adcChannel_t channel, ADCSample_t *sample);

#ifdef INCLUDE_TEST
adcResult_t AdcTest(void);
#endif

#endif /* CC3200_ADC_H */
