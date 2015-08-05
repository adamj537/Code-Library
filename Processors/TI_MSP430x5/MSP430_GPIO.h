#ifndef GPIO__H
#define GPIO__H

/*****************************************************************************
** TYPEDEFS AND STRUCTURES
*****************************************************************************/

typedef enum							// pin levels
{
	GPIOPinValueLow,
	GPIOPinValueHigh
} GPIOlevel_t;

typedef enum							// pin direction
{
	GPIODirectionInput,
	GPIODirectionOutput,
	GPIODirectionInputOutput
} GPIOdir_t;

typedef enum							// pin pull
{
	GPIOPinPullNone,
	GPIOPinPullUp,
	GPIOPinPullDown
} GPIOpull_t;

typedef enum							// pin interrupt sensitivity
{
	GPIOIntDetectNone,
	GPIOIntDetectEdgeRising,
	GPIOIntDetectEdgeFalling,
	GPIOIntDetectEdgeBoth,
	GPIOIntDetectLevelHigh,
	GPIOIntDetectLevelLow
} GPIOIntDetect_t;

typedef enum							// GPIO status
{
	GPIOResultOK = 0,
	GPIOResultFail,
	GPIOResultNotImplemented,
	GPIOResultInvalidSelection
} GPIOResult_t;

/*!
 * This enumeration produces valid indexes for GPIO ports which are
 * enabled by the application configuration. These enumeration values should
 * NOT be used directly by the caller of GPIO functions. Instead, the application
 * should use a #define to create an association to the desired port.
 */
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
} eGPIOPort_t;

/*! GPIO Interrupt callback function prototype */
typedef void (*GPIOIntCallback_t)(void);

/* GPIO Port pin configuration structure.
 * Configuration structure for a port pin instance.
 */
typedef struct {
	uint32_t ui32_MuxPosition;		//<! MUX index of the peripheral that should control the pin 
	uint32_t ui32_Direction;		//<! Port buffer input/output direction. 
	uint32_t ui32_InputPull;		//<! Logic level pull of the input buffer. 
	uint32_t ui32_Powersave;		//<! Enable the lowest possible power state on the pin (All other configurations will be ignored, the pin will be disabled)
} sGPIOConfig_t;

/* External channel configuration structure.
 * Configuration structure for the edge detection mode of an external
 * interrupt channel.
 */
typedef struct {
	uint32_t ui32_MuxPosition;			//<! MUX position the GPIO pin should be configured to. 
	uint32_t ui32_InputPull;			//<! Internal pull to enable on the input pin. 
	uint32_t ui32_WakeIfSleeping;		//<! Wake up the device if the channel interrupt fires during sleep mode.
	uint32_t ui32_FilterInputSignal;	//<! Filter the raw input signal to prevent noise from triggering an interrupt accidentally 
	uint32_t ui32_DetectionCriteria;	//<! Edge detection mode to use.
} sGPIOIntConfig_t;

/************************************************************************/
/* Port Initialization Functions										*/
/************************************************************************/
/*!
* @brief Initialize the selected GPIO port
* @param[in] ui32_GpioPort The index/identifier of the GPIO port to initialize
* @return GPIOResultOK on success, GPIOResultFail on failure
*/
eGPIOResult_t GPIOInit(eGPIOPort_t eGP_gpioPort);
/*!
* @brief Uninitialize the selected GPIO port
* @param[in] ui32_GpioPort The index/identifier of the GPIO port to uninitialize.
* @return GPIOResultOK on success, GPIOResultFail on failure
*/
eGPIOResult_t GPIODeinit(eGPIOPort_t eGP_gpioPort);

/************************************************************************/
/* Port Oriented Functions												*/
/************************************************************************/
/*!
* @brief Configure the pins specified by mask in the selected port
* @param[in] eGP_gpioPort The index/identifier of the GPIO port to configure
* @param[in] gPS_Mask The specific pins within the port to configure
* @param[in] sPtr_GpioConfig A pointer to a sGpioConfig_t
* @return GPIOResultOK on success, GPIOResultInvalidSelection on failure
*/
eGPIOResult_t GPIOConfigPort(eGPIOPort_t eGP_gpioPort, GPIOPortSize_t gPS_Mask, sGPIOConfig_t *sPtr_gpioConfig);
/*!
* @brief Read the state of the pins in the specified port with respect to the mask
* that are configured as inputs.
* @param[in] eGP_gpioPort The index/identifier of the GPIO port to read
* @param[in] gPS_Mask A bit mask of pins to read from the port
* @return The value of the pins of the port in the mask that are configured as inputs
*/
GPIOPortSize_t GPIOReadPort(eGPIOPort_t eGP_gpioPort, GPIOPortSize_t gPS_mask);
/*!
 * @brief Write the value of the pins in the specified port with respect to the mask
 *  that are configured as outputs.
 * @param[in] eGP_gpioPort The index/identifier of the GPIO port to write
 * @param[in] gPS_Mask A bit mask of pins to write in the port
 * @param[in] ui8_PortValue The value to write to the masked pins.
 * @return GPIOResultOK on success, GPIOResultInvalidSelection on failure
 */
eGPIOResult_t GPIOWritePort(eGPIOPort_t eGP_gpioPort, GPIOPortSize_t gPS_mask, uint8_t ui8_portValue);
/*!
 * @brief Toggle the state of the pins in the specified port with respect to the mask
 * that are configured as outputs.
 * @param[in] eGP_gpioPort The index/identifier of the GPIO port to Toggle
 * @param[in] gPS_Mask A bit mask of pins to toggle on the port
 * @return GPIOResultOK on success, GPIOResultInvalidSelection on failure
 */
eGPIOResult_t GPIOTogglePort(eGPIOPort_t eGP_gpioPort, GPIOPortSize_t gPS_mask);

/************************************************************************/
/* Pin Oriented Functions												*/
/************************************************************************/
/*!
* @brief Configure a single pin in the specified port.
* @param[in] eGP_gpioPort The index/identifier of the GPIO port the pin belongs to.
* @param[in] eGP_GpioPin The index/identifier of the GPIO pin to configure
* @param[in] sPtr_GpioConfig A pointer to a sGpioConfig_t
* @return GPIOResultOK on success, GPIOResultInvalidSelection on failure
*/
eGPIOResult_t GPIOConfigPin(eGPIOPort_t eGP_gpioPort, GPIOPortSize_t eGP_gpioPin, sGPIOConfig_t *sPtr_gpioConfig);
/*!
 * @brief Read the state of the pin specified (if input)
 * @param[in] eGP_gpioPort The index/identifier of the GPIO port to read
 * @param[in] ePS_gpioPin The index/identifier of the GPIO pin to read
 * @return The value of the pin (if input)
 */
uint8_t GPIOReadPin(eGPIOPort_t eGP_gpioPort, GPIOPortSize_t ePS_gpioPin);
/*!
 * @brief Write the value of the pin specified (if an output)
 * @param[in] eGP_gpioPort The index/identifier of the GPIO port to write
 * @param[in] eGP_GpioPin The index/identifier of the GPIO pin to write
 * @param[in] ui8_PinValue The value to write to the pin.
 * @return GPIOResultOK on success, GPIOResultInvalidSelection on failure
 */
eGPIOResult_t GPIOWritePin(eGPIOPort_t eGP_gpioPort, GPIOPortSize_t eGP_gpioPin, uint8_t ui8_pinValue);
/*!
* @brief Toggle the state of the pin specified (if configured as output)
* @param[in] eGP_gpioPort The index/identifier of the GPIO port to Toggle
* @param[in] eGP_GpioPin The index/identifier of the GPIO pin to Toggle
* @return GPIOResultOK on success, GPIOResultInvalidSelection on failure
*/
eGPIOResult_t GPIOTogglePin(eGPIOPort_t eGP_gpioPort, GPIOPortSize_t eGP_gpioPin);

/************************************************************************/
/* Interrupt Configuration Functions									*/
/************************************************************************/
/*!
 * @brief Configure a GPIO pin as an external interrupt source and register a callback function or unregister
 *  a previously registered callback function.
 * @param[in] eGP_gpioPort The index/identifier of the GPIO port to configure.
 * @param[in] eGP_GpioPin The index/identifier of the GPIO pin to configure.
 * @param[in] sPtr_GpioIntConfig A pointer to a sGPIOIntConfig_t structure with the desired pin/interrupt configuration
 * @param[in] funcPtr_GpioCallback A pointer to a function that will receive the interrupt, or NULL to unregister the callback function
 * @return GPIOResultOK on success, GPIOResultFail on failure
 */
eGPIOResult_t GPIOConfigInterrupt(eGPIOPort_t eGP_gpioPort, GPIOPortSize_t eGP_gpioPin, sGPIOIntConfig_t *sPtr_gpioIntConfig, GPIOIntCallback_t funcPtr_GpioCallback);
/*!
 * @brief Enable the interrupt for the specified pin. (Must be configured first!)
 * @param[in] eGP_gpioPort The index/identifier of the GPIO port to enable the interrupt for.
 * @param[in] eGP_GpioPin The index/identifier of the GPIO pin to enable the interrupt for.
 * @return GPIOResultOK on success, GPIOResultInvalidSelection on failure.
 */
eGPIOResult_t GPIOEnableInterrupt(eGPIOPort_t eGP_gpioPort, GPIOPortSize_t eGP_gpioPin);
/*!
 * @brief Disable the interrupt for the specified pin. 
 * @param[in] eGP_gpioPort The index/identifier of the GPIO port to enable the interrupt for.
 * @param[in] eGP_GpioPin The index/identifier of the GPIO pin to disable the interrupt for.
 * @return GPIOResultOK on success, GPIOResultInvalidSelection on failure.
 */
eGPIOResult_t GPIODisableInterrupt(eGPIOPort_t eGP_gpioPort, GPIOPortSize_t eGP_gpioPin);

/************************************************************************/
/* Test Functions                                                       */
/************************************************************************/
eGPIOResult_t GPIOTest(void);

/*! @} */ /* End of expfuncs group. */


#endif /* HAL_GPIO_H_ */
