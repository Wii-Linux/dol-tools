/*
 * DOL Runner - Emulated hardware - MMIO handling - Hollywood
 * Copyright (C) 2025 Techflash
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "../emu.h"
#include "forward.h"
#include "ai.h"

uint32_t E_MMIO_Hollywood_Read(uint32_t addr, int accessWidth) {
	switch (addr & 0xFFFFF000) {
	case 0x0D006000: { /* Multiple things */
		if ((addr & 0x0000FF00) == 0x00006000) { /* DI */
			printf("FATAL: MMIO: Trying to read from DI register 0x%02X - this is unsupported\n", addr & 0x000000FF);
			E_State.fatalError = true;
			break;
		}

		else if ((addr & 0x0000FF00) == 0x00006400) /* SI */
			return E_MMIO_Forwarded_Read(addr, accessWidth);

		else if ((addr & 0x0000FF00) == 0x00006800) /* EXI */
			return E_MMIO_Forwarded_Read(addr, accessWidth);

		else if ((addr & 0x0000FF00) == 0x00006C00) /* AI */
			return E_MMIO_AI_Read(addr, accessWidth);

		break;
	}
	default: {
		printf("FATAL: MMIO: Hollywood: Trying to read from unimplemented addr: 0x%08X\n", addr);
		E_State.fatalError = true;
		return 0;
	}
	}
	return 0;
}

void E_MMIO_Hollywood_Write(uint32_t addr, uint32_t val, int accessWidth) {
	switch (addr & 0xFFFFF000) {
	case 0x0D006000: /* Multiple things */ {
		if ((addr & 0x0000FF00) == 0x00006000) { /* DI */
			printf("FATAL: MMIO: Trying to write to DI register 0x%02X - this is unsupported\n", addr & 0x000000FF);
			E_State.fatalError = true;
			break;
		}

		else if ((addr & 0x0000FF00) == 0x00006400) { /* SI */
			E_MMIO_Forwarded_Write(addr, val, accessWidth);
			break;
		}

		else if ((addr & 0x0000FF00) == 0x00006800) { /* EXI */
			E_MMIO_Forwarded_Write(addr, val, accessWidth);
			break;
		}

		else if ((addr & 0x0000FF00) == 0x00006C00) { /* AI */
			E_MMIO_AI_Write(addr, val, accessWidth);
			break;
		}

		break;
	}

	default: {
		printf("FATAL: MMIO: Hollywood: Trying to write 0x%08X to unimplemented addr: 0x%08X\n", val, addr);
		E_State.fatalError = true;
		return;
	}
	}
}
