/*
 * DOL Runner - Emulated hardware - CPU handling - Common
 * Copyright (C) 2025 Techflash
 */
#include "common.h"
#include <stdio.h>

void E_PPC_Validate_MSR(void) {
	uint32_t val, msr;

	msr = E_State.cpu.msr;

	/* Is MSR[POW] set, and any of the low-power mode settings are enabled? */
	val = E_State.cpu.hid0;
	if (((val & HID0_DOZE) != 0 || (val & HID0_NAP) != 0 || (val & HID0_SLEEP) != 0) &&
		(msr & MSR_POW) != 0 ) {
		puts("FATAL: PPC: Attempted to enter low power (Doze/Nap/Sleep) state with MSR[POW] active");
		E_State.fatalError = true;
		return;
	}

	/* Does MSR[DR] == MSR[IR]? */
	if (((msr & MSR_IR) >> MSR_IR_SHIFT) != ((msr & MSR_DR) >> MSR_DR_SHIFT)) {
		puts("FATAL: PPC: Attempted to set only MSR[IR] or only MSR[DR] - they must both be on, or both be off");
		E_State.fatalError = true;
		return;
	}

	return;
}

int E_PPC_Validate_HighBATAccess(char *op, char type) {
	/* BATs>=4 are illegal on Gekko */
	if (E_State.consoleType == CONSOLE_TYPE_GAMECUBE) {
		printf("FATAL: PPC: Attempt to %s to higher %cBATs on Gekko (only supported on Broadway)\n", op, type);
		E_State.fatalError = true;
		return 1;
	}

	/* BATs>=4 are illegal on Broadway if it doesn't have SBE set in HID4 */
	if (E_State.consoleType == CONSOLE_TYPE_WII && (E_State.cpu.hid4 & HID4_SBE) == 0) {
		printf("FATAL: PPC: Attempt to %s to higher %cBATs but HID4[SBE] is not set\n", op, type);
		E_State.fatalError = true;
		return 1;
	}

	return 0;
}
