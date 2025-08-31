/*
 * DOL Runner - Emulated hardware - MMIO handling - Audio DSP
 * Copyright (C) 2025 Techflash
 */

#ifndef _DOLTOOLS_DOLRUN_MMIO_DSP_H
#define _DOLTOOLS_DOLRUN_MMIO_DSP_H

#include <stdbool.h>
#include <stdint.h>

#define DSP_CSR_BOOTMODE (1 << 11)
#define DSP_CSR_HALT (1 << 2)
#define DSP_CSR_RESET (1 << 0)

struct _dsp_state {
	uint32_t priv_dmaSrcPtr;
	uint32_t priv_dmaDestPtr;
	bool priv_dmaDir;
	int priv_resetTimer;
	bool priv_halt;

	uint16_t *iram;
	uint16_t *dram;
	uint16_t *irom;
	uint16_t *drom;

	uint16_t inMboxH;
	uint16_t inMboxL;
	uint16_t outMboxH;
	uint16_t outMboxL;
	uint16_t csr;
	uint16_t arSize;
	uint32_t dmaMMAddr;
	uint32_t dmaARAddr;
	uint32_t dmaSize;
};

extern void E_MMIO_DSP_Init(void);
extern uint32_t E_MMIO_DSP_Read(uint32_t addr, int accessWidth);
extern void E_MMIO_DSP_Write(uint32_t addr, uint32_t val, int accessWidth);

#endif
