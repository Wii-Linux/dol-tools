/*
 * DOL Runner - Emulated hardware - MMIO handling
 * Copyright (C) 2025 Techflash
 */

#ifndef _DOLTOOLS_DOLRUN_MMIO_H
#define _DOLTOOLS_DOLRUN_MMIO_H

#include <stdint.h>

/*
 * Bring up all relevant MMIO functionality
 */
extern int E_MMIO_Chipset_Init(void);

/*
 * Handle Flipper MMIO - Read
 */
extern uint32_t E_MMIO_Flipper_Read(uint32_t addr, int accessWidth);

/*
 * Handle Flipper MMIO - Write
 */
extern void E_MMIO_Flipper_Write(uint32_t addr, uint32_t val, int accessWidth);


/*
 * Handle Hollywood MMIO - Read
 */
extern uint32_t E_MMIO_Hollywood_Read(uint32_t addr, int accessWidth);

/*
 * Handle Hollywood MMIO - Write
 */
extern void E_MMIO_Hollywood_Write(uint32_t addr, uint32_t val, int accessWidth);

#endif
