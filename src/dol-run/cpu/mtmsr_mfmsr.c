/*
 * DOL Runner - Emulated hardware - CPU handling - 'mtmsr' / 'mfmsr' instruction
 * Copyright (C) 2025 Techflash
 */

#include <stdio.h>
#include "common.h"


void E_PPC_Emulate_mtmsr(uint32_t val) {
	E_State.cpu.msr = val;
	E_PPC_Validate_MSR();
	return;
}

uint32_t E_PPC_Emulate_mfmsr(void) {
	return E_State.cpu.msr;
}
