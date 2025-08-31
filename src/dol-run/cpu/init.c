/*
 * DOL Runner - Emulated hardware - CPU handling - Initialization
 * Copyright (C) 2025 Techflash
 */
#include "../cpu.h"
#include "../emu.h"
#include "../timer.h"


static void E_CPU_DecHook(void) {
	/* magic values, exalained:
	 * These are to calculate *roughly* how many decrementer
	 * ticks would have passed throughout the period of time
	 * that has elapsed between timer ticks.
	 *
	 * 1944000 = (486000000 [1] ÷ 4) × (0.016 [2])
	 * [1] - Gekko clock speed
	 * [2] - Timer target
	 *
	 * 2916000 = (729000000 [1] ÷ 4) × (0.016 [2])
	 * [1] - Broadway clock speed
	 * [2] - Timer target
	 */

	if (E_State.consoleType == CONSOLE_TYPE_GAMECUBE) {
		if (E_State.cpu.decrementer < 1944000) {
			E_State.cpu.decrementer = 0;
			/* TODO: fire interrupt */
		}
		else
			E_State.cpu.decrementer -= 1944000;
	}
	else if (E_State.consoleType == CONSOLE_TYPE_WII) {
		if (E_State.cpu.decrementer < 2916000) {
			E_State.cpu.decrementer = 0;
			/* TODO: fire interrupt */
		}
		else
			E_State.cpu.decrementer -= 2916000;
	}
}

void E_CPU_Init(void) {
	int i;
	E_State.cpu.hid0 = 0;
	E_State.cpu.hid1 = 0;
	E_State.cpu.hid2 = 0;
	E_State.cpu.hid4 = 0 | HID4_RSRVD1;
	E_State.cpu.srr0 = 0;
	E_State.cpu.srr1 = 0;
	E_State.cpu.l2cr = 0;
	E_State.cpu.mmcr0 = 0;
	E_State.cpu.mmcr1 = 0;
	for (i = 0; i < 4; i++) {
		E_State.cpu.pmc[i] = 0;
		E_State.cpu.sprg[i] = 0;
	}
	E_Timer_AddHook(E_CPU_DecHook);
}
