/******************************************************************************
 *
 *	Filename:		Calculate.c
 *
 *	Author:			Adam Johnson
 *
 *	License:		This code is released under the MIT License – Please use,
 *					change and share it.
 *
 *	Description:	This is a math library, containing functions to calculate
 *					polynomials, velocity, and volumetric flow.  Also contains
 *					scaling, averaging, area, and unit conversion functions.
 *
 *	TODO:			The wet-bulb formula doesn't seem to give correct answers.
 *
 *****************************************************************************/

#include <stdint.h>						// universal data types
#include <math.h>						// contains exp(), pow(), log10()
#include "Calculate.h"					// header for this module

// Constants to convert temperatures
#define OFFSET_F_TO_R		459.67		// offset between °F and °R
#define OFFSET_C_TO_K		273.15		// offset between °C and K
#define OFFSET_C_TO_F		32.0		// offset between °C and °F

// Constants to calculate psychrometrics
#define MAGNUS_ALPHA		6.112		// for Magnus formula
#define MAGNUS_BETA			17.62		// for Magnus formula
#define MAGNUS_BETA_10		7.65		// for Magnus formula (base-10 version ONLY!)
#define MAGNUS_BETA_ICE		22.46		// for Magnus formula (for really cold temps)
#define MAGNUS_LAMBDA		243.12		// for Magnus formula
#define MAGNUS_LAMBDA_ICE	272.62		// for Magnus formula (for really cold temps)
#define B					0.00066		// psychrometer coeficient (for CalcWetBulb)

// This must match the order of unitL_t!
static const FLOAT lConversion[] =
{// Conversion from m      Unit
	1.0,				// meter - default internal unit
	100.0,				// centimeter
	3.2808399,			// foot
	39.370079			// inch
};

// This must match the order of unitP_t!
static const FLOAT pConversion[] =
{// Conversion from kPa    Unit
	1.0,				// kiloPascal - default internal unit
	10.0,				// hectoPascal
	10.0,				// millibar
	1000.0,				// Pascal
	4.0146308,			// inch of H2O
	0.33455256,			// foot of H2O
	0.29529987,			// inch of Hg
	0.14503774,			// PSI
	2.3206038,			// ounce force / inch^2
	101.971621,			// mm of H2O
	10.1971621,			// cm of H2O
	7.5006168			// mm of Hg
};

// This must match the order of unitV_t!
static const FLOAT vConversion[] =
{// Conversion from m/s    Unit
	1.0,				// meter / second - default internal unit
	3600.0,				// meter / hour
	3.6,				// kilometer / hour
	1.9438445,			// knot
	2.2369363,			// mile / hour
	3.2808399,			// foot / second
	196.85039			// foot / minute
};

// This must match the order of unitF_t!
static const FLOAT fConversion[] =
{// Conversion from m^3/s  Unit
	1.0,				// cubic meter / second - default internal unit
	3600.0,				// cubic meter / hour
	1000.0,				// liter / second
	1000.0 / 60.0,		// liter / minute
	1000.0 / 3600.0,	// liter / hour
	2118.88,			// cubic foot / minute
	15850.323,			// gallon / minute
	15850.323 / 60.0,	// gallon / hour
	15850.323 / 1440.0	// gallon / day
};

// This must match the order of unitT_t!
static const FLOAT tConversion[] =
{//	Conversion from K      Unit
	1.0,				// Celsius - default internal unit
	1.0,				// Kelvin
	9.0 / 5.0,			// Rankine
	9.0 / 5.0			// Fahrenheit
};

/******************************************************************************
 *
 *	Function:		make32
 *
 *	Description:	
 *
 *	Inspiration:	http://www.microchip.com/forums/m640322.aspx
 *
 *	Parameters:		four bytes
 *
 *	Return Value:	value formed by concatenating the bytes
 *
 *****************************************************************************/
 
uint32_t make32(uint8_t var1, uint8_t var2, uint8_t var3, uint8_t var4)
{
	return (((((uint32_t)var1 << 8 | var2) << 8) | var3) << 8) | var4;
}

/******************************************************************************
 *
 *	Function:		CalcSwapBytes
 *
 *	Description:	Swaps bytes of a 16-bit variable.
 *
 *	Parameters:		data - value to act upon.
 *
 *	Return Value:	swapped value
 *
 *****************************************************************************/
 
uint16_t CalcSwapBytes(uint16_t data)
{
	return ((data & 0xFF) << 8) | ((data >> 8) & 0xFF);
}

/******************************************************************************
 *
 *	Function:		CalcSwapWords
 *
 *	Description:	Swaps words of a 32-bit variable.
 *
 *	Parameters:		data - value to act upon
 *
 *	Return Value:	swapped value
 *
 *****************************************************************************/
 
uint32_t CalcSwapWords(uint32_t data)
{
	return ((data & 0xFFFF) << 16) | ((data >> 16) & 0xFFFF);
}

/******************************************************************************
*
*	Function:		CalcIntRoot
*
*	Description:	Calculates integer truncated square root of another integer.
*
*	Inspiration:	http://jodarom.sdf1.org/code/arith/isqrt_joda.c
*	License:		Originally MIT by John L. Dahlstrom
*
*	Parameters:		n - a 32-bit unsigned integer
*
*	Return value:	integer truncated square root of n
*
******************************************************************************/

uint32_t CalcIntRoot(uint32_t n)
{
	uint32_t rs = 0;
	uint32_t dP2 = 0x40000000;
	uint32_t gs;
	uint8_t  i;

	// Iteration specific constants may replace dP2;
	// An optimizing compiler may do this.

	for (i = 0; i < 16; i++)
	{
		gs = rs | dP2;
		
		rs >>= 1;
		
		if (n >= gs)
		{
			n -= gs;
			
			rs |= dP2;
		}
		
		dP2 >>= 2;
	}
	
	return rs;
}

/******************************************************************************
 *
 *	Function:		CalcLerp
 *
 *	Description:	Performs linear interpolation.
 *
 *	Parameters:		v0 - first value
 *					v1 - second value
 *					t - fraction between v0 and v1; must be between 0 and 1
 *
 *	Return Value:	interpolated value
 *
 *****************************************************************************/
 
FLOAT CalcLerp(FLOAT v0, FLOAT v1, FLOAT t)
{
  return v0 + (v1 - v0) * t;

  // Alternate formula:
  //return v0 * (1 - t) + v1 * t;
}

/******************************************************************************
 *
 *	Function:		CalcScale
 *
 *	Description:	Converts a scaled value from one scale to another.
 *					X1, X2 is the range of the original scale.
 *					Y1, Y2 is the range of the new scale.
 *
 *	Parameters:		FLOAT X - the value of interest.
 *					FLOAT X1 - the low end of the old scale.
 *					FLOAT X2 - the high end of the old scale.
 *					FLOAT Y1 - the low end of the new scale.
 *					FLOAT Y2 - the high end of the new scale.
 *
 *	Return Value:	the result of the conversion (0.0 if error)
 *
 *****************************************************************************/

FLOAT CalcScale (FLOAT X, FLOAT X1, FLOAT X2, FLOAT Y1, FLOAT Y2)
{
	FLOAT result;						// output of the calculation
	FLOAT denominator = X2 - X1;		// size of old scale
	
	// Don't divide by zero!
	if (denominator == 0.0f)
	{
		// If we're dividing by zero, just return 0.0.
		result = 0.0f;
	}
	else
	{
		result = ((Y2 * (X - X1)) + (Y1 * (X2 - X))) / denominator;
	}
	
	return result;
}

/******************************************************************************
 *
 *	Function:		CalcPolynomial
 *
 *	Description:	Calculates the output of a polynomial of the form
 *					y = C_n * (x^n) + C_n-1 * (x^(n-1)) ... + C0
 *
 *	Parameters:		FLOAT x - the independent variable
 *					FLOAT *c - array of coefficients
 *					uint32_t n - the equation's order (number coefficients - 1)
 *
 *	Return Value:	FLOAT - the dependent variable
 *
 *****************************************************************************/

FLOAT CalcPolynomial(FLOAT x, FLOAT c[], uint32_t n)
{
	FLOAT y;							// output of the equation

	y = c[n];							// Start with the first coefficient.

	while (n > 0)						// If there's another coefficient...
	{
		y *= x;							// Multiply by x.
		y += c[--n];					// Add next coefficient.
	}

	return y;							// Return the result.
}

/******************************************************************************
 *	Function:		CalcGCD
 *
 *	Description:	Calculates greatest common divisor of unsigned numbers
 *
 *	Parameters:		uint16_t a - first number
 *					uint16_t b - second number
 *
 *	Return Value:	greatest common divisor of a and b
 *
 *****************************************************************************/
 
uint16_t CalcGCD(uint16_t a, uint16_t b)
{
	uint16_t c;
	
	while (b != 0)
	{
		c = a; 
		a = b; 
		b = c % b;
	}
	
	return a;
}

/******************************************************************************
 *
 *	Function:		CalcMMAverage
 *
 *	Description:	Calculates a moving average (without a buffer).  See
 *					Wikipedia entry for "Modified Moving Average."  We don't
 *					check for division by zero, so make sure numSamples is
 *					greater than zero or this function might violate physics.
 *
 *	Parameters:		FLOAT *avgPrev - the previous average; use newSample the
 *						first time the function is run
 *					FLOAT newSample - a new value to be added to the average
 *					uint16_t numSamples - the number of samples to be averaged
 *
 *	Return Value:	FLOAT - the new average
 *
 *****************************************************************************/

FLOAT CalcMMAverage(FLOAT *avgPrev, FLOAT newSample, uint16_t numSamples)
{
	*avgPrev = (*avgPrev * (numSamples - 1) + newSample) / numSamples;

	return *avgPrev;	// Return the new average.
}

/******************************************************************************
 *
 *	Function:		CalcExpAverage
 *
 *	Description:	Calculates an exponentially weighted average according to
 *					the formula y(t+1) = y(t) + k(x(t) - y(t)), where
 *					y(t) is current averaged output,
 *					y(t+1) is the next output,
 *					k is the damping factor according to
 *						k = 1 - e^(-1/(r*t), where
 *							r = sample rate [samples/second]
 *							t = filter time constant [seconds]
 *						or k = 1 - e^(-p/t), where
 *							p = sample period [seconds/sample], and
 *							t = filter time constant [seconds].
 *
 *					The time constant (t) represents the time it takes this
 *					function's step response to reach 1 - 1/e (or 63.2) percent
 *					of it's final value.  5 time constants gives about 99% of
 *					the final value.  See Wikipedia entry for "Time constant."
 *
 *					This is often used to "smooth" noisy sensor readings over a
 *					certain time period - which means we'll almost always want
 *					to use 5 time-constants.  For example, let's say we have a
 *					pressure sensor which is reading x PSI and suddenly it reads
 *					y PSI, and you have a display, but you want it to slowly go
 *					from x to y in 10 seconds instead of instantly.  Divide 10
 *					seconds by 5 to get a time constant of 2 seconds, and so use
 *					2 for tau.
 *
 *					We don't check for division by zero, so make sure tau is
 *					greater than zero, or this funcition may violate physics.
 *
 *	Parameters:		FLOAT *avgPrev - the previous average; use newSample the
 *						first time the function is run
 *					FLOAT newSample - a new value to be added to the average
 *					FLOAT period - the time period we're sampling over
 *					FLOAT tau - time constant for filter (can't be zero!)
 *
 *	Return Value:	FLOAT - the new average
 *
 *****************************************************************************/

FLOAT CalcExpAverage(FLOAT *avgPrev, FLOAT newSample, FLOAT period, FLOAT tau)
{
	FLOAT k;			// damping factor

	k = 1.0 - exp((-1.0 * period) / tau);

	*avgPrev = *avgPrev + k * (newSample - *avgPrev);

	return *avgPrev;	// Return the new average.
}

/******************************************************************************
 *
 *	Function:		CalcVelocity
 *
 *	Description:	Calculate velocity from pressure, temperature, and
 *					k-factor.  Note that temperature is used here...cause it's
 *					possible to compensate the pressure sensor for
 *					temperature effects yet ignore temperature effects on the
 *					density of air when finding velocity.  I choose not to
 *					ignore such things.  This function does, however, ignore
 *					effects of barometric pressure, since I've no way to
 *					measure it, and assumes a barometric pressure of 100 kPa.
 *					We start with this equation:
 *
 *					V = k * sqrt[2(Rs)(P)(T)/Pb], where
 *
 *					V = velocity [m/s]
 *					Rs = specific gas constant for air [0.2870 J/(kg*K)]
 *					P = pressure [kPa]
 *					T = temperature [K]
 *					Pb = barometric pressure [100 kPa]
 *					k = true velocity / measured velocity [unitless]
 *
 *					Note that the unit of P and Pb must match.
 *
 *					The "k-factor" is necessary because of duct parameters
 *					like throw pattern, shape, size, construction materials,
 *					and proximity of dampers or elbows to the measurement site.
 *					All these variables can make the measured velocity at one
 *					point in the duct different than the velocity in the rest
 *					of the system.  The k-factor, supplied by the duct
 *					manufacturer, is a unitless ratio of velocity in the system
 *					to the velocity measured at a point the user can access.
 *
 *	TODO:			Support any unit of measure for temperature and pressure.
 *
 *	Parameters:		FLOAT pressure - differential pitot tube pressure [kPa]
 *					FLOAT temperature - ambient temperature [K]
 *					FLOAT kFact - k-factor (hopefully of the duct under test)
 *
 *	Return Value:	FLOAT - air velocity, compensated for temperature
 *
 *****************************************************************************/

FLOAT CalcVelocity(FLOAT pressure, FLOAT temperature, FLOAT kFact)
{
	FLOAT velocitySquared;	// operand to square root function
	int8_t sign;			// stores sign of pressure so sqrt doesn't explode

	// Calculate the square of velocity from pressure.
	velocitySquared = 2 * R_AIR * pressure * temperature / STD_P;

	// Make sure argument of sqrt() is not negative, but preserve the sign.
	if (velocitySquared < 0)
	{
		velocitySquared *= -1;
		sign = -1;
	}
	else
	{
		sign = 1;
	}

	// Take the square root and multiply by kFact to complete the equation.
	return sqrt(velocitySquared) * kFact * sign;
}

/******************************************************************************
 *
 *	Function:		CalcFlow
 *
 *	Description:	Calculates volumetric flow from velocity and area.
 *
 *	Parameters:		FLOAT velocity - air velocity
 *					FLOAT area - area of duct
 * 
 *	Return Value:	FLOAT - area of the shape
 *
 *****************************************************************************/

FLOAT CalcFlow(FLOAT velocity, FLOAT area)
{
	return velocity * area;
}

/******************************************************************************
 *
 *	Function:		CalcVaporPressure
 *
 *	Description:	From dew point temperature, get saturated vapor pressure.
 *					From ambient temperature, get actual vapor pressure.
 *
 *					This function uses a form of the Magnus formula for vapor
 *					pressure.  Many versions of this function exist, in both
 *					base 10 and base e, with various values of alpha, beta, and
 *					gamma.  One common form looks like this (to find it, search
 *					online for "Sensirion Introduction to Humidity"):
 *
 *					pressure = a * e^((ß)(t)/(? + t))
 *
 *					We're using this form, which is equivalent:
 *
 *					pressure = a * 10^((B)(t)/(? + t)), where B = ß / ln(10)
 *
 *	Parameters:		temperature - dew point or ambient temperature [°C]
 *
 *	Return value:	saturated or actual vapor pressure [millibars(mb)/hPa]
 *
 *****************************************************************************/
 
FLOAT CalcVaporPressure(FLOAT temperature)
{
	FLOAT result;								// temporary variable
	
	result = (MAGNUS_BETA_10 * temperature);	// temp = B * t

	result /= (MAGNUS_LAMBDA + temperature);	// temp = temp / (lambda + t)
	
	result =  MAGNUS_ALPHA * pow(10, temp);		// a * 10^(temp)
	
	return result;
}

/******************************************************************************
 *
 *	Function:		CalcDewPoint
 *
 *	Description:	Calculates dew-point (or frost-point) given temperature and
 *					relative humidity.  Humidity must not be less than 1%.
 *
 *					The formula used in this function is derived from the
 *					Magnus formula.  To find it, search online for "Sensirion
 *					dewpoint calculation".  The formula is:
 *
 *					     lambda(ln(RH/100%) + beta*t/(lambda + t))
 *					dp = ------------------------------------------, where:
 *					     beta - (ln(RH/100%) + beta*t/(lambda + t))
 *
 *					dp = dew-point (or frost-point) [°C]
 *					RH = relative humidity [%]
 *					t = temperature [°C]
 *					beta, lambda = constants
 *
 *	Parameters:		temperature - temperature [°C]
 *					humidity - relative humidity [1 - 100%]
 *
 *	Return Value:	dew-point or frost-point [°C]
 *
 *****************************************************************************/
 
FLOAT CalcDewPoint(FLOAT temperature, FLOAT humidity)
{
	FLOAT h;						// intermediate calculation
	FLOAT beta = MAGNUS_BETA;		// If the temperature is above the freezing
	FLOAT lambda = MAGNUS_LAMBDA;	// point of water, use values for "above
									// water" (which give the "dew point").
	
	// We can't take the logarithm of 0 or a negative number, so let's bound
	// humidity at 1%.
	if (humidity < 1.0)
		humidity = 1.0;
	
	// If the temperature is below the freezing point of water, use values for
	// "above ice" (which give the "frost point").
	if (temperature < 0.0)
	{
		beta = MAGNUS_BETA_ICE;
		lambda = MAGNUS_LAMBDA_ICE;
	}
	
	// Calculate intermediate value.
	h = log(humidity / 100) + beta * temperature / (lambda + temperature);
	
	// Calculate dew point (or frost point).
	return lambda * h / (beta - h);
}

/******************************************************************************
 *
 *	Function:		CalcWetBulb
 *
 *	Description:	Calculate the wet-bulb temperature given the dry bulb
 *					(ambient) temperature, relative humidity, and barometric
 *					pressure.
 *
 *					The formula used in this function is derived from the
 *					Magnus formula.  To find it, search online for "Sensirion
 *					intro to humidity".  The formula is:
 *
 *					wb = t - [(A/(B*p)) * e^((B*t)(L+t)) * (1 - RH/100%)], where:
 *
 *					wb = wet-bulb temperature [°C]
 *					t = dry-bulb (normal, ambient) temperature [°C]
 *					A, B, L = constants
 *					RH = relative humidity
 *
 *	Parameters:		dryBulbTemp - normal, ambient temperature [°C]
 *					humidity - humidity - relative humidity [1 - 100%]
 *					baroPress - barometric pressure [mbar]
 *
 *	Return Value:	wet-bulb temperature [°C]
 *
 *****************************************************************************/
 
FLOAT CalcWetBulb(FLOAT dryBulbTemp, FLOAT humidity, FLOAT baroPress)
{
	FLOAT magnus;					// temporary value
	FLOAT dryness;					// opposite of relative humidity
	FLOAT beta = MAGNUS_BETA;		// If temperature is above 0°C,
	FLOAT lamda = MAGNUS_LAMBDA;	// use coefficients "above water".

	// If the temperature is below 0°C, use coefficients for "above ice".
	if (dryBulbTemp < 0)
	{
		beta = MAGNUS_BETA_ICE;
		lamda = MAGNUS_LAMBDA_ICE;
	}
	
	// Compute an intermediate part of the wet-bulb equation.
	magnus = exp((beta * dryBulbTemp) / (lamda + dryBulbTemp));
	
	// Compute "relative dryness" of the air (opposite of relative humidity).
	dryness = 1.0 - (humidity / 100);
	
	// Compute wet-bulb temperature.
	return dryBulbTemp - ((MAGNUS_ALPHA / (B * baroPress)) * magnus * dryness);
}

/******************************************************************************
 *
 *	Function:		CalcArea
 *
 *	Description:	This function calculates area of a shape given the type of
 *					shape, its width, and length.  If the function doesn't
 *					recognize the type of shape, it assumes an ellipse.
 *
 *	Parameters:		shape shape - enumerated type of shape
 *					FLOAT xDim - x dimension of the shape
 *					FLOAT yDim - x dimension of the shape
 *
 *	Return Value:	FLOAT - area of the shape
 *
 *****************************************************************************/

FLOAT CalcArea(shape_t shape, FLOAT xDim, FLOAT yDim)
{
	FLOAT area;

	if (shape == RECTANGLE)		// RECTANGLE: A=(x)(y)
	{
		area = xDim * yDim;
	}
	else						// CIRCLE & ELLIPSE: A=PI(r1)(r2)
	{
		area = PI * (xDim / 2) * (yDim / 2);
	}

	return area;
}
 
/******************************************************************************
 *
 *	Function:		CalcConvertLength
 *
 *	Description:	Converts between various units of length.
 *
 *	Parameters:		FLOAT oldVal - the value to be converted
 *					unitL_t oldUnit - the original unit of measure
 *					unitL_t newUnit - the desired unit of measure
 *
 *	Return Value:	FLOAT - the converted value
 *
 *****************************************************************************/

FLOAT CalcConvertLength(FLOAT oldVal, unitL_t oldUnit, unitL_t newUnit)
{
	// Look up conversion from old unit to new unit.
	return (oldVal / lConversion[oldUnit] * lConversion[newUnit]);
}

/******************************************************************************
 *
 *	Function:		CalcConvertPressure
 *
 *	Description:	Converts between various units of pressure.
 *
 *	Parameters:		FLOAT oldVal - the value to be converted [kPa]
 *					unitP_t oldUnit - the original unit of measure
 *					unitP_t newUnit - the desired unit of measure
 *
 *	Return Value:	FLOAT - the converted value
 *
 *****************************************************************************/

FLOAT CalcConvertPressure(FLOAT oldVal, unitP_t oldUnit, unitP_t newUnit)
{
	// Look up conversion from old unit to new unit.
	return (oldVal / pConversion[oldUnit] * pConversion[newUnit]);
}

/******************************************************************************
 *
 *	Function:		CalcConvertVelocity
 *
 *	Description:	Converts between various units of velocity.
 *
 *	Parameters:		FLOAT oldVal - the value to be converted
 *					unitV_t oldUnit - the original unit of measure
 *					unitV_t newUnit - the desired unit of measure
 *
 *	Return Value:	FLOAT - the converted value
 *
 *****************************************************************************/

FLOAT CalcConvertVelocity(FLOAT oldVal, unitV_t oldUnit, unitV_t newUnit)
{
	// Look up conversion from old unit to new unit.
	return (oldVal / vConversion[oldUnit] * vConversion[newUnit]);
}

/******************************************************************************
 *
 *	Function:		CalcConvertFlow
 *
 *	Description:	Converts between various units of volumetric flow.
 *
 *	Parameters:		FLOAT oldVal - the value to be converted
 *					unitF_t oldUnit - the original unit of measure
 *					unitV_t newUnit - the new unit of measure
 *
 *	Return Value:	FLOAT - the converted value
 *
 *****************************************************************************/

FLOAT CalcConvertFlow(FLOAT oldVal, unitF_t oldUnit, unitF_t newUnit)
{
	// Look up conversion from old unit to new unit.
	return (oldVal / fConversion[oldUnit] * fConversion[newUnit]);
}

/******************************************************************************
 *
 *	Function:		CalcConvertTemp
 *
 *	Description:	Converts between various units of temperature.  If you want
 *					to convert a temperature INTERVAL, use only Kelvin and
 *					Rankine and you'll save lots of math.
 *
 *					This function was derived by setting up a 4x4 chart of all
 *					possible temperature conversion formulas.  Separate the
 *					formulas by order of operations.  Notice that all
 *					temperature conversions can be represented by an addition,
 *					then a multiplication, then an addition.  Add some if/then
 *					statements to do the correct additions and you're done.
 *
 *	Parameters:		FLOAT oldVal - the value to be converted
 *					unitT_t oldUnit - the original unit of measure
 *					unitT_t newUnit - the new unit of measure
 *
 *	Return Value:	the result of the conversion
 *
 *****************************************************************************/
 
FLOAT CalcConvertTemp(FLOAT oldVal, unitT_t oldUnit, unitT_t newUnit)
{
	FLOAT temp = oldVal;				// temporary temperature interval
	
	// FIRST ADDITION:  Convert Celsius or Fahrenheit to Kelvin or Rankine.
	if (oldUnit == CELSIUS)
	{
		if ((newUnit == KELVIN) || (newUnit == RANKINE))
			temp += OFFSET_C_TO_K;
	}

	else if (oldUnit == FAHRENHEIT)
	{
		if ((newUnit == KELVIN) || (newUnit == RANKINE))
			temp += OFFSET_F_TO_R;
		else if (newUnit == CELSIUS)
			temp -= OFFSET_C_TO_F;
	}
	
	// MULTIPLICATION:  Convert between Kelvin and Rankine.
	temp = (temp / tConversion[oldUnit] * tConversion[newUnit]);
	
	// SECOND ADDITION:  Convert Kelvin or Rankine to Celsius or Fahrenheit.
	if (newUnit == CELSIUS)
	{
		if ((oldUnit == KELVIN) || (oldUnit == RANKINE))
			temp -= OFFSET_C_TO_K;
	}

	else if (newUnit == FAHRENHEIT)
	{
		if ((oldUnit == KELVIN) || (oldUnit == RANKINE))
			temp -= OFFSET_F_TO_R;
		else if (oldUnit == CELSIUS)
			temp += OFFSET_C_TO_F;
	}
	
	return temp;
}

/******************************************************************************
 *
 *	Function:		CalcSteinhart
 *
 *	Description:	Converts a thermistor's voltage into a temperature using a
 *					Beta equation for thermistors.
 *
 *					Thermistors can be linearized with the Steinhart-Hart
 *					equation:
 *					1/T = a + b * ln(R) + c * ln(R)^3, where:
 *
 *					T = temperature [K],
 *					R = thermistor's resistance [ohm],
 *					a, b, c = calibration coefficients
 *
 *					This function uses the beta equation, which is the
 *					Steinhart-Hart equation with these coefficients:
 *					a = 1/To - (1/B)ln(Ro)
 *					b = 1/B
 *					c = 0
 *
 *					To = thermistor's reference temperature [K]
 *						 (typically 25 C or 298.15 K)
 *					Ro = thermistor's resistance at reference temperature [ohm]
 *
 *					which results in this equation:
 *					1/T = 1/To + (1/B)ln(R/Ro)
 *
 *					where B = beta, the calibration coefficient
 *
 *	Inspiration:	https://en.wikipedia.org/wiki/Thermistor
 *
 *	Parameters:		beta - B coefficient
 *					r0 - Ro, the thermistor's resisistance at known temperature
 *					t0 - To, thermistor's calibration temperature [K]
 *					voltage - thermistor's current voltage
 *
 *	Return Value:	thermistor's current temperature [K]
 *
 ******************************************************************************/

FLOAT CalcSteinhart(FLOAT beta, FLOAT r0, FLOAT t0, FLOAT voltage)
{
	FLOAT r;			// resistance [ohm]
	FLOAT steinhart;	// eventually this will be temperature [K]
	
	steinhart  = r / r0;				// (R/Ro)
	steinhart  = log(steinhart);		// ln(R/Ro)
	steinhart /= beta;					// 1/B * ln(R/Ro)
	steinhart += 1.0 / t0;				// + (1/To)
	steinhart  = 1.0 / steinhart;		// Invert.
}

FLOAT DividerFindR2(FLOAT R1, FLOAT Vref, int adcMaxCount, int adcCount)
{
	// R2 = R1 * Vout / (Vref - Vout)
	return (R1 * adcCount) / (adcMaxCount - adcCount);
}

FLOAT DividerFindR1(FLOAT R2, FLOAT Vref, FLOAT Vout)
{
	return (R2 * (Vref - Vout) / Vout);
}