/*
 * DOL Runner - Emulated hardware - MMIO handling - Video Interface
 * Copyright (C) 2025 Techflash
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "vi.h"
#include "../timer.h"
#include "../emu.h"

static void E_MMIO_VI_TimerHook(void) {
	puts("TimerHook: Hello from the VI timer hook");
	/* TODO: Do something here */
}

void E_MMIO_VI_Init(void) {
	memset(&E_State.chipset.vi, 0, sizeof(struct _vi_state));
	E_Timer_AddHook(E_MMIO_VI_TimerHook);
}

uint32_t E_MMIO_VI_Read(uint32_t addr, int accessWidth) {
	if (accessWidth == 4) {
		switch (addr & 0x00000FFF) {
		default: {
			printf("FATAL: MMIO: VI: 32-bit Read from unknown register 0x%03X\n", addr & 0x00000FFF);
			E_State.fatalError = true;
			return 0;
		}
		}
	}
	else if (accessWidth == 2) {
		switch (addr & 0x00000FFF) {
		case 0x06C:
			return E_State.chipset.vi.viclk;
		default: {
			printf("FATAL: MMIO: VI: 16-bit Read from unknown register 0x%03X\n", addr & 0x00000FFF);
			E_State.fatalError = true;
			return 0;
		}
		}
	}
	else if (accessWidth == 1) {
		switch (addr & 0x00000FFF) {
		default: {
			printf("FATAL: MMIO: VI: 8-bit Read from unknown register 0x%03X\n", addr & 0x00000FFF);
			E_State.fatalError = true;
			return 0;
		}
		}
	}
	__builtin_unreachable();
	return 0;
}

void E_MMIO_VI_Write(uint32_t addr, uint32_t val, int accessWidth) {
	if (accessWidth == 4) {
		switch (addr & 0x00000FFF) {
		default: {
			printf("FATAL: MMIO: VI: 32-bit Write (0x%08X) to unknown register 0x%02X\n", val, addr & 0x00000FFF);
			E_State.fatalError = true;
			return;
		}
		}
	}
	else if (accessWidth == 2) {
		val &= 0x0000FFFF;
		switch (addr & 0x00000FFF) {
		default: {
			printf("FATAL: MMIO: VI: 16-bit Write (0x%04X) to unknown register 0x%02X\n", val & 0x0000FFFF, addr & 0x00000FFF);
			E_State.fatalError = true;
			return;
		}
		}
	}
	else if (accessWidth == 1) {
		val &= 0x000000FF;
		switch (addr & 0x00000FFF) {
		default: {
			printf("FATAL: MMIO: VI: 8-bit Write (0x%02X) to unknown register 0x%02X\n", val & 0x000000FF, addr & 0x00000FFF);
			E_State.fatalError = true;
			return;
		}
		}
	}
	__builtin_unreachable();
	return;
}
