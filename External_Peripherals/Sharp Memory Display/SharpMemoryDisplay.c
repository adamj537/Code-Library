/******************************************************************************
 *
 *  Sharp Memory Display Connector
 *  -----------------------------------------------------------------------
 *  Pin   Function        Notes
 *  ===   ==============  ===============================
 *    1   VIN             3.3-5.0V (into LDO supply)
 *    2   3V3             3.3V out
 *    3   GND
 *    4   SCLK            Serial Clock
 *    5   MOSI            Serial Data Input
 *    6   CS              Serial Chip Select
 *    9   EXTMODE         COM Inversion Select (Low = SW clock/serial)
 *    7   EXTCOMIN        External COM Inversion Signal
 *    8   DISP            Display On(High)/Off(Low)
 *****************************************************************************/

#include "SharpDisplay.h"

uint8_t _ss, _clk, _mosi;
uint32_t _sharpmem_vcom;
byte *sharpmem_buffer;

#ifdef USE_FAST_PINIO
volatile RwReg *dataport, *clkport;
uint32_t datapinmask, clkpinmask;
#endif

// 1<<n is a costly operation on AVR -- table usu. smaller & faster
static const uint8_t PROGMEM
set[] = {  1,  2,  4,  8,  16,  32,  64,  128 },
clr[] = { (uint8_t)~1 , (uint8_t)~2 , (uint8_t)~4 , (uint8_t)~8,
          (uint8_t)~16, (uint8_t)~32, (uint8_t)~64, (uint8_t)~128 };

void sendbyte(uint8_t data);
void sendbyteLSB(uint8_t data);

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif
#ifndef _swap_uint16_t
#define _swap_uint16_t(a, b) { uint16_t t = a; a = b; b = t; }
#endif

#define SHARPMEM_BIT_WRITECMD   (0x80)
#define SHARPMEM_BIT_VCOM       (0x40)
#define SHARPMEM_BIT_CLEAR      (0x20)
#define TOGGLE_VCOM             do { _sharpmem_vcom = _sharpmem_vcom ? 0x00 : SHARPMEM_BIT_VCOM; } while(0);

/******************************************************************************
 *  Private functions
 *****************************************************************************/

/**************************************************************************//**
 *  @brief Sends a single byte in pseudo-SPI.
 *****************************************************************************/
void SharpMemSendbyte(uint8_t data)
{
	uint8_t i = 0;

	// LCD expects LSB first

#if defined(USE_FAST_PINIO)
	for (i = 0; i < 8; i++)
	{
		// Make sure clock starts low
		*clkport &= ~clkpinmask;
		
		if (data & 0x80) 
			*dataport |=  datapinmask;
		else 
			*dataport &= ~datapinmask;

		// Clock is active high
		*clkport |=  clkpinmask;
		data <<= 1; 
	}
	
	*clkport &= ~clkpinmask;
#else
	for (i = 0; i < 8; i++)
	{ 
		// Make sure clock starts low
		digitalWrite(_clk, LOW);
		
		if (data & 0x80) 
			digitalWrite(_mosi, HIGH);
		else 
			digitalWrite(_mosi, LOW);

		// Clock is active high
		digitalWrite(_clk, HIGH);
		data <<= 1; 
	}
	
	// Make sure clock ends low
	digitalWrite(_clk, LOW);
#endif
}

void SharpMemSendbyteLSB(uint8_t data) 
{
	uint8_t i = 0;

	// LCD expects LSB first
	
#if defined(USE_FAST_PINIO)
	for (i=0; i<8; i++)
	{ 
		// Make sure clock starts low
		*clkport &= ~clkpinmask;
		
		if (data & 0x01) 
			*dataport |=  datapinmask;
		else 
			*dataport &= ~datapinmask;
		
		// Clock is active high
		*clkport |=  clkpinmask;
		data >>= 1; 
	}
	
	// Make sure clock ends low
	*clkport &= ~clkpinmask;
#else
	for (i=0; i<8; i++) 
	{ 
		// Make sure clock starts low
		digitalWrite(_clk, LOW);
		
		if (data & 0x01) 
			digitalWrite(_mosi, HIGH);
		else 
			digitalWrite(_mosi, LOW);
		
		// Clock is active high
		digitalWrite(_clk, HIGH);
		data >>= 1; 
	}
	
	// Make sure clock ends low
	digitalWrite(_clk, LOW);
#endif
}

/******************************************************************************
 * Exported Functions 
 *****************************************************************************/

/**************************************************************************//**
 *  @brief	Initialize the display module
 *****************************************************************************/
bool SharpMemInit(uint8_t clk, uint8_t mosi, uint8_t ss, uint16_t width, uint16_t height)
{
	bool result = false;				// pessimistic return value
	
	// Initialize graphics library.
	Adafruit_GFX(width, height);
	
	// Remember SPI pins used.
	_clk  = clk;
	_mosi = mosi;
	_ss   = ss;

	// Set pin state before direction to make sure they start this way (no glitching)
	digitalWrite(_ss, HIGH);  
	digitalWrite(_clk, LOW);  
	digitalWrite(_mosi, HIGH);  

	pinMode(_ss, OUTPUT);
	pinMode(_clk, OUTPUT);
	pinMode(_mosi, OUTPUT);

#if defined(USE_FAST_PINIO)
	clkport     = portOutputRegister(digitalPinToPort(_clk));
	clkpinmask  = digitalPinToBitMask(_clk);
	dataport    = portOutputRegister(digitalPinToPort(_mosi));
	datapinmask = digitalPinToBitMask(_mosi);
#endif

	// Set the vcom bit to a defined state
	_sharpmem_vcom = SHARPMEM_BIT_VCOM;

	sharpmem_buffer = (byte *)malloc((WIDTH * HEIGHT) / 8);

	if (sharpmem_buffer)
	{
		setRotation(0);
		
		// Success!
		result = true;
	}

	return result;
}

/**************************************************************************//**
 *  @brief Draws a single pixel in image buffer
 *  @param[in]  x - the x position (0 based)
 *  @param[in]  y - the y position (0 based)
 *****************************************************************************/
void SharpMemSetPixel(uint16_t x, uint16_t y, uint16_t color)
{
	bool result = false;				// pessimistic return value
	
	// If an argument is invalid...
	if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height))
	{
		result = false;
	}
	// If the arguments are all valid...
	else
	{
		// 
		switch(rotation)
		{
		case 1:
			_swap_int16_t(x, y);
			x = WIDTH  - 1 - x;
			break;
		case 2:
			x = WIDTH  - 1 - x;
			y = HEIGHT - 1 - y;
			break;
		case 3:
			_swap_int16_t(x, y);
			y = HEIGHT - 1 - y;
			break;
		}

		if (color)
		{
			sharpmem_buffer[(y * WIDTH + x) / 8] |= pgm_read_byte(&set[x & 7]);
		}
		else
		{
			sharpmem_buffer[(y * WIDTH + x) / 8] &= pgm_read_byte(&clr[x & 7]);
		}
		
		// Success!
		result = true;
	}
	
	return result;
}

/**************************************************************************//** 
 *	@brief Gets the value (1 or 0) of the specified pixel from the buffer
 *	@param[in]	x - the x position (0 based)
 *	@param[in]	y - the y position (0 based)
 *	@return		1 if the pixel is enabled, 0 if disabled
 *****************************************************************************/
bool SharpMemGetPixel(uint16_t x, uint16_t y)
{
	bool result = false;				// pessimistic return value

	// If an argument is invalid...
	if((x >= _width) || (y >= _height))
	{
		result = false;
	}
	else
	{
		switch(rotation)
		{
		case 1:
			_swap_uint16_t(x, y);
			x = WIDTH  - 1 - x;
			break;
		case 2:
			x = WIDTH  - 1 - x;
			y = HEIGHT - 1 - y;
			break;
		case 3:
			_swap_uint16_t(x, y);
			y = HEIGHT - 1 - y;
			break;
		}

		return sharpmem_buffer[(y * WIDTH + x) / 8] & pgm_read_byte(&set[x & 7]) ? 1 : 0;
	}
}

/**************************************************************************//**
 *	@brief Clears the screen
 *****************************************************************************/
void SharpMemClearDisplay() 
{
	memset(sharpmem_buffer, 0xff, (WIDTH * HEIGHT) / 8);
	// Send the clear screen command rather than doing a HW refresh (quicker)
	digitalWrite(_ss, HIGH);
	sendbyte(_sharpmem_vcom | SHARPMEM_BIT_CLEAR);
	sendbyteLSB(0x00);
	TOGGLE_VCOM;
	digitalWrite(_ss, LOW);
}

/**************************************************************************//**
 *	@brief Renders the contents of the pixel buffer on the LCD
 *****************************************************************************/
void SharpMemRefresh(void) 
{
	uint16_t i, totalbytes, currentline, oldline;  
	totalbytes = (WIDTH * HEIGHT) / 8;

	// Send the write command
	digitalWrite(_ss, HIGH);
	sendbyte(SHARPMEM_BIT_WRITECMD | _sharpmem_vcom);
	TOGGLE_VCOM;

	// Send the address for line 1
	oldline = currentline = 1;
	sendbyteLSB(currentline);

	// Send image buffer
	for (i=0; i<totalbytes; i++)
	{
		sendbyteLSB(sharpmem_buffer[i]);
		
		currentline = ((i+1)/(WIDTH/8)) + 1;
		
		if(currentline != oldline)
		{
			// Send end of line and address bytes
			sendbyteLSB(0x00);
			
			if (currentline <= HEIGHT)
			{
				sendbyteLSB(currentline);
			}
			
			oldline = currentline;
		}
	}

	// Send another trailing 8 bits for the last line
	sendbyteLSB(0x00);
	digitalWrite(_ss, LOW);
}