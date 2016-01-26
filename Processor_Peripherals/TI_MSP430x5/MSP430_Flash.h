//*****************************************************************************
//
// FlashMem.h - Driver for the FLASHCTL Module.
//
//*****************************************************************************

#ifndef __MSP430WARE_FLASHMEM_H__
#define __MSP430WARE_FLASHMEM_H__

#ifdef __MSP430_HAS_FLASH__				// Include this only if we have flash.

//*****************************************************************************
//
// The following are values that can be passed to the mask parameter for
// functions: FlashCtl_status() as well as returned by the FlashCtl_status()
// function.
//
//*****************************************************************************
#define FLASHCTL_READY_FOR_NEXT_WRITE				WAIT
#define FLASHCTL_ACCESS_VIOLATION_INTERRUPT_FLAG	ACCVIFG
#define FLASHCTL_PASSWORD_WRITTEN_INCORRECTLY		KEYV
#define FLASHCTL_BUSY								BUSY

#define MEM_ADDR_INFO_D		0x001800	// 128-byte Information Segment D
#define MEM_ADDR_INFO_C		0x001880	// 128-byte Information Segment C
#define MEM_ADDR_INFO_B		0x001900	// 128-byte Information Segment B
#define MEM_ADDR_INFO_A		0x001980	// 128-byte Information Segment A
#define MEM_ADDR_BANK_A		0x008000	// 128-kbyte Memory Bank A
#define MEM_ADDR_BANK_B		0x028000	// 128-kbyte Memory Bank B
#define MEM_ADDR_BANK_C		0x048000	// 128-kbyte Memory Bank C
#define MEM_ADDR_BANK_D		0x068000	// 128-kbyte Memory Bank D

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************

void FlashMemSegmentErase(uint8_t *flashPtr);
void FlashMemBankErase(uint8_t *flashPtr);
void FlashMemMassErase(uint8_t *flashPtr);
BOOL FlashMemEraseCheck(uint8_t *flashPtr, uint16_t count);
void FlashMemWrite8(uint8_t *dataPtr, uint8_t *flashPtr, uint16_t count);
void FlashMemWrite16(uint16_t *dataPtr, uint16_t *flashPtr, uint16_t count);
void FlashMemWrite32(uint32_t *dataPtr, uint32_t *flashPtr, uint16_t count);
uint8_t FlashMemStatus(uint8_t mask);
void FlashMemLockInfoA(void);
void FlashMemUnlockInfoA(void);

#endif
#endif // __MSP430WARE_FLASHMEM_H__
