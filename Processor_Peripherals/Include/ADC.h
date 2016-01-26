/******************************************************************************
 *
 *	Filename:		ADC.h
 *
 *	Author:			Adam Johnson
 *
 *	Description:	Driver for a processor's internal ADC.
 *
 *****************************************************************************/

#ifndef ADC_H
#define ADC_H

typedef uint16_t adcSample_t;			// ADC result type

typedef enum							// ADC gain settings
{
	ADC_GAIN_1
} adcGain_t;

typedef enum							// ADC resolution settings
{
	ADC_RES_8,							// 8-bit
	ADC_RES_12,							// 12-bit
	ADC_RES_16,							// 16-bit
	ADC_RES_24							// 24-bit
} adcRes_t;

typedef enum							// ADC mode settings
{
	ADC_MODE_CONTINUOUS,				// Convert continuously.
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
	adcGain_t gain;						// The amount of gain to apply
	adcRef_t reference;					// The analog reference source
	adcRes_t resolution;				// The resolution of the returned samples
	adcMode_t mode;						// The operation mode of ADC conversions
	bool differential;					// TRUE = differential input; FALSE = single ended input
	bool leftAdjust;					// TRUE = left justified results
} adcConfig_t;

typedef void (*adcCallback_t)(adcChannel_t channel);	// prototype for callback functions

adcResult_t AdcInit(uint8_t channel, adcConfig_t *configPtr);
adcResult_t AdcSetCallback(uint8_t channel, adcCbType_t type, adcCallback_t Callback);
adcResult_t AdcReadSamples(uint8_t channel, adcSample_t *sampleArray, uint32_t numSamples);
adcResult_t AdcReadSample(uint8_t channel, adcSample_t *sample);

#endif /* ADC_H */
