/******************************************************************************
 *
 *	Filename:		Calculate.h
 *
 *	Author:			Adam Johnson
 *
 *	Description:	This is a math library, containing functions to calculate
 *					polynomials, velocity, and volumetric flow.  Also contains
 *					scaling, averaging, area, and unit conversion functions,
 *					and a few macros to make comparing floating-point numbers
 *					easier.
 *
 *****************************************************************************/

#ifndef CALCULATE__H
#define	CALCULATE__H

#include <float.h>				// provides definition of FLT_EPSILON

// Configuration settings
#define FLOAT	float			// change this to use "float" or "double"

// Math Constants
#define PI		3.14159			// math constant used in area formulas
#define R_AIR	0.2870			// specific gas constant for air [kJ/(kg*K)]
#define STD_P	100				// standard barometric air pressure [kPa]
#define STD_T	25				// standard ambient temperature [°C]

// Macros to compare floating-point numbers
#define FLOAT_IS_EQUAL(a, b)			((fabs(a - b)) < FLT_EPSILON)
#define FLOAT_IS_UNEQUAL(a, b)			(!FLOAT_IS_EQUAL)
#define FLOAT_IS_GREATER(a, b)			((a > b) && (!FLT_IS_EQUAL(a,b)))
#define FLOAT_IS_GREATER_OR_EQUAL(a, b)	((a > b) || FLT_IS_EQUAL(a,b))
#define FLOAT_IS_LESS(a, b)				((a < b) && (!FLT_IS_EQUAL(a,b)))

typedef enum					// Shapes (for finding cross-sectional area)
{
	CIRCLE,
	RECTANGLE,
	OVAL
} shape_t;

typedef enum					// Units of length
{
	M,							// meter - default internal unit
	CM,							// centimeter
	FT,							// foot
	INCH						// inch
} unitL_t;

typedef enum					// Units of pressure
{
	KPA,						// kiloPascal - default internal unit
	HPA,						// hectoPascal
	MBAR,						// millibar
	PA,							// Pascal
	INWC,						// inch of H2O
	FTWC,						// foot of H2O
	INHG,						// inch of Hg
	PSI,						// PSI
	OZIN,						// ounce force / inch^2
	MMWC,						// mm of H2O
	CMWC,						// cm of H2O
	MMHG						// mm of Hg
} unitP_t;

typedef enum					// Units of velocity
{
	M_S,						// meter / second - default internal unit
	M_H,						// meter / hour
	K_H,						// kilometer / hour
	KN,							// knot
	MPH,						// mile / hour
	FPS,						// foot / second
	FPM							// foot / minute
} unitV_t;

typedef enum					// Units of volumetric flow
{
	M3S,						// cubic meter / second - default internal unit
	M3H,						// cubic meter / hour
	LPS,						// liter / second
	LPM,						// liter / minute
	LPH,						// liter / hour
	CFM,						// cubic foot / minute
	GPM,						// gallon / minute
	GPH,						// gallon / hour
	GPD							// gallon / day
} unitF_t;

typedef enum					// Units of temperature
{
	CELSIUS,					// Celsius - default internal unit
	KELVIN,						// Kelvin
	RANKINE,					// Rankine (did you know this existed?)
	FAHRENHEIT					// Fahrenheit
} unitT_t;

// General Math Functions
uint32_t make32(uint8_t var1, uint8_t var2, uint8_t var3, uint8_t var4);		// Concatenate bytes.
uint16_t CalcSwapBytes(uint16_t data);											// Swap bytes.
uint32_t CalcSwapWords(uint32_t data);											// Swap words.
uint32_t CalcIntRoot(uint32_t n);												// Square root an integer.
FLOAT CalcLerp(FLOAT v0, FLOAT v1, FLOAT t);									// Interpolate linearly.
FLOAT CalcScale (FLOAT X, FLOAT X1, FLOAT X2, FLOAT Y1, FLOAT Y2);				// Calculate proportions.
FLOAT CalcArea(shape_t shape, FLOAT xDim, FLOAT yDim);							// Get area of a shape [units depend on parameters].
FLOAT CalcMMAverage(FLOAT *avgPrev, FLOAT newSample, uint16_t numSamples);		// Do a running average.
FLOAT CalcExpAverage(FLOAT *avgPrev, FLOAT newSample, FLOAT period, FLOAT tau);	// Do an exponential average.
FLOAT CalcPolynomial(FLOAT x, FLOAT c[], uint32_t n);							// Do a polynomial.
uint16_t CalcGCD(uint16_t a, uint16_t b);										// Calculate greatest common divisor of unsigned numbers.

// Unit Conversion Functions
FLOAT CalcConvertLength(FLOAT oldVal, unitL_t oldUnit, unitL_t newUnit);		// convert length
FLOAT CalcConvertPressure(FLOAT oldVal, unitP_t oldUnit, unitP_t newUnit);		// convert pressure
FLOAT CalcConvertVelocity(FLOAT oldVal, unitV_t oldUnit, unitV_t newUnit);		// convert velocity
FLOAT CalcConvertFlow(FLOAT oldVal, unitF_t oldUnit, unitF_t newUnit);			// convert flow
FLOAT CalcConvertTemp(FLOAT oldVal, unitT_t oldUnit, unitT_t newUnit);			// convert temperature

// Metrology Functions
FLOAT CalcVelocity(FLOAT pressure, FLOAT temperature, FLOAT kFact);				// get velocity [m/s]
FLOAT CalcFlow(FLOAT velocity, FLOAT area);										// get volumetric flow [m^3/h]
FLOAT CalcVaporPressure(FLOAT temperature);										// get vapor pressure [mbar]
FLOAT CalcDewPoint(FLOAT temperature, FLOAT humidity);							// get dew point [°C]
FLOAT CalcWetBulb(FLOAT dryBulbTemp, FLOAT humidity, FLOAT baroPress);			// get wet-bulb temperature [°C]

#endif	/* CALCULATE__H */

