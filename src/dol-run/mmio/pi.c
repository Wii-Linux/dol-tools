/*
 * DOL Runner - Emulated hardware - MMIO handling - Processor Interface
 * Copyright (C) 2025 Techflash
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../emu.h"

static struct _pi_state *state;

void E_MMIO_PI_Init(void) {
	state = &E_State.chipset.pi;
	memset(state, 0, sizeof(struct _pi_state));
}

uint32_t E_MMIO_PI_Read(uint32_t addr, int accessWidth) {
	if (accessWidth == 4) {
		switch (addr & 0x00000FFF) {
		case 0x000: /* INTSR - Interrupt cause */
			return state->intsr;
		case 0x004: /* INTMR - Interrupt mask */
			return state->intmr;
		case 0x00c: /* FIFO base start */
			return state->fifo_bs;
		case 0x010: /* FIFO base end */
			return state->fifo_be;
		case 0x014: /* FIFO current write pointer */
			return state->fifo_wp;
		case 0x02C: /* Hardware version */
			return 0x246500B1;
		default: {
			printf("FATAL: MMIO: PI: 32-bit Read from unknown register 0x%03X\n", addr & 0x00000FFF);
			E_State.fatalError = true;
			return 0;
		}
		}
	}
	else if (accessWidth == 2) {
		switch (addr & 0x00000FFF) {
		default: {
			printf("FATAL: MMIO: PI: 16-bit Read from unknown register 0x%03X\n", addr & 0x00000FFF);
			E_State.fatalError = true;
			return 0;
		}
		}
	}
	else if (accessWidth == 1) {
		switch (addr & 0x00000FFF) {
		default: {
			printf("FATAL: MMIO: PI: 8-bit Read from unknown register 0x%03X\n", addr & 0x00000FFF);
			E_State.fatalError = true;
			return 0;
		}
		}
	}
	__builtin_unreachable();
	return 0;
}

 void E_MMIO_PI_Write(uint32_t addr, uint32_t val, int accessWidth) {
	if (accessWidth == 4) {
		switch (addr & 0x00000FFF) {
		case 0x000: { /* INTSR - Interrupt cause */
			state->intsr = val;
			break;
		}
		case 0x004: { /* INTMR - Interrupt mask */
			state->intmr = val;
			break;
		}
		case 0x00c: { /* FIFO base start */
			state->fifo_bs = val;
			break;
		}
		case 0x010: { /* FIFO base end */
			state->fifo_be = val;
			break;
		}
		case 0x014: { /* FIFO current write pointer */
			state->fifo_wp = val;
			break;
		}
		case 0x02C: { /* Hardware version */
			printf("WARN: MMIO: PI: Probably unsupported write (0x%08X) to hardware version register!\n", val);
			break;
		}
		default: {
			printf("FATAL: MMIO: PI: 32-bit Write (0x%08X) to unknown register 0x%03X\n", val, addr & 0x00000FFF);
			E_State.fatalError = true;
			return;
		}
		}
	}
	else if (accessWidth == 2) {
		val &= 0x0000FFFF;
		switch (addr & 0x00000FFF) {
		default: {
			printf("FATAL: MMIO: PI: 16-bit Write (0x%04X) to unknown register 0x%03X\n", val & 0x0000FFFF, addr & 0x00000FFF);
			E_State.fatalError = true;
			return;
		}
		}
	}
	else if (accessWidth == 1) {
		val &= 0x000000FF;
		switch (addr & 0x00000FFF) {
		default: {
			printf("FATAL: MMIO: PI: 8-bit Write (0x%02X) to unknown register 0x%03X\n", val & 0x000000FF, addr & 0x00000FFF);
			E_State.fatalError = true;
			return;
		}
		}
	}
	__builtin_unreachable();
	return;
}
