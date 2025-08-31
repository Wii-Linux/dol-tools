/*
 * DOL Runner - Emulated hardware - MMIO handling - Video Interface
 * Copyright (C) 2025 Techflash
 */

#ifndef _DOLTOOLS_DOLRUN_MMIO_VI_H
#define _DOLTOOLS_DOLRUN_MMIO_VI_H

#include <stdint.h>

struct _vi_state {
	uint16_t vtr;
	uint16_t dcr;
	uint32_t htr0;
	uint32_t htr1;
	uint32_t vto;
	uint32_t vte;
	uint32_t bbei;
	uint32_t bboi;
	uint32_t tfbl;
	uint32_t tfbr;
	uint32_t bfbl;
	uint32_t bfbr;
	uint16_t dpv;
	uint16_t dph;
	uint32_t dispInt[4];
	uint32_t dl0;
	uint32_t dl1;
	uint16_t hsw;
	uint16_t hsr;
	uint32_t fct[7];
	uint16_t viclk;
	uint16_t visel;
	uint16_t hbe;
	uint16_t hbs;
};

extern void E_MMIO_VI_Init(void);
extern uint32_t E_MMIO_VI_Read(uint32_t addr, int accessWidth);
extern void E_MMIO_VI_Write(uint32_t addr, uint32_t val, int accessWidth);
#endif
