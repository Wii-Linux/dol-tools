/*
 * DOL Runner - Emulated hardware - MMIO handling - Audio Interface
 * Copyright (C) 2025 Techflash
 */

#ifndef _DOLTOOLS_DOLRUN_MMIO_AI_H
#define _DOLTOOLS_DOLRUN_MMIO_AI_H

#include <stdint.h>

struct _ai_state {
	uint32_t aicr;
};

extern void E_MMIO_AI_Init(void);
extern uint32_t E_MMIO_AI_Read(uint32_t addr, int accessWidth);
extern void E_MMIO_AI_Write(uint32_t addr, uint32_t val, int accessWidth);

#endif
