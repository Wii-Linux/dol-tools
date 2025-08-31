/*
 * DOL Runner - Emulated hardware - MMIO handling - Memory Interface
 * Copyright (C) 2025 Techflash
 */

#ifndef _DOLTOOLS_DOLRUN_MMIO_MI_H
#define _DOLTOOLS_DOLRUN_MMIO_MI_H

#include <stdint.h>

struct _mi_state {
	uint32_t protectedRegion[4];
	uint16_t protType;
	uint16_t intMask;
	uint16_t intCause;
	uint16_t addrlo;
	uint16_t addrhi;
	uint16_t unk_20;
};

extern void E_MMIO_MI_Init(void);
extern uint32_t E_MMIO_MI_Read(uint32_t addr, int accessWidth);
extern void E_MMIO_MI_Write(uint32_t addr, uint32_t val, int accessWidth);

#endif
