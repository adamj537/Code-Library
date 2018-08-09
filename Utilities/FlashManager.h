/******************************************************************************
 * @file	FlashManager.h
 * @author	Adam Johnson
 * @remarks	Contains functions for EEPROM emulation using two pages of
 *			Flash memory.  Based on code from NXP's app note AN11008,
 *			titled "Flash based non-volatile storage."  Originally
 *			developed for MSP430.
 *****************************************************************************/

#ifndef MEM_MANAGER_H
#define MEM_MANAGER_H

// Initialize access to non-volatile memory.
bool FlashManInit(void);

// Set the value of a variable.
bool FlashManSetVariable(uint16_t id, uint8_t *value, uint16_t size);

// Get the value of a variable.
bool FlashManGetVariable(uint16_t id, uint8_t *value, uint16_t size);

// Get the number of variables Flash will hold.
uint32_t FlashManGetMaxVariables(void);

#endif
