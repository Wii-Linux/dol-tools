/*
 * DOL Runner - Emulated hardware - CPU handling - 'rfi' instruction
 * Copyright (C) 2025 Techflash
 */
#include "common.h"

uint32_t E_PPC_Emulate_rfi(void) {
	/* according to the PowerPC 750CL user manual, exactly these bits should be transferred */
	uint32_t mask = 0x87C0FF73;
	E_State.cpu.msr = (E_State.cpu.msr & ~mask) | (E_State.cpu.srr1 & mask);
	E_State.cpu.msr &= ~MSR_POW; /* clear bit 13 */

	/* we likely need to update our memory map after this, since it affects MSR */
	E_State.needsMemMapUpdate = true;

	E_PPC_Validate_MSR();

	/* clear the lowest 2 bits */
	return (E_State.cpu.srr0 & ~3);
}
