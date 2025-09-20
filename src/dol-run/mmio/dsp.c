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

static struct _dsp_state *state;

static void E_MMIO_DSP_DMAStart(void) {
	uint32_t dmaDir = (state->dmaSize & 0x80000000) >> 31;
	state->priv_dmaDir = (bool)dmaDir;

	if (!dmaDir) {
		/* 0 = Write to ARAM */
		state->priv_dmaSrcPtr = state->dmaMMAddr;
		state->priv_dmaDestPtr = state->dmaARAddr;
	}
	else {
		/* 1 = Read from ARAM */
		state->priv_dmaSrcPtr = state->dmaARAddr;
		state->priv_dmaDestPtr = state->dmaMMAddr;
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
	if (state->priv_resetTimer == 2) {
		puts("MMIO: DSP: DSP in reset stage 2");
		state->priv_resetTimer--;
		if ((state->csr & DSP_CSR_BOOTMODE) == 0) {
			puts("MMIO: DSP: Pulling from memory to seed IRAM");
			virt = E_MemPhysToVirt(0x01000000, true);
			if (!virt.mapped) {
				puts("FATAL: MMIO: DSP: MEM1 isn't mapped ?!");
				E_State.fatalError = true;
				return;
			}
			memcpy(state->iram, (const void *)virt.addr, 1024);
			return;
		}
	}

	else if (state->priv_resetTimer == 1) {
		puts("MMIO: DSP: DSP is about to finish resetting (stage 1)");
		state->csr &= ~DSP_CSR_RESET;

		state->priv_resetTimer--;
		puts("MMIO: DSP: Finished DSP reset");
	}

	/* are we halted? */
	if (state->priv_halt)
		return;


	/* TODO: Actually start doing more stuff here */
	dmaSize = state->dmaSize & 0x7FFFFFFF;
	if (dmaSize & 0x1F) {
		E_State.fatalError = true;
		puts("FATAL: MMIO: DSP: Trying to do unaligned DMA");
		return;
	}

	if (dmaSize == 0)
		goto noDMA;

	/* do a block of DMA */
	if (!state->priv_dmaDir)
		printf("MMIO: DSP: Would do 32 bytes of DSP, from 0x%08X (ARAM) to 0x%08X (Mem)\n", state->priv_dmaSrcPtr, state->priv_dmaDestPtr);
	else
		printf("MMIO: DSP: Would do 32 bytes of DSP, from 0x%08X (Mem) to 0x%08X (ARAM)\n", state->priv_dmaSrcPtr, state->priv_dmaDestPtr);
	state->priv_dmaDestPtr += 32;
	state->priv_dmaSrcPtr += 32;
	state->dmaSize -= 32;

noDMA:
	return;
}

void E_MMIO_DSP_Init(void) {
	state = &E_State.chipset.dsp;
	memset(state, 0, sizeof(struct _dsp_state));
	state->iram = malloc(8 * 1024);
	state->dram = malloc(8 * 1024);
	state->irom = malloc(8 * 1024);
	state->drom = malloc(4 * 1024);
	state->csr  = 0x0816; /* at least, this is the start that Linux has it in at idle */
	E_Timer_AddHook(E_MMIO_DSP_TimerHook);
}

void E_MMIO_DSP_Cleanup(void) {
	free(state->iram);
	free(state->dram);
	free(state->irom);
	free(state->drom);
	state->priv_halt = true;
}

uint32_t E_MMIO_DSP_Read(uint32_t addr, int accessWidth) {
	if (accessWidth == 4) {
		switch (addr & 0x00000FFF) {
		case 0x020: /* DSP_AR_DMA_MMADDR */
			return state->dmaMMAddr;
		case 0x024: /* DSP_AR_DMA_ARADDR */
			return state->dmaARAddr;
		case 0x028: /* DSP_AR_DMA_SIZE */
			return state->dmaSize;
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
			return state->inMboxH;
		case 0x002: /* mailbox in L */
			return state->inMboxL;
		case 0x004: /* mailbox out H */
			return state->outMboxH;
		case 0x006: /* mailbox out L */
			return state->outMboxL;
		case 0x00A: /* CSR */
			return state->csr;
		case 0x012: /* AR_SIZE */
			return state->arSize;
		case 0x020: /* DSP_AR_DMA_MMADDR (High) */
		    return (state->dmaMMAddr & 0xFFFF0000) >> 16;
		case 0x022: /* DSP_AR_DMA_MMADDR (Low) */
		    return state->dmaMMAddr & 0x0000FFFF;
		case 0x024: /* DSP_AR_DMA_ARADDR (High) */
			return (state->dmaMMAddr & 0xFFFF0000) >> 16;
		case 0x026: /* DSP_AR_DMA_ARADDR (Low) */
			return state->dmaARAddr & 0x0000FFFF;
		case 0x028: /* DSP_AR_DMA_SIZE (High) */
			return (state->dmaSize & 0xFFFF0000) >> 16;
		case 0x02A: /* DSP_AR_DMA_SIZE (Low) */
			return state->dmaSize & 0x0000FFFF;
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
			state->dmaMMAddr = val;
			break;
		}
		case 0x024: /* AR_DMA_ARADDR */ {
			state->dmaARAddr = val;
			break;
		}
		case 0x028: /* AR_DMA_SIZE */ {
			state->dmaSize = val;
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
			state->inMboxH = val;
			if (val & 0x00008000)
				E_MMIO_DSP_MboxPush_H(val);
			break;
		}
		case 0x002: /* mailbox in L */ {
			state->inMboxL = val;
			if (state->inMboxH & 0x00008000)
				E_MMIO_DSP_MboxPush_L(val);
			break;
		}
		case 0x004: /* mailbox out H */ {
			if (val & 0x00008000)
				state->outMboxH |= 0x00008000;
			else
				state->outMboxH &= ~0x00008000;
			break;
		}
		case 0x006: /* mailbox out L */ {
			puts("WARN: MMIO: DSP: Trying to write to mailbox out L");
			break;
		}
		case 0x00A: /* CSR */ {
			state->csr = val;
			if (val & DSP_CSR_RESET) { /* DSP is in reset now, don't do anything else */
				puts("MMIO: DSP: DSP is now in reset");
				state->priv_resetTimer = 2;
			}

			if (val & DSP_CSR_HALT) {
				puts("MMIO: DSP: DSP is now halted");
				state->priv_halt = true;
				state->outMboxH &= ~0x00008000;
			}
			else if (state->priv_halt) {
				puts("MMIO: DSP: DSP is now resuming");
				state->priv_halt = false;
				state->outMboxH |= 0x00008000;
			}
			break;
		}
		case 0x012: /* AR_SIZE */ {
			state->arSize = val;
			break;
		}
		case 0x020: /* AR_DMA_MMADDR (High) */ {
			state->dmaMMAddr &= 0x0000FFFF; /* clear high bits */
			state->dmaMMAddr |= (val << 16); /* set relevant high bits */
			break;
		}
		case 0x022: /* AR_DMA_MMADDR (Low) */ {
			state->dmaMMAddr &= 0xFFFF0000; /* clear low bits */
			state->dmaMMAddr |= val; /* set relevant low bits */
			break;
		}
		case 0x024: /* AR_DMA_ARADDR (High) */ {
			state->dmaARAddr &= 0x0000FFFF; /* clear high bits */
			state->dmaARAddr |= (val << 16); /* set relevant high bits */
			break;
		}
		case 0x026: /* AR_DMA_ARADDR (Low) */ {
			state->dmaARAddr &= 0xFFFF0000; /* clear low bits */
			state->dmaARAddr |= val; /* set relevant low bits */
			break;
		}
		case 0x028: /* AR_DMA_SIZE (High) */ {
			state->dmaSize &= 0x0000FFFF; /* clear high bits */
			state->dmaSize |= (val << 16); /* set relevant high bits */
			break;
		}
		case 0x02A: /* AR_DMA_SIZE (Low) */ {
			state->dmaSize &= 0xFFFF0000; /* clear low bits */
			state->dmaSize |= val; /* set relevant low bits */
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
