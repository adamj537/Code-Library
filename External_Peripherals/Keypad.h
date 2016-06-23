/******************************************************************************
 *
 *	Filename:		Keypad.h
 *
 *	Author:			Adam Johnson
 *
 *	Description:	Library to handle buttons, switches, quadrature encoders,
 *					key matrices.
 *
 *****************************************************************************/
 
#ifndef KEYPAD_H
#define KEYPAD_H

/******************************************************************************
 *	Enumerations
 *****************************************************************************/

typedef enum
{
	KeyResultOK,
	KeyResultInvalidSelection,
	KeyResultFail
} eKeyResult_t;

typedef enum
{
	KeytypeButton,
	KeytypeEncoder,
	KeytypeSelector,
	KeytypeMatrix
} eKeyType_t;

typedef enum
{
	KeyEventButton,
	KeyEventEncoder,
	KeyEventSelector
} eKeyEventType_t;

/******************************************************************************
 *	Pin Configuration Structures
 *****************************************************************************/

typedef struct							// structure with pin info for button
{
	DUINT32_T repeatTime;				// time to register a repeated key press
	eGPIOPort_t port;					// button's port
	GPIOPortSize_t pin;					// button's pin
	DBOOL_T assertionState;				// logic level when pin is activated
} sButtonPinConfig_t;

typedef struct							// structure with pin info for quadrature encoder
{
	eGPIOPort_t portA;					// channel A port
	eGPIOPort_t portB;					// channel B port
	GPIOPortSize_t pinA;				// channel A pin
	GPIOPortSize_t pinB;				// channel B pin
} sRotaryEncoderPinConfig_t;

typedef struct							// structure with pin info for selector or switch
{
	eGPIOPort_t *portArray;				// array of ports used by each key
	GPIOPortSize_t *pinArray;			// array of pins used by each key
	DUINT8_T pinArraySize;				// num pins used (size of arrays)
	DBOOL_T assertionState;				// logic level when pin is activated
} sSelectorPinConfig_t;

typedef struct							// structure with pin info for matrix
{
	DUINT32_T repeatTime;				// time to register a repeated key press
	eGPIOPort_t *portArrayX;			// array of ports for matrix columns
	GPIOPortSize_t *pinArrayX;			// array of pins for matrix columns
	eGPIOPort_t *portArrayY;			// array of ports for matrix rows
	GPIOPortSize_t *pinArrayY;			// array of pins for matrix rows
	DUINT8_T numX;						// number of columns (size of column arrays)
	DUINT8_T numY;						// number of rows (size of row arrays)
	DBOOL_T assertionState;				// logic level when pin is activated
} sMatrixPinConfig_t;

/******************************************************************************
 *	Key Configuration Structure/Union
 *****************************************************************************/

typedef union							// union for the various types of keys
{
	sButtonPinConfig_t button;			// button
	sRotaryEncoderPinConfig_t encoder;	// rotary encoder
	sSelectorPinConfig_t selector;		// selector / switch
	sMatrixPinConfig_t matrix;			// key matrix
} uKeyConfig_t;

typedef struct							// structure to configure a keypad
{
	uKeyConfig_t pinConfig;				// info about what pins are used
	eKeyType_t type;					// button, encoder, selector, matrix
} sKeyConfig_t;

/******************************************************************************
 *	Key State Structure/Union
 *****************************************************************************/

typedef enum
{
	ButtonEventPress,
	ButtonEventRelease,
	ButtonEventPressAndRelease,
	ButtonEventPressAndHold,
	ButtonEventDoublePressAndRelease
} eButtonEvent_t;

typedef enum
{
	RotaryEncoderEventClockwise,
	RotaryEncoderEventCounterclockwise
} eRotaryEncoderEvent_t;

typedef union
{
	eButtonEvent_t buttonEvent;			// button / matrix events
	eRotaryEncoderEvent_t rotaryEvent;	// rotary encoder events
	DUINT8_T selectorEvent;				// selector / switch events
} uKeyEvent_t;

typedef struct							// structure to report a keypress
{
	uKeyEvent_t event;					// info about the triggered event
	eKeyEventType_t type;				// button, encoder, selector, matrix
} sKeyState_t;

/******************************************************************************
 *	Function Prototypes
 *****************************************************************************/

typedef void *keyCallback_t(sKeyConfig_t *config, sKeyState_t *state);

eKeyResult_t KeyInit(void);
eKeyResult_t KeyRegister(sKeyConfig_t *config);
eKeyResult_t KeyUnregister(sKeyConfig_t *config);
eKeyResult_t KeyGet(void);
 
#endif	// KEYPAD_H
