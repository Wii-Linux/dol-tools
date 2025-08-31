/*
 * DOL Runner - Emulated hardware - CPU handling - 'mtsr' / 'mfsr' instruction
 * Copyright (C) 2025 Techflash
 */
#include "common.h"

void E_PPC_Emulate_mtsr(uint32_t reg, uint32_t val) {
	E_State.cpu.segmentRegs[reg] = val;
	return;
}

uint32_t E_PPC_Emulate_mfsr(uint32_t reg) {
	return E_State.cpu.segmentRegs[reg];
}
