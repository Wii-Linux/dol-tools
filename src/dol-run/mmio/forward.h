/*
 * DOL Runner - Emulated hardware - MMIO handling - Forwarding support
 * Copyright (C) 2025 Techflash
 */

#ifndef _DOLTOOLS_DOLRUN_MMIO_FORWARD_H
#define _DOLTOOLS_DOLRUN_MMIO_FORWARD_H

#include <stdint.h>

/*
 * Initialize MMIO forwarding support.
 * E_State.host must be set.
 */
extern int E_MMIO_Forwarding_Init(void);

/*
 * Clean up forwarding
 */
extern void E_MMIO_Forwarding_Cleanup(void);

/*
 * Perform a forwarded read.
 */
extern uint32_t E_MMIO_Forwarded_Read(uint32_t addr, int accessWidth);


/*
 * Perform a forwarded write.
 */
extern void E_MMIO_Forwarded_Write(uint32_t addr, uint32_t val, int accessWidth);

#endif
