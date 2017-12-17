/******************************************************************************
 *
 *	Filename:		ADCDriver.h
 *
 *	Author:			Adam Johnson
 *
 *	Description:	Driver for a processor's internal ADC.
 *
 *	Terms of Use:	MIT License
 *
 *****************************************************************************/

#ifndef ADCDRIVER_H
#define ADCDRIVER_H

#include <stdint.h>						// universal data types

#ifndef ADC_COUNT_IS_DEFINED			// ensure ADC's sample size is defined
#error "adcCount_t should be typedefed in your project."
#warning "like this:  typedef uint16_t adcCount_t"
#warning "Define ADC_COUNT_IS_DEFINED too."
#endif

typedef enum							// error codes
{
	ADC_RESULT_OK = 0,					// All is well!
	ADC_RESULT_FAIL,					// It's the target's fault.
	ADC_RESULT_NOT_IMPLEMENTED,			// It's my fault.
	ADC_RESULT_INVALID_SELECTION		// It's your fault.
} adcResult_t;

typedef enum							// ADC mode settings
{
	ADC_MODE_SINGLE,					// Convert only once.
	ADC_MODE_CONTINUOUS,				// Convert continuously.
} adcMode_t;

typedef enum							// ADC reference settings
{
	ADC_REF_1_5V
} adcRef_t;

typedef enum							// types of callback functions
{
	ADC_CB_DONE,						// conversion complete
	ADC_CB_ERROR						// error
} adcCbType_t;

typedef struct							// configuration structure
{
	uint8_t gain;						// The amount of gain to apply
	uint8_t resolution;					// The resolution of the returned samples
	adcMode_t mode;						// The operation mode of ADC conversions
	adcRef_t reference;					// The analog reference source
	bool differential;					// TRUE = differential input; FALSE = single ended input
	bool leftAdjust;					// TRUE = left justified results
} adcConfig_t;

typedef void (*adcCallback_t)(adcChannel_t channel);	// prototype for callback functions

adcResult_t AdcInit(uint8_t channel, adcConfig_t *configPtr);
adcResult_t AdcSetCallback(uint8_t channel, adcCbType_t type, adcCallback_t Callback);
adcResult_t AdcReadCounts(uint8_t channel, adcCount_t *sample);
adcResult_t AdcReadVoltage(uint8_t channel, float *sample);

#endif /* ADCDRIVER_H */
