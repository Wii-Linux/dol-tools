/*
 * DOL Runner - Emulated hardware - MMIO handling - Hollywood IPC
 * Copyright (C) 2025 Techflash
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "hlwd-ipc.h"
#include "../emu.h"

static struct _ipc_state *state;

void E_MMIO_IPC_Init(void) {
	state = &E_State.chipset.ipc;
	memset(state, 0, sizeof(struct _ipc_state));
}

uint32_t E_MMIO_IPC_Read(uint32_t addr, int accessWidth) {
	if (accessWidth == 4) {
		switch (addr & 0x000000FF) {
		case 0x00:
			return state->ppcmsg;
		case 0x04:
			return state->ppcctrl;
		case 0x08:
			return state->armmsg;
		case 0x0C: {
			puts("WARN: MMIO: IPC: PPC Read from ARMCTRL");
			return 0;
		}
		default: {
			printf("FATAL: MMIO: IPC: 32-bit Read from unknown register 0x%02X\n", addr & 0x000000FF);
			E_State.fatalError = true;
			return 0;
		}
		}
	}
	else if (accessWidth == 2) {
		switch (addr & 0x000000FF) {
		default: {
			printf("FATAL: MMIO: IPC: 16-bit Read from unknown register 0x%02X\n", addr & 0x000000FF);
			E_State.fatalError = true;
			return 0;
		}
		}
	}
	else if (accessWidth == 1) {
		switch (addr & 0x000000FF) {
		default: {
			printf("FATAL: MMIO: IPC: 8-bit Read from unknown register 0x%02X\n", addr & 0x000000FF);
			E_State.fatalError = true;
			return 0;
		}
		}
	}
	__builtin_unreachable();
	return 0;
}

void E_MMIO_IPC_Write(uint32_t addr, uint32_t val, int accessWidth) {
	if (accessWidth == 4) {
		switch (addr & 0x000000FF) {
		case 0x00: {
			state->ppcmsg = val;
			break;
		}
		case 0x04: {
			state->ppcctrl = val;
			/*E_MMIO_IPC_FlagsUpdated();*/
			break;
		}
		case 0x08: {
			state->armmsg = val;
			break;
		}
		case 0x0C: {
			printf("WARN: MMIO: IPC: Ignoring PPC Write (0x%08X) to ARMCTRL\n", val);
			break;
		}
		default: {
			printf("FATAL: MMIO: IPC: 32-bit Write (0x%08X) to unknown register 0x%02X\n", val, addr & 0x000000FF);
			E_State.fatalError = true;
			return;
		}
		}
	}
	else if (accessWidth == 2) {
		val &= 0x00000FFF;
		switch (addr & 0x000000FF) {
		default: {
			printf("FATAL: MMIO: IPC: 16-bit Write (0x%04X) to unknown register 0x%02X\n", val & 0x0000FFFF, addr & 0x000000FF);
			E_State.fatalError = true;
			return;
		}
		}
	}
	else if (accessWidth == 1) {
		val &= 0x000000FF;
		switch (addr & 0x000000FF) {
		default: {
			printf("FATAL: MMIO: IPC: 8-bit Write (0x%02X) to unknown register 0x%02X\n", val & 0x000000FF, addr & 0x000000FF);
			E_State.fatalError = true;
			return;
		}
		}
	}
	__builtin_unreachable();
	return;
}
