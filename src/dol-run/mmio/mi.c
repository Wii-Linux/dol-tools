/*
 * DOL Runner - Emulated hardware - MMIO handling - Memory Interface
 * Copyright (C) 2025 Techflash
 */
#include "mi.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../emu.h"

static struct _mi_state *state;

void E_MMIO_MI_Init(void) {
	state = &E_State.chipset.mi;
	memset(state, 0, sizeof(struct _mi_state));
}

uint32_t E_MMIO_MI_Read(uint32_t addr, int accessWidth) {
	if (accessWidth == 4) {
		switch (addr & 0x000000FF) {
		default: {
			printf("FATAL: MMIO: MI: 32-bit Read from unknown register 0x%02X\n", addr & 0x000000FF);
			E_State.fatalError = true;
			return 0;
		}
		}
	}
	else if (accessWidth == 2) {
		switch (addr & 0x000000FF) {
		default: {
			case 0x10:
				return state->protType;
			case 0x1C:
				return state->intMask;
			case 0x20:
				return state->unk_20;
			printf("FATAL: MMIO: MI: 16-bit Read from unknown register 0x%02X\n", addr & 0x000000FF);
			E_State.fatalError = true;
			return 0;
		}
		}
	}
	else if (accessWidth == 1) {
		switch (addr & 0x000000FF) {
		default: {
			printf("FATAL: MMIO: MI: 8-bit Read from unknown register 0x%02X\n", addr & 0x000000FF);
			E_State.fatalError = true;
			return 0;
		}
		}
	}
	__builtin_unreachable();
	return 0;
}

void E_MMIO_MI_Write(uint32_t addr, uint32_t val, int accessWidth) {
	if (accessWidth == 4) {
		switch (addr & 0x000000FF) {
		default: {
			printf("FATAL: MMIO: MI: 32-bit Write (0x%08X) to unknown register 0x%02X\n", val, addr & 0x000000FF);
			E_State.fatalError = true;
			return;
		}
		}
	}
	else if (accessWidth == 2) {
		val &= 0x0000FFFF;
		switch (addr & 0x000000FF) {
		case 0x10: {
			state->protType = val;
			break;
		}
		case 0x1C: {
			state->intMask = val;
			break;
		}
		case 0x20: {
			state->unk_20 = val;
			break;
		}
		default: {
			printf("FATAL: MMIO: MI: 16-bit Write (0x%04X) to unknown register 0x%02X\n", val & 0x0000FFFF, addr & 0x000000FF);
			E_State.fatalError = true;
			return;
		}
		}
	}
	else if (accessWidth == 1) {
		val &= 0x000000FF;
		switch (addr & 0x000000FF) {
		default: {
			printf("FATAL: MMIO: MI: 8-bit Write (0x%02X) to unknown register 0x%02X\n", val & 0x000000FF, addr & 0x000000FF);
			E_State.fatalError = true;
			return;
		}
		}
	}
	__builtin_unreachable();
	return;
}
