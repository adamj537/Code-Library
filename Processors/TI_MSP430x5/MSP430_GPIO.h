#ifndef MSP430_GPIO_H
#define MSP430_GPIO_H

/******************************************************************************
 * TYPEDEFS AND STRUCTURES
 *****************************************************************************/

typedef enum							// pin direction
{
	GPIO_DIR_IN,						// input mode
	GPIO_DIR_OUT,						// output mode
	GPIO_DIR_BOTH						// bidirectional mode
} GPIOdir_t;

typedef enum							// pin pull
{
	GPIO_PULL_NONE,						// no resistors
	GPIO_PULL_UP,						// use pull-up resistors
	GPIO_PULL_DOWN						// use pull-down resistors
} GPIOpull_t;

typedef enum							// pin interrupt type
{
	GPIO_INT_NONE,						// disabled
	GPIO_INT_EDGE_RISING,				// low-to-high transition
	GPIO_INT_EDGE_FALLING,				// high-to-low transition
	GPIO_INT_EDGE_BOTH,					// any transition
	GPIO_INT_LEVEL_HIGH,				// interrupt whenever we're high
	GPIO_INT_LEVEL_LOW					// interrupt whenever we're low
} GPIOIntType_t;

typedef enum							// GPIO status
{
	GPIO_RESULT_OK = 0,					// All is well!
	GPIO_RESULT_FAIL,					// It's the target's fault.
	GPIO_RESULT_NOT_IMPLEMENTED,		// It's my fault.
	GPIO_RESULT_INVALID_SELECTION		// It's your fault.
} GPIOResult_t;

// This enumeration produces valid indexes for GPIO ports which are enabled by
// the application configuration.
typedef enum {
	GPIO_PORTA = 0,
	GPIO_PORTB,
	GPIO_PORTC,
	GPIO_PORTD,
	GPIO_PORTE,
	GPIO_PORTF,
	GPIO_PORTG,
	GPIO_PORTH,
	GPIO_PORTI,
	GPIO_PORTJ,
	GPIO_PORTK,
	GPIO_PORTL
} GPIOPort_t;

// GPIO Port pin configuration structure.
typedef struct {
	uint32_t function;		// index of the peripheral that should control the pin
	GPIOdir_t direction;	// Port buffer input/output direction. 
	GPIOpull_t inputPull;	// Logic level pull of the input buffer. 
	BOOL powersave;			// Enable the lowest possible power state on the pin (All other configurations will be ignored, the pin will be disabled)
} GPIOConfig_t;

// Configuration structure for the edge detection mode of an external interrupt
typedef struct {
	uint32_t function;					// MUX position the GPIO pin should be configured to. 
	GPIOpull_t inputPull;				// Internal pull to enable on the input pin. 
	BOOL wakeIfSleeping;				// Wake up the device if the channel interrupt fires during sleep mode.
	BOOL ui32_FilterInputSignal;		// Filter the raw input signal to prevent noise from triggering an interrupt accidentally 
	GPIOIntType_t DetectionCriteria;	// Edge detection mode to use.
} GPIOIntConfig_t;

// GPIO Interrupt callback function prototype
typedef void (*GPIOIntCallback_t)(void);

// Port Initialization Functions
GPIOResult_t GPIOInit(GPIOPort_t port);
GPIOResult_t GPIODeinit(GPIOPort_t port);

// Port Oriented Functions
GPIOResult_t GPIOConfigPort(GPIOPort_t port, GPIOPortSize_t mask, GPIOConfig_t *configPtr);
GPIOPortSize_t GPIOReadPort(GPIOPort_t port, GPIOPortSize_t mask);
GPIOResult_t GPIOWritePort(GPIOPort_t port, GPIOPortSize_t mask, uint8_t ui8_portValue);
GPIOResult_t GPIOTogglePort(GPIOPort_t port, GPIOPortSize_t mask);

// Pin Oriented Functions
GPIOResult_t GPIOConfigPin(GPIOPort_t port, GPIOPortSize_t pin, GPIOConfig_t *configPtr);
uint8_t GPIOReadPin(GPIOPort_t port, GPIOPortSize_t pin);
GPIOResult_t GPIOWritePin(GPIOPort_t port, GPIOPortSize_t pin, uint8_t value);
GPIOResult_t GPIOTogglePin(GPIOPort_t port, GPIOPortSize_t pin);

// Interrupt Configuration Functions
GPIOResult_t GPIOConfigInterrupt(GPIOPort_t port, GPIOPortSize_t pin, GPIOIntConfig_t *configPtr, GPIOIntCallback_t callbackPtr);
GPIOResult_t GPIOEnableInterrupt(GPIOPort_t port, GPIOPortSize_t pin);
GPIOResult_t GPIODisableInterrupt(GPIOPort_t port, GPIOPortSize_t pin);

GPIOResult_t GPIOTest(void);

#endif /* MSP430_GPIO_H */
