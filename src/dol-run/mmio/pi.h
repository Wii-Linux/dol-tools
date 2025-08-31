/*
 * DOL Runner - Emulated hardware - MMIO handling - Processor Interface
 * Copyright (C) 2025 Techflash
 */

#ifndef _DOLTOOLS_DOLRUN_MMIO_PI_H
#define _DOLTOOLS_DOLRUN_MMIO_PI_H

#include <stdint.h>

struct _pi_state {
	uint32_t intsr;
	uint32_t intmr;
	uint32_t fifo_bs;
	uint32_t fifo_be;
	uint32_t fifo_wp;
};

extern void E_MMIO_PI_Init(void);
extern uint32_t E_MMIO_PI_Read(uint32_t addr, int accessWidth);
extern void E_MMIO_PI_Write(uint32_t addr, uint32_t val, int accessWidth);

#endif
