/******************************************************************************
 *
 *	Filename:		TestCalculate.c
 *
 *	Author:			Adam Johnson
 *
 *	Description:	This file contains tests for a code module.  Having this
 *					ensures that as the module is ported or upgraded, is still
 *					has the same usage and performs correctly.
 *
 *****************************************************************************/

#ifdef INCLUDE_TEST

#include "Calculate.h"
#include "TestCalculate.h"

/******************************************************************************
 *
 *	Function:		TestCalcTemperature
 *
 *	Description:	Tests the CalcConvertTemp function by converting a value
 *					multiple ways and checking the answer.
 *
 *	Return Value:	0 for success; other values indicate failure
 *
 *****************************************************************************/
 
uint8_t TestCalcTemperature(void)
{
	float oldVal = 5.0;					// value to convert
	float newVal;						// result of unit conversion
	uint8_t error = 1;					// a pessimistic return value :(
	
	do	// I'm afraid of goto statements, so I'll use this instead ;}
	{
		// Convert from K to K.
		newVal = CalcConvertTemp(oldVal, KELVIN, KELVIN);
		if FLOAT_IS_UNEQUAL(newVal, 5.0)
			break;

		// Convert from K to °C.
		newVal = CalcConvertTemp(oldVal, KELVIN, CELSIUS);
		if FLOAT_IS_UNEQUAL(newVal, -268.15)
			break;

		// Convert from K to °F.
		newVal = CalcConvertTemp(oldVal, KELVIN, FAHRENHEIT);
		if FLOAT_IS_UNEQUAL(newVal, -450.67)
			break;

		// Convert from K to °R.
		newVal = CalcConvertTemp(oldVal, KELVIN, RANKINE);
		if FLOAT_IS_UNEQUAL(newVal, 9)
			break;

		// Convert from °C to K.
		newVal = CalcConvertTemp(oldVal, CELSIUS, KELVIN);
		if FLOAT_IS_UNEQUAL(newVal, 278.15)
			break;

		// Convert from °C to °C.
		newVal = CalcConvertTemp(oldVal, CELSIUS, CELSIUS);
		if FLOAT_IS_UNEQUAL(newVal, 5.0)
			break;

		// Convert from °C to °F.
		newVal = CalcConvertTemp(oldVal, CELSIUS, FAHRENHEIT);
		if FLOAT_IS_UNEQUAL(newVal, 41)
			break;

		// Convert from °C to °R.
		newVal = CalcConvertTemp(oldVal, CELSIUS, RANKINE);
		if FLOAT_IS_UNEQUAL(newVal, 500.67)
			break;

		// Convert from °F to K.
		newVal = CalcConvertTemp(oldVal, FAHRENHEIT, KELVIN);
		if FLOAT_IS_UNEQUAL(newVal, 258.15)
			break;

		// Convert from °F to °C.
		newVal = CalcConvertTemp(oldVal, FAHRENHEIT, CELSIUS);
		if FLOAT_IS_UNEQUAL(newVal, -15)
			break;

		// Convert from °F to °F.
		newVal = CalcConvertTemp(oldVal, FAHRENHEIT, FAHRENHEIT);
		if FLOAT_IS_UNEQUAL(newVal, 5.0)
			break;

		// Convert from °F to °R.
		newVal = CalcConvertTemp(oldVal, FAHRENHEIT, RANKINE);
		if FLOAT_IS_UNEQUAL(newVal, 464.67)
			break;

		// Convert from °R to K.
		newVal = CalcConvertTemp(oldVal, RANKINE, KELVIN);
		if FLOAT_IS_UNEQUAL(newVal, 2.7777778)
			break;

		// Convert from °R to °C.
		newVal = CalcConvertTemp(oldVal, RANKINE, CELSIUS);
		if FLOAT_IS_UNEQUAL(newVal, -270.37222)
			break;

		// Convert from °R to °F.
		newVal = CalcConvertTemp(oldVal, RANKINE, FAHRENHEIT);
		if FLOAT_IS_UNEQUAL(newVal, -454.67)
			break;

		// Convert from °R to °R.
		newVal = CalcConvertTemp(oldVal, RANKINE, RANKINE);
		if FLOAT_IS_UNEQUAL(newVal, 5)
			break;
		
		// Only pass if we reach this line.
		error = 0;
	} while (0);
	
	return error;
}

/******************************************************************************
 *
 *	Function:		TestCalcVelocity
 *
 *	Description:	Tests the CalcVelocity function.
 *
 *	Return Value:	0 for success; other values indicate failure
 *
 *****************************************************************************/

uint8_t TestCalcVelocity(void)
{
	
}

#endif