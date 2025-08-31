/*
 * DOL Runner - Emulated hardware - MMIO handling - Audio DSP
 * Copyright (C) 2025 Techflash
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "dsp.h"
#include "../timer.h"
#include "../emu.h"

static void E_MMIO_DSP_DMAStart(void) {
	uint32_t dmaDir = (E_State.chipset.dsp.dmaSize & 0x80000000) >> 31;
	E_State.chipset.dsp.priv_dmaDir = (bool)dmaDir;

	if (!dmaDir) {
		/* 0 = Write to ARAM */
		E_State.chipset.dsp.priv_dmaSrcPtr = E_State.chipset.dsp.dmaMMAddr;
		E_State.chipset.dsp.priv_dmaDestPtr = E_State.chipset.dsp.dmaARAddr;
	}
	else {
		/* 1 = Read from ARAM */
		E_State.chipset.dsp.priv_dmaSrcPtr = E_State.chipset.dsp.dmaARAddr;
		E_State.chipset.dsp.priv_dmaDestPtr = E_State.chipset.dsp.dmaMMAddr;
	}
}

/* TODO: Implement these */
static void E_MMIO_DSP_MboxPush_H(uint16_t val) {

}


static void E_MMIO_DSP_MboxPush_L(uint16_t val) {

}

static void E_MMIO_DSP_TimerHook(void) {
	memAddr_t virt;
	uint32_t dmaSize;
	puts("TimerHook: Hello from the DSP timer hook");

	/* are we in reset?  it progresses even during halt */
	if (E_State.chipset.dsp.priv_resetTimer == 2) {
		puts("MMIO: DSP: DSP in reset stage 2");
		E_State.chipset.dsp.priv_resetTimer--;
		if ((E_State.chipset.dsp.csr & DSP_CSR_BOOTMODE) == 0) {
			puts("MMIO: DSP: Pulling from memory to seed IRAM");
			virt = E_MemPhysToVirt(0x01000000, true);
			if (!virt.mapped) {
				puts("FATAL: MMIO: DSP: MEM1 isn't mapped ?!");
				E_State.fatalError = true;
				return;
			}
			memcpy(E_State.chipset.dsp.iram, (const void *)virt.addr, 1024);
			return;
		}
	}

	else if (E_State.chipset.dsp.priv_resetTimer == 1) {
		puts("MMIO: DSP: DSP is about to finish resetting (stage 1)");
		E_State.chipset.dsp.csr &= ~DSP_CSR_RESET;

		E_State.chipset.dsp.priv_resetTimer--;
		puts("MMIO: DSP: Finished DSP reset");
	}

	/* are we halted? */
	if (E_State.chipset.dsp.priv_halt)
		return;


	/* TODO: Actually start doing more stuff here */
	dmaSize = E_State.chipset.dsp.dmaSize & 0x7FFFFFFF;
	if (dmaSize & 0x1F) {
		E_State.fatalError = true;
		puts("FATAL: MMIO: DSP: Trying to do unaligned DMA");
		return;
	}

	if (dmaSize == 0)
		goto noDMA;

	/* do a block of DMA */
	if (!E_State.chipset.dsp.priv_dmaDir)
		printf("MMIO: DSP: Would do 32 bytes of DSP, from 0x%08X (ARAM) to 0x%08X (Mem)\n", E_State.chipset.dsp.priv_dmaSrcPtr, E_State.chipset.dsp.priv_dmaDestPtr);
	else
		printf("MMIO: DSP: Would do 32 bytes of DSP, from 0x%08X (Mem) to 0x%08X (ARAM)\n", E_State.chipset.dsp.priv_dmaSrcPtr, E_State.chipset.dsp.priv_dmaDestPtr);
	E_State.chipset.dsp.priv_dmaDestPtr += 32;
	E_State.chipset.dsp.priv_dmaSrcPtr += 32;
	E_State.chipset.dsp.dmaSize -= 32;

noDMA:
	return;
}

void E_MMIO_DSP_Init(void) {
	memset(&E_State.chipset.dsp, 0, sizeof(struct _dsp_state));
	E_State.chipset.dsp.iram = malloc(8 * 1024);
	E_State.chipset.dsp.dram = malloc(8 * 1024);
	E_State.chipset.dsp.irom = malloc(8 * 1024);
	E_State.chipset.dsp.drom = malloc(4 * 1024);
	E_State.chipset.dsp.csr  = 0x0816; /* at least, this is the start that Linux has it in at idle */
	E_Timer_AddHook(E_MMIO_DSP_TimerHook);
}

void E_MMIO_DSP_Cleanup(void) {
	free(E_State.chipset.dsp.iram);
	free(E_State.chipset.dsp.dram);
	free(E_State.chipset.dsp.irom);
	free(E_State.chipset.dsp.drom);
	E_State.chipset.dsp.priv_halt = true;
}

uint32_t E_MMIO_DSP_Read(uint32_t addr, int accessWidth) {
	if (accessWidth == 4) {
		switch (addr & 0x00000FFF) {
		case 0x020: /* DSP_AR_DMA_MMADDR */
			return E_State.chipset.dsp.dmaMMAddr;
		case 0x024: /* DSP_AR_DMA_ARADDR */
			return E_State.chipset.dsp.dmaARAddr;
		case 0x028: /* DSP_AR_DMA_SIZE */
			return E_State.chipset.dsp.dmaSize;
		default: {
			printf("FATAL: MMIO: DSP: 32-bit Read from unknown register 0x%03X\n", addr & 0x00000FFF);
			E_State.fatalError = true;
			return 0;
		}
		}
	}
	else if (accessWidth == 2) {
		switch (addr & 0x00000FFF) {
		case 0x000: /* mailbox in H */
			return E_State.chipset.dsp.inMboxH;
		case 0x002: /* mailbox in L */
			return E_State.chipset.dsp.inMboxL;
		case 0x004: /* mailbox out H */
			return E_State.chipset.dsp.outMboxH;
		case 0x006: /* mailbox out L */
			return E_State.chipset.dsp.outMboxL;
		case 0x00A: /* CSR */
			return E_State.chipset.dsp.csr;
		case 0x012: /* AR_SIZE */
			return E_State.chipset.dsp.arSize;
		case 0x020: /* DSP_AR_DMA_MMADDR (High) */
		    return (E_State.chipset.dsp.dmaMMAddr & 0xFFFF0000) >> 16;
		case 0x022: /* DSP_AR_DMA_MMADDR (Low) */
		    return E_State.chipset.dsp.dmaMMAddr & 0x0000FFFF;
		case 0x024: /* DSP_AR_DMA_ARADDR (High) */
			return (E_State.chipset.dsp.dmaMMAddr & 0xFFFF0000) >> 16;
		case 0x026: /* DSP_AR_DMA_ARADDR (Low) */
			return E_State.chipset.dsp.dmaARAddr & 0x0000FFFF;
		case 0x028: /* DSP_AR_DMA_SIZE (High) */
			return (E_State.chipset.dsp.dmaSize & 0xFFFF0000) >> 16;
		case 0x02A: /* DSP_AR_DMA_SIZE (Low) */
			return E_State.chipset.dsp.dmaSize & 0x0000FFFF;
		default: {
			printf("FATAL: MMIO: DSP: 16-bit Read from unknown register 0x%03X\n", addr & 0x00000FFF);
			E_State.fatalError = true;
			return 0;
		}
		}
	}
	else if (accessWidth == 1) {
		switch (addr & 0x00000FFF) {
		default: {
			printf("FATAL: MMIO: DSP: 8-bit Read from unknown register 0x%03X\n", addr & 0x00000FFF);
			E_State.fatalError = true;
			return 0;
		}
		}
	}
	__builtin_unreachable();
	return 0;
}

void E_MMIO_DSP_Write(uint32_t addr, uint32_t val, int accessWidth) {
	if (accessWidth == 4) {
		switch (addr & 0x00000FFF) {
		case 0x020: /* AR_DMA_MMADDR */ {
			E_State.chipset.dsp.dmaMMAddr = val;
			break;
		}
		case 0x024: /* AR_DMA_ARADDR */ {
			E_State.chipset.dsp.dmaARAddr = val;
			break;
		}
		case 0x028: /* AR_DMA_SIZE */ {
			E_State.chipset.dsp.dmaSize = val;
			E_MMIO_DSP_DMAStart();
			break;
		}
		default: {
			printf("FATAL: MMIO: DSP: 32-bit Write (0x%08X) to unknown register 0x%03X\n", val, addr & 0x00000FFF);
			E_State.fatalError = true;
			return;
		}
		}
	}
	else if (accessWidth == 2) {
		val &= 0x0000FFFF;
		switch (addr & 0x00000FFF) {
		case 0x000: /* mailbox in H */ {
			E_State.chipset.dsp.inMboxH = val;
			if (val & 0x00008000)
				E_MMIO_DSP_MboxPush_H(val);
			break;
		}
		case 0x002: /* mailbox in L */ {
			E_State.chipset.dsp.inMboxL = val;
			if (E_State.chipset.dsp.inMboxH & 0x00008000)
				E_MMIO_DSP_MboxPush_L(val);
			break;
		}
		case 0x004: /* mailbox out H */ {
			if (val & 0x00008000)
				E_State.chipset.dsp.outMboxH |= 0x00008000;
			else
				E_State.chipset.dsp.outMboxH &= ~0x00008000;
			break;
		}
		case 0x006: /* mailbox out L */ {
			puts("WARN: MMIO: DSP: Trying to write to mailbox out L");
			break;
		}
		case 0x00A: /* CSR */ {
			E_State.chipset.dsp.csr = val;
			if (val & DSP_CSR_RESET) { /* DSP is in reset now, don't do anything else */
				puts("MMIO: DSP: DSP is now in reset");
				E_State.chipset.dsp.priv_resetTimer = 2;
			}

			if (val & DSP_CSR_HALT) {
				puts("MMIO: DSP: DSP is now halted");
				E_State.chipset.dsp.priv_halt = true;
				E_State.chipset.dsp.outMboxH &= ~0x00008000;
			}
			else if (E_State.chipset.dsp.priv_halt) {
				puts("MMIO: DSP: DSP is now resuming");
				E_State.chipset.dsp.priv_halt = false;
				E_State.chipset.dsp.outMboxH |= 0x00008000;
			}
			break;
		}
		case 0x012: /* AR_SIZE */ {
			E_State.chipset.dsp.arSize = val;
			break;
		}
		case 0x020: /* AR_DMA_MMADDR (High) */ {
			E_State.chipset.dsp.dmaMMAddr &= 0x0000FFFF; /* clear high bits */
			E_State.chipset.dsp.dmaMMAddr |= (val << 16); /* set relevant high bits */
			break;
		}
		case 0x022: /* AR_DMA_MMADDR (Low) */ {
			E_State.chipset.dsp.dmaMMAddr &= 0xFFFF0000; /* clear low bits */
			E_State.chipset.dsp.dmaMMAddr |= val; /* set relevant low bits */
			break;
		}
		case 0x024: /* AR_DMA_ARADDR (High) */ {
			E_State.chipset.dsp.dmaARAddr &= 0x0000FFFF; /* clear high bits */
			E_State.chipset.dsp.dmaARAddr |= (val << 16); /* set relevant high bits */
			break;
		}
		case 0x026: /* AR_DMA_ARADDR (Low) */ {
			E_State.chipset.dsp.dmaARAddr &= 0xFFFF0000; /* clear low bits */
			E_State.chipset.dsp.dmaARAddr |= val; /* set relevant low bits */
			break;
		}
		case 0x028: /* AR_DMA_SIZE (High) */ {
			E_State.chipset.dsp.dmaSize &= 0x0000FFFF; /* clear high bits */
			E_State.chipset.dsp.dmaSize |= (val << 16); /* set relevant high bits */
			break;
		}
		case 0x02A: /* AR_DMA_SIZE (Low) */ {
			E_State.chipset.dsp.dmaSize &= 0xFFFF0000; /* clear low bits */
			E_State.chipset.dsp.dmaSize |= val; /* set relevant low bits */
			E_MMIO_DSP_DMAStart();
			break;
		}
		default: {
			printf("FATAL: MMIO: DSP: 16-bit Write (0x%04X) to unknown register 0x%02X\n", val & 0x0000FFFF, addr & 0x00000FFF);
			E_State.fatalError = true;
			return;
		}
		}
	}
	else if (accessWidth == 1) {
		val &= 0x000000FF;
		switch (addr & 0x00000FFF) {
		default: {
			printf("FATAL: MMIO: DSP: 8-bit Write (0x%02X) to unknown register 0x%02X\n", val & 0x000000FF, addr & 0x00000FFF);
			E_State.fatalError = true;
			return;
		}
		}
	}
	__builtin_unreachable();
	return;
}
