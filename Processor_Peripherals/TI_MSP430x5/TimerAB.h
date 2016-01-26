//*****************************************************************************
//
// TimerAB.h - Driver for the TIMER A & B Modules.  Based on TI's MSP430Ware.
//
//*****************************************************************************

#ifndef MSP430_TIMER_AB__H
#define MSP430_TIMER_AB__H

// These defines let us figure out what timer peripherals the MSP430 has.
#if defined(__MSP430_HAS_T0A2__) || defined(__MSP430_HAS_T1A2__) \
 || defined(__MSP430_HAS_T2A2__) || defined(__MSP430_HAS_T3A2__) \
 || defined(__MSP430_HAS_T0A3__) || defined(__MSP430_HAS_T1A3__) \
 || defined(__MSP430_HAS_T2A3__) || defined(__MSP430_HAS_T3A3__) \
 || defined(__MSP430_HAS_T0A5__) || defined(__MSP430_HAS_T1A5__) \
 || defined(__MSP430_HAS_T2A5__) || defined(__MSP430_HAS_T3A5__) \
 || defined(__MSP430_HAS_T0A7__) || defined(__MSP430_HAS_T1A7__) \
 || defined(__MSP430_HAS_T2A7__) || defined(__MSP430_HAS_T3A7__)
  #define MSP430_HAS_TIMER_A
#endif

#if defined(__MSP430_HAS_T0B3__) || defined(__MSP430_HAS_T0B5__) \
 || defined(__MSP430_HAS_T0B7__) || defined(__MSP430_HAS_T1B3__) \
 || defined(__MSP430_HAS_T1B5__) || defined(__MSP430_HAS_T1B7__)
  #define MSP430_HAS_TIMER_B
#endif

/******************************************************************************
 * The following is a parameter used for TimerGetCounterValue that
 * determines the maximum difference in counts of the TAxR register for a
 * majority vote.
 *****************************************************************************/
#define TIMER_THRESHOLD	50

/******************************************************************************
 * The following are registers used by Timer A & B modules.
 *****************************************************************************/

#define ABTIMER_REG_CTL		(0x0000)	// Main Control
#define ABTIMER_REG_CCTL0	(0x0002)	// Capture/Compare Control 0
#define ABTIMER_REG_CCTL1	(0x0004)	// Capture/Compare Control 1
#define ABTIMER_REG_CCTL2	(0x0006)	// Capture/Compare Control 2
#define ABTIMER_REG_CCTL3	(0x0008)	// Capture/Compare Control 3
#define ABTIMER_REG_CCTL4	(0x000A)	// Capture/Compare Control 4
#define ABTIMER_REG_CCTL5	(0x000C)	// Capture/Compare Control 5
#define ABTIMER_REG_CCTL6	(0x000E)	// Capture/Compare Control 6
#define ABTIMER_REG_R		(0x0010)	// the count of the timer
#define ABTIMER_REG_CCR0	(0x0012)	// Capture/Compare 0
#define ABTIMER_REG_CCR1	(0x0014)	// Capture/Compare 1
#define ABTIMER_REG_CCR2	(0x0016)	// Capture/Compare 2
#define ABTIMER_REG_CCR3	(0x0018)	// Capture/Compare 3
#define ABTIMER_REG_CCR4	(0x001A)	// Capture/Compare 4
#define ABTIMER_REG_CCR5	(0x001C)	// Capture/Compare 5
#define ABTIMER_REG_CCR6	(0x001E)	// Capture/Compare 6
#define ABTIMER_REG_IV		(0x002E)	// Interrupt Vector Word
#define ABTIMER_REG_EX0		(0x0020)	// Interrupt Divider Expansion

/*****************************************************************************
 * Enumeration:	Timer Clock Source (part of CTL register)
 *
 * Where used:	TimerInitContinuousMode(), TimerInitUpMode(),
 * 				TimerInitUpDownMode(), TimerOutputPWM()
 *
 * Values:		External TxClk - external clocking
 * 				ACLK - auxiliary clock
 * 				SMCLK - system main clock
 * 				Inverted External TxClk - what it sounds like :)
 *****************************************************************************/
typedef enum {
	TIMER_CLOCK_EXTERNAL_TXCLK			= (0*0x100u),	// TxSSEL__TACLK,
	TIMER_CLOCK_ACLK					= (1*0x100u),	// TxSSEL__ACLK,
	TIMER_CLOCK_SMCLK					= (2*0x100u),	// TxSSEL__SMCLK,
	TIMER_CLOCK_INVERTED_EXTERNAL_TXCLK	= (3*0x100u),	// TBSSEL__INCLK,
} tmrClk_t;

/******************************************************************************
 * Enumeration:	Timer Clock Source Divider - the ID bits of the CTL register and
 * 				the TxIDEX register determine the clock divider.
 *
 * Where used:	TimerInitContinuousMode(), TimerInitUpMode(),
 * 				TimerInitUpDownMode(), TimerOutputPWM()
/ *****************************************************************************/
typedef enum {
	TIMER_CLOCK_DIVIDER_1 = 0x00,
	TIMER_CLOCK_DIVIDER_2 = 0x08,
	TIMER_CLOCK_DIVIDER_3 = 0x02,
	TIMER_CLOCK_DIVIDER_4 = 0x10,
	TIMER_CLOCK_DIVIDER_5 = 0x04,
	TIMER_CLOCK_DIVIDER_6 = 0x05,
	TIMER_CLOCK_DIVIDER_7 = 0x06,
	TIMER_CLOCK_DIVIDER_8 = 0x18,
	TIMER_CLOCK_DIVIDER_10 = 0x0C,
	TIMER_CLOCK_DIVIDER_12 = 0x0D,
	TIMER_CLOCK_DIVIDER_14 = 0x0E,
	TIMER_CLOCK_DIVIDER_16 = 0x0F,
	TIMER_CLOCK_DIVIDER_20 = 0x14,
	TIMER_CLOCK_DIVIDER_24 = 0x15,
	TIMER_CLOCK_DIVIDER_28 = 0x16,
	TIMER_CLOCK_DIVIDER_32 = 0x17,
	TIMER_CLOCK_DIVIDER_40 = 0x1C,
	TIMER_CLOCK_DIVIDER_48 = 0x1D,
	TIMER_CLOCK_DIVIDER_56 = 0x1E,
	TIMER_CLOCK_DIVIDER_64 = 0x1F,
} tmrDiv_t;

/******************************************************************************
 * Enumeration:	Timer Mode (part of CTL register)
 *
 * Where Used:	TimerStartCounter()
 *
 * Values:
 * 	Stop:		stops the timer.
 * 	Up:			timer counts up to the value of CCR0 register (which defines
 * 					the period), then restarts from zero.
 * 	Continuous:	timer counts to 0xFFFF, then restarts from zero; used to make a
 * 					timer interval or generate frequencies, and it's simpler this
 * 					way than using the CCR0 register for a period.  In this mode,
 * 					the CCR0 register acts like the other CCR registers.
 * 	Up/Down:	timer repeatedly counts up to the value of CCR0 register, then
 * 					back down to zero; useful when dead time is required between
 * 					two output signals (i.e. H-bridges).
 *****************************************************************************/
typedef enum {
	TIMER_MODE_STOP			= MC_0,
	TIMER_MODE_UP			= MC_1,
	TIMER_MODE_CONTINUOUS	= MC_2,
	TIMER_MODE_UPDOWN		= MC_3,
} tmrMode_t;

/******************************************************************************
 * Enumeration:	Timer Interrupt Enable (part of CTL register) This enables
 * 				the TAIFG interrupt.
 *
 * Where used:	TimerInitContinuousMode(), TimerInitUpMode(),
 * 				TimerInitUpDownMode()
 *
 * Values:		Enable - enable the interrupt
 * 				Disable - disable the interrupt
 *****************************************************************************/
typedef enum {
	TIMER_INTERRUPT_ENABLE		= 0x02,	// TAIE = TBIE = (0x0002)
	TIMER_INTERRUPT_DISABLE		= 0x00,
} tmrIE_t;

/*****************************************************************************
 * Enumeration:	Timer Capture Mode (part of CCTLn registers)
 *
 * Where used:	TimerInitCaptureMode()
 *
 * Values:		No capture - the corresponding capture channel is disabled
 * 				Rising edge - capture on rising edge of input signal
 * 				Falling edge - capture on falling edge of input signal
 * 				Rising/falling - capture on both rising and falling edges
 *****************************************************************************/
typedef enum {
	TIMER_CAPTURE_NONE							= CM_0,
	TIMER_CAPTURE_ON_RISING_EDGE				= CM_1,
	TIMER_CAPTURE_ON_FALLING_EDGE				= CM_2,
	TIMER_CAPTURE_ON_RISING_AND_FALLING_EDGE	= CM_3,
} tmrCMode_t;

/*****************************************************************************
 * Enumeration:	Capture Mode Input Select (part of CCTLn registers)
 *
 * Description:	These bits select the CCR0 input signal that will be acted
 * 				upon (see Timer Capture Mode enum for how it will be acted
 * 				upon).
 *
 * Where Used:	TimerInitCaptureMode()
 *
 * Values:		CCIxA - use the CCIxA signal as the capture input
 * 				CCIxB - use the CCIxB signal as the capture input
 * 				GND - ???
 * 				VCC - ???
 *****************************************************************************/
typedef enum {
	TIMER_CAPTURE_INPUT_CCIxA	= CCIS_0,
	TIMER_CAPTURE_INPUT_CCIxB	= CCIS_1,
	TIMER_CAPTURE_INPUT_GND		= CCIS_2,
	TIMER_CAPTURE_INPUT_VCC		= CCIS_3,
} tmrInput_t;

/******************************************************************************
 * Enumeration:	Output Mode (part of CCTLn registers). Modes Toggle/reset,
 * 				Set/reset, Toggle/set, and Reset/set can't be used with CCR0.
 *
 * Where Used:	TimerInitCaptureMode(), TimerInitCompareMode(),
 * 				TimerOutputPWM()
 *
 * Values:
 * 	Output - the output signal OUTn is defined by the CCTLn register's OUT bit.
 *	Set - the output is set when the timer counts to the CCRn value, and is
 *			reset when the timer counts to the CCR0 value.
 *	Toggle/Reset - the output is toggled when the timer counts to the CCRn
 * 			value, and resets when the timer counts to the CCR0 value.
 *	Set/Reset - the output is set when the timer counts to the CCRn value, and
 * 			resets when the timer counts to the CCR0 value.
 *	Toggle - the output is toggled when the timer counts to the CCRn value; the
 *			output period is double the timer period.
 *	Reset - the output is reset when the timer counts to the CCRn value, and
 *			remains reset until another output mode is selected and affects
 *			the output.
 *	Toggle/Set - the output is toggled when the timer counts to the CCRn value,
 *			and is set when the timer counts to the CCR0 value.
 *	Reset/Set - the output is reset when the timer counts to the CCRn value,
 *			and is set when the timer counts to the CCR0 value.
 *****************************************************************************/
typedef enum {
	TIMER_OUTPUTMODE_OUTBITVALUE	= OUTMOD_0,
	TIMER_OUTPUTMODE_SET			= OUTMOD_1,
	TIMER_OUTPUTMODE_TOGGLE_RESET	= OUTMOD_2,
	TIMER_OUTPUTMODE_SET_RESET		= OUTMOD_3,
	TIMER_OUTPUTMODE_TOGGLE			= OUTMOD_4,
	TIMER_OUTPUTMODE_RESET			= OUTMOD_5,
	TIMER_OUTPUTMODE_TOGGLE_SET		= OUTMOD_6,
	TIMER_OUTPUTMODE_RESET_SET		= OUTMOD_7,
} tmrOutMode_t;

/******************************************************************************
 * Enumeration:	Capture/Compare Registers - in compare mode, they hold the data
 * 				for the comparison to the timer value in the timer register (R);
 * 				in capture mode, when a capture occurs, the value of the timer
 * 				register (R) is copied into the capture/compare register.
 *
 * Where used:	compareRegister parameter in TimerInitCompareLatchLoadEvent(),
 * 				TimerSetCompareValue(); captureCompareRegister parameter in
 * 				TimerEnableCaptureCompareInterrupt(),
 * 				TimerDisableCaptureCompareInterrupt(),
 * 				TimerGetCaptureCompareInterruptStatus(),
 * 				TimerGetSynchronizedCaptureCompareInput(),
 * 				TimerGetOutputForOutputModeOutBitValue(),
 * 				TimerGetCaptureCompareCount(),
 * 				TimerSetOutputForOutputModeOutBitValue(),
 * 				TimerClearCaptureCompareInterruptFlag(); the param parameter in
 * 				TimerInitCaptureMode(), TimerInitCompareMode(), TimerOutputPWM()
 *****************************************************************************/
typedef enum {
	TIMER_CC_REGISTER_0 = 0x02,
	TIMER_CC_REGISTER_1 = 0x04,
	TIMER_CC_REGISTER_2 = 0x06,
	TIMER_CC_REGISTER_3 = 0x08,
	TIMER_CC_REGISTER_4 = 0x0A,
	TIMER_CC_REGISTER_5 = 0x0C,
	TIMER_CC_REGISTER_6 = 0x0E,
} tmrCCR_t;

//*****************************************************************************
//
// The following are values that can be passed to the param parameter for
// function TimerInitCaptureMode().
//
//*****************************************************************************
typedef enum {
	TIMER_CAPTURE_ASYNCHRONOUS	= 0x00,
	TIMER_CAPTURE_SYNCHRONOUS	= SCS,
} tmrCAsynch_t;

//*****************************************************************************
//
// The following are values that can be passed to the synchronized parameter
// for function TimerGetCCBit().
//
//*****************************************************************************
typedef enum {
	TIMER_READ_SYNCHRONIZED_CAPTURE_COMPARE_INPUT	= SCCI,
	TIMER_READ_CAPTURE_COMPARE_INPUT				= CCI,
	TIMER_READ_OUTBIT								= OUT,
	TIMER_CAPTURE_OVERFLOW							= COV,
	TIMER_CC_INTERRUPT_FLAG							= CCIFG,
} tmrCCBit_t;

//*****************************************************************************
//
// The following are values that can be passed to the counterLength parameter
// for functions: TimerSelectCounterLength().
//
//*****************************************************************************
typedef enum {
	TIMER_B_COUNTER_16BIT	= CNTL_3,
	TIMER_B_COUNTER_12BIT	= CNTL_2,
	TIMER_B_COUNTER_10BIT	= CNTL_1,
	TIMER_B_COUNTER_8BIT	= CNTL_0,
} tmrBCtrBitsize_t;

//*****************************************************************************
//
// The following are values that can be passed to the groupLatch parameter for
// functions: Timer_B_selectLatchingGroup().
//
//*****************************************************************************
typedef enum {
	TIMER_B_GROUP_NONE				= TBCLGRP_0,
	TIMER_B_GROUP_CL12_CL23_CL56	= TBCLGRP_1,
	TIMER_B_GROUP_CL123_CL456		= TBCLGRP_2,
	TIMER_B_GROUP_ALL				= TBCLGRP_3,
} tmrBGroup_t;

//*****************************************************************************
//
// The following are values that can be passed to the compareLatchLoadEvent
// parameter for functions: Timer_B_initCompareLatchLoadEvent().
//
//*****************************************************************************
typedef enum {
	TIMER_B_LATCH_ON_WRITE_TO_TBxCCRn_COMPARE_REGISTER					= CLLD_0,
	TIMER_B_LATCH_WHEN_COUNTER_COUNTS_TO_0_IN_UP_OR_CONT_MODE			= CLLD_1,
	TIMER_B_LATCH_WHEN_COUNTER_COUNTS_TO_0_IN_UPDOWN_MODE				= CLLD_2,
	TIMER_B_LATCH_WHEN_COUNTER_COUNTS_TO_CURRENT_COMPARE_LATCH_VALUE	= CLLD_3,
} tmrBLatchEvent_t;

/******************************************************************************
 *!
 * \brief Used in the TimerInitUpMode() function as the param parameter.
 * 		This is used for the Up Mode and for the Up/Down Mode.
 *****************************************************************************/
typedef struct TimerInitUpModeParam {
    //! Selects the clock source (ACLK, SMCLK, external, inverted external)
	tmrClk_t clock;
    //! Is the divider for the clock source
	tmrDiv_t divider;
    //! Is the specified timer period. This is the value that gets written
    //! into the CCR0. Limited to 16 bits[uint16_t]
    uint16_t period;
    //! Enable or disable timer interrupt (enumerated ENABLE or DISABLE)
    tmrIE_t TIE;
    //! Enable or disable timer CCR0 capture compare interrupt.
    BOOL CCR0_CCIE;
    //! Decides if timer clock divider, count direction, count need to be reset
    BOOL clear;
    //! Whether to start the timer immediately  (TRUE or FALSE)
    BOOL start;
} sTmrInitUp_t;

//*****************************************************************************
//
//! \brief Used in the TimerInitContinuousMode() function as the param
//! parameter.
//
//*****************************************************************************
typedef struct TimerInitContinuousModeParam {
    //! Selects the clock source (ACLK, SMCLK, external, inverted external)
	tmrClk_t clock;
    //! Is the divider for the clock source
	tmrDiv_t divider;
    //! Enable or disable timer interrupt (enumerated ENABLE or DISABLE)
	tmrIE_t TIE;
    //! Decides if timer clock divider, count direction, count need to be reset
	BOOL clear;
    //! Whether to start the timer immediately  (TRUE or FALSE)
    BOOL start;
} sTmrInitContinuous_t;

//*****************************************************************************
//
//! \brief Used in the TimerOutputPWM() function as the param parameter.
//
//*****************************************************************************
typedef struct TimerOutputPWMParam {
    //! Selects the clock source (ACLK, SMCLK, external, inverted external)
	tmrClk_t clock;
    //! Is the divider for the clock source
	tmrDiv_t divider;
    //! Selects the desired timer period
    uint16_t period;
    //! Selects the compare register being used. Refer to datasheet to ensure
    //! the device has the capture compare register being used.
    tmrCCR_t ccRegister;
    //! Specifies the output mode.
    tmrOutMode_t outputMode;
    //! Specifies the dutycycle for the generated waveform
    uint16_t dutyCycle;
} sTmrInitPWM_t;

//*****************************************************************************
//
//! \brief Used in the TimerInitCaptureMode() function as the param
//! parameter.
//
//*****************************************************************************
typedef struct TimerInitCaptureModeParam {
    //! Selects the capture register being used. Refer to datasheet to ensure
    //! the device has the capture compare register being used.
	tmrCCR_t ccRegister;
    //! Is the capture mode selected.
	tmrCMode_t mode;
    //! Decides the Input Select
	tmrInput_t inputSelect;
    //! Decides if capture source should be synchronized with timer clock
	tmrCAsynch_t synchronizeSource;
    //! Enable or disable timer capture compare interrupt
	BOOL ccInterruptEnable;
    //! Specifies the output mode.
	tmrOutMode_t outputMode;
} sTmrInitCapture_t;

//*****************************************************************************
//
//! \brief Used in the TimerInitCompareMode() function as the param
//! parameter.
//
//*****************************************************************************
typedef struct TimerInitCompareModeParam {
    //! Selects the compare register being used. Refer to datasheet to ensure
    //! the device has the compare register being used.
	tmrCCR_t ccRegister;
    //! Enable or disable timer capture compare interrupt.
	BOOL ccInterruptEnable;
    //! Specifies the output mode.
	tmrOutMode_t outputMode;
    //! Is the count to be compared with in compare mode
    uint16_t compareValue;
} sTmrInitCompare_t;

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************

void		TimerAB_StartCounter(uint8_t timer, tmrMode_t mode);
void		TimerAB_InitContinuousMode(uint8_t timer, sTmrInitContinuous_t *config);
void		TimerAB_InitUpMode(uint8_t timer, sTmrInitUp_t *config);
void		TimerAB_InitUpDownMode(uint8_t timer, sTmrInitUp_t *config);
void		TimerAB_InitCaptureMode(uint8_t timer, sTmrInitCapture_t *config);
void		TimerAB_InitCompareMode(uint8_t timer, sTmrInitCompare_t *config);
void		TimerAB_OutputPWM(uint8_t timer, sTmrInitPWM_t *param);
void		TimerAB_EnableInterrupt(uint8_t timer);
void		TimerAB_DisableInterrupt(uint8_t timer);
BOOL		TimerAB_IsInterruptPending(uint8_t timer);
void		TimerAB_Clear(uint8_t timer);
void		TimerAB_EnableCCInterrupt(uint8_t timer, tmrCCR_t ccRegister);
void		TimerAB_DisableCCInterrupt(uint8_t timer, tmrCCR_t ccRegister);
BOOL		TimerAB_GetCCBitValue(uint8_t timer, tmrCCR_t ccRegister, tmrCCBit_t bit);
uint16_t	TimerAB_GetCCCount(uint8_t timer, tmrCCR_t ccRegister);
void		TimerAB_SetCCOutBitValue(uint8_t timer, tmrCCR_t ccRegister, BOOL bitValue);
void		TimerAB_Stop(uint8_t timer);
void		TimerAB_SetCompareValue(uint8_t timer, tmrCCR_t ccRegister, uint16_t value);
void		TimerAB_ClearTimerInterruptFlag(uint8_t timer);
void		TimerAB_ClearCCInterruptFlag(uint8_t timer, tmrCCR_t ccRegister);
uint16_t	TimerAB_GetCounterValue(uint8_t timer);

// The following functions needed to be created to interface with the HAL.
void		TimerAB_SetOutputMode(uint8_t timer, tmrCCR_t ccRegister, tmrOutMode_t outputMode);

// The following functions work for timers of type B only.
void		TimerB_SelectCounterLength(uint8_t timer, tmrBCtrBitsize_t length);
void		TimerB_SelectLatchingGroup(uint8_t timer, tmrBGroup_t groupLatch);
void		TimerB_InitCompareLatchLoadEvent(uint8_t timer, tmrCCR_t ccRegister, tmrBLatchEvent_t compareLatchLoadEvent);

#endif // MSP430_TIMER_AB__H
