/*
 * DOL Runner - Emulated hardware
 * Copyright (C) 2025 Techflash
 */

 /* XXX: yes this is horrific, but it appears to be necessary */
#ifndef __USE_GNU
#define __USE_GNU
#include "cpu.h"
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <signal.h>
#include <ucontext.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "emu.h"
#include "mmio.h"
#include "config.h"

extern void emergencyBailOut(void);
static bool borked = false;


#if CONFIG_VERBOSE_SIGILL
#define debug_sigill(x) x
#else
#define debug_sigill(x) (void)0
#endif

#if CONFIG_VERBOSE_SIGSEGV
#define debug_sigsegv(x) x
#else
#define debug_sigsegv(x) (void)0
#endif

char *E_ConsoleTypeToStr(enum _consoleTypes type) {
	switch (type) {
	case CONSOLE_TYPE_GAMECUBE:
		return "GameCube";
	case CONSOLE_TYPE_WII:
		return "Wii";
	default:
		return "Unknown / Error";
	}

	return "Unknown / Error";
}

void E_Cleanup() {
	int i;
	for (i = 0; i < 8; i++) {
		if (E_State.mem.mem1_map_i[i] != (void *)-1)
			munmap(E_State.mem.mem1_map_i[i], E_State.mem.mem1_size);
		if (E_State.mem.mem1_map_d[i] != (void *)-1)
			munmap(E_State.mem.mem1_map_d[i], E_State.mem.mem1_size);
		if (E_State.mem.mem2_map_i[i] != (void *)-1)
			munmap(E_State.mem.mem2_map_i[i], E_State.mem.mem2_size);
		if (E_State.mem.mem2_map_d[i] != (void *)-1)
			munmap(E_State.mem.mem2_map_d[i], E_State.mem.mem2_size);
	}

	if (E_State.mem.mem1_fd > 0)
		close(E_State.mem.mem1_fd);
	if (E_State.mem.mem2_fd > 0)
		close(E_State.mem.mem2_fd);

	if (E_State.mem.aram)
		free(E_State.mem.aram);
}

void E_SIGSEGV_Handler(int sig, siginfo_t *info, void *uctx) {
	ucontext_t *ctx = (ucontext_t *)uctx;
	uint32_t **pc = ((uint32_t **)&ctx->uc_mcontext.regs->nip);
	uint32_t faultingAddr = (uint32_t)ctx->uc_mcontext.regs->dar;
	uint32_t accessType = ctx->uc_mcontext.regs->dsisr;
	uint32_t r2, r13;
	bool isRead = accessType == 0x40000000;
	bool isWrite = accessType == 0x42000000;
	bool isTranslating = ((E_State.cpu.msr & MSR_IR) >> MSR_IR_SHIFT) && ((E_State.cpu.msr & MSR_DR) >> MSR_DR_SHIFT);
	bool storeMem = false, updateRegDest = false, islha = false;
	uint32_t range, val, regSrc, regDest, instr, srcPtr, destPtr;
	int16_t memOff;
	int accessWidth;
	memAddr_t phys;
	(void)sig;
	(void)info;

	/* Save guest TLS/SDA pointers, restore host TLS/SDA pointers */
	asm volatile(
		"mr %0, %%r2\n\t"
		"mr %1, %%r13\n\t"
		// "mr %%r2, %2\n\t"
		"mr %%r13, %3\n\t"
		: "=r"(r2), "=r"(r13), "=r"(E_State.hostSDA[0]), "=r"(E_State.hostSDA[1]) : : "memory"
	);

	debug_sigsegv(
		printf("Caught SIGSEGV @ %p\n", *pc);
		printf("Faulting address: 0x%08X\n", faultingAddr);
		printf("Faulting access type: 0x%08X\n", accessType);
	);
	if (!isRead && !isWrite) {
		puts("FATAL: EMU: Unknown memory access type ^^");
		E_State.fatalError = true;
		goto sigsegv_out;
	}
	fflush(stdout);

	/* error handling */
	if (borked) {
		puts("That didn't work either - giving up.  Ctrl+C or `kill` the process.  Hanging in an infloop.");
		fflush(stdout);
		while (true) { /* ... */ }
	}
	if (E_State.fatalError) {
		puts("Woah there, we already caught a fatal error!");
		puts("Let's try exiting from here instead, because it clearly didn't work from the main thread...");
		fflush(stdout);
		borked = true;
		exit(1);
	}

	val = 0;

	/* get relevant instruction */
	instr = **pc;

	/* lwz? */
	if (((instr & 0xFC000000) >> 26) == 32) {
		debug_sigsegv(puts("is lwz"));
		regDest = (instr & 0x03E00000) >> 21;
		regSrc = (instr & 0x001F0000) >> 16;
		memOff = (instr & 0x0000FFFF);
		srcPtr = ctx->uc_mcontext.regs->gpr[regSrc];
		debug_sigsegv(printf("PPC: Loading from address in register r%d (0x%08X) at offset %d to register r%d\n", regSrc, srcPtr, memOff, regDest));
	}

	/* lhz? */
	else if (((instr & 0xFC000000) >> 26) == 40) {
		debug_sigsegv(puts("is lhz"));
		regDest = (instr & 0x03E00000) >> 21;
		regSrc = (instr & 0x001F0000) >> 16;
		memOff = (instr & 0x0000FFFF);
		srcPtr = ctx->uc_mcontext.regs->gpr[regSrc];
		accessWidth = 2;

		debug_sigsegv(printf("PPC: Loading from address in register r%d (0x%08X) at offset %d to register r%d\n", regSrc, srcPtr, memOff, regDest));
	}

	/* stw? */
	else if (((instr & 0xFC000000) >> 26) == 36) {
		debug_sigsegv(puts("is stw"));
		storeMem = true;
		regSrc = (instr & 0x03E00000) >> 21;
		regDest = (instr & 0x001F0000) >> 16;
		memOff = (instr & 0x0000FFFF);
		accessWidth = 4;

		val = ctx->uc_mcontext.regs->gpr[regSrc];
		destPtr = ctx->uc_mcontext.regs->gpr[regDest] + memOff;
		debug_sigsegv(printf("PPC: destPtr = 0x%08X, val = 0x%08X\n", destPtr, val));
	}

	/* sth? */
	else if (((instr & 0xFC000000) >> 26) == 44) {
		debug_sigsegv(puts("is sth"));
		storeMem = true;
		regSrc = (instr & 0x03E00000) >> 21;
		regDest = (instr & 0x001F0000) >> 16;
		memOff = (instr & 0x0000FFFF);
		accessWidth = 2;

		val = ctx->uc_mcontext.regs->gpr[regSrc] & 0x0000FFFF;
		destPtr = ctx->uc_mcontext.regs->gpr[regDest] + memOff;
		debug_sigsegv(printf("PPC: destPtr = 0x%08X, val = 0x%04X\n", destPtr, val));
	}

	/* sthu? */
	else if (((instr & 0xFC000000) >> 26) == 45) {
		debug_sigsegv(puts("is sthu"));
		storeMem = true;
		updateRegDest = true;
		regSrc = (instr & 0x03E00000) >> 21;
		regDest = (instr & 0x001F0000) >> 16;
		memOff = (instr & 0x0000FFFF);
		accessWidth = 2;

		val = ctx->uc_mcontext.regs->gpr[regSrc] & 0x0000FFFF;
		destPtr = ctx->uc_mcontext.regs->gpr[regDest] + memOff;
		debug_sigsegv(printf("PPC: destPtr = 0x%08X, val = 0x%04X\n", destPtr, val));
	}

	/* lha? */
	else if (((instr & 0xFC000000) >> 26) == 42) {
		debug_sigsegv(puts("is lha"));
		islha = true;
		regDest = (instr & 0x03E00000) >> 21;
		regSrc = (instr & 0x001F0000) >> 16;
		memOff = (instr & 0x0000FFFF);
		srcPtr = ctx->uc_mcontext.regs->gpr[regSrc];
		accessWidth = 2;

		debug_sigsegv(printf("PPC: Loading from address in register r%d (0x%08X) at offset %d to register r%d\n", regSrc, srcPtr, memOff, regDest));
	}

	else {
		printf("FATAL: PPC: Unknown load/store instruction: 0x%08X\n", instr);
		E_State.fatalError = true;
		goto sigsegv_out;
	}

	/* try to resolve that address */
	if (isTranslating) {
		phys = E_MemVirtToPhys(faultingAddr, false);
		if (!phys.mapped) {
			phys = E_MemVirtToPhys(faultingAddr, true);
			if (!phys.mapped) {
				printf("FATAL: EMU: Trying to access unmapped address 0x%08X\n", faultingAddr);
				E_State.fatalError = true;
				goto sigsegv_out;
			}
		}
	}
	else {
		phys.addr = faultingAddr;
		phys.mapped = true;
		phys.writable = true;
	}
	range = phys.addr & 0xFFF00000;
	if (range == 0x0C000000) { /* Legacy Flipper registers */
		if (isWrite)
			E_MMIO_Flipper_Write(phys.addr, val, accessWidth);
		else if (isRead)
			val = E_MMIO_Flipper_Read(phys.addr, accessWidth);

		if (updateRegDest)
			ctx->uc_mcontext.regs->gpr[regDest] = destPtr;
		(*pc)++;
	}
	else if (range == 0x0D800000 || range == 0x0D000000) {
		if (E_State.consoleType == CONSOLE_TYPE_GAMECUBE) {
			printf("FATAL: EMU: Attempting to access Hollywood registers (0x%08X) on GameCube\n", phys.addr);
			E_State.fatalError = true;
			goto sigsegv_out;
		}

		phys.addr &= ~0x00800000;
		if (isWrite)
			E_MMIO_Hollywood_Write(phys.addr, val, accessWidth);
		else if (isRead)
			val = E_MMIO_Hollywood_Read(phys.addr, accessWidth);
		if (updateRegDest)
			ctx->uc_mcontext.regs->gpr[regDest] = destPtr;
		(*pc)++;
	}
	else {
		printf("FATAL: EMU: Trying to access phys address 0x%08X in nonsense range 0x%08X\n", phys.addr, range);
		E_State.fatalError = true;
		goto sigsegv_out;
	}

sigsegv_out:
	debug_sigsegv
	(printf("isRead: %d, isWrite: %d, storeMem: %d, islha: %d\n", isRead, isWrite, storeMem, islha));
	/* if this is a read, write the read value back into the register/memory */
	if (isRead) {
		if (storeMem) {
			if (accessWidth == 4)
				*(uint32_t *)destPtr = val;
			else if (accessWidth == 2)
				*(uint32_t *)destPtr = val & 0x0000FFFF;
			else if (accessWidth == 1)
				*(uint32_t *)destPtr = val & 0x000000FF;
		}
		else {
			/* XXX: Gross hack for specifically lha */
			if (islha) {
				if (val & 0x00008000)
					ctx->uc_mcontext.regs->gpr[regDest] = 0xFFFF0000;
				else
					ctx->uc_mcontext.regs->gpr[regDest] = 0x00000000;

				ctx->uc_mcontext.regs->gpr[regDest] |= (val & 0x0000FFFF);
				debug_sigsegv(printf("lha got raw value 0x%08X, wrote 0x%08X\n", val, (uint32_t)ctx->uc_mcontext.regs->gpr[regDest]));
			}
			else if (accessWidth == 4)
				ctx->uc_mcontext.regs->gpr[regDest] = val;
			else if (accessWidth == 2)
				ctx->uc_mcontext.regs->gpr[regDest] = val & 0x0000FFFF;
			else if (accessWidth == 1)
				ctx->uc_mcontext.regs->gpr[regDest] = val & 0x000000FF;
		}
	}

	if (E_State.fatalError) {
		printf("EMU: Uh oh!  Caught fatal error in handling this SIGILL (@ 0x%08X) - trying to recover...\n", (uint32_t)*pc);
		ctx->uc_mcontext.regs->gpr[1] = ((uint32_t)(&E_State.sigstk)); /* give it our stack - game may have clobbered it */

		/* try to give it sane TLS/SDA pointers */
		ctx->uc_mcontext.regs->gpr[2] = E_State.hostSDA[0];
		ctx->uc_mcontext.regs->gpr[13] = E_State.hostSDA[1];
		*pc = ((uint32_t *)emergencyBailOut);
		return;
	}

	debug_sigsegv(printf("Successfully handled instruction, returning to PC=0x%08X\n", (uint32_t)*pc));

	/* Restore guest TLS/SDA pointers */
	asm volatile(
		// "mr %%r2, %0\n\t"
		"mr %%r13, %1\n\t"
		: "=r"(r2), "=r"(r13) : : "memory"
	);

	return;
}

void E_SIGILL_Handler(int sig, siginfo_t *info, void *uctx) {
	ucontext_t *ctx = (ucontext_t *)uctx;
	uint32_t **pc = ((uint32_t **)&ctx->uc_mcontext.regs->nip);
	uint32_t val = **pc;
	uint32_t sprRaw, sprLow, sprHi, spr, regSrc, regDest, msr, segmentReg, r2, r13;
	(void)sig;
	(void)info;
	/* Save guest TLS/SDA pointers, restore host TLS/SDA pointers */
	asm volatile(
		"mr %0, %%r2\n\t"
		"mr %1, %%r13\n\t"
		// "mr %%r2, %2\n\t"
		"mr %%r13, %3\n\t"
		: "=r"(r2), "=r"(r13), "=r"(E_State.hostSDA[0]), "=r"(E_State.hostSDA[1]) : : "memory"
	);


	debug_sigill(printf("Caught SIGILL @ %p\n", *pc));
	fflush(stdout);

	/* error handling */
	if (borked) {
		puts("That didn't work either - giving up.  Ctrl+C or `kill` the process.  Hanging in an infloop.");
		fflush(stdout);
		while (true) { /* ... */ }
	}
	if (E_State.fatalError) {
		puts("Woah there, we already caught a fatal error!");
		puts("Let's try exiting from here instead, because it clearly didn't work from the main thread...");
		fflush(stdout);
		borked = true;
		exit(1);
	}
	msr = E_State.cpu.msr;

	/* mfspr? */
	if (((val & 0x3FF) >> 1) == 339 && ((val & 0xFC000000) >> 26) == 31) {
		debug_sigill(puts("is mfspr"));
		sprRaw = (val & 0x001FF800) >> 11;
		sprLow = sprRaw & 0x1F;
		sprHi  = (sprRaw >> 5) & 0x1F;
		spr    = sprHi | (sprLow << 5);
		debug_sigill(printf("sprRaw: 0x%08X, sprLow: 0x%08X, sprHi: 0x%08X, spr: %u\n", sprRaw, sprLow, sprHi, spr));
		regDest = (val & 0x03E00000) >> 21;
		debug_sigill(printf("regDest: %d\n", regDest));
		ctx->uc_mcontext.regs->gpr[regDest] = E_PPC_Emulate_mfspr(spr);
		(*pc)++; /* forward by 1 instruction, since it's a uint32_t */
	}

	/* mtspr? */
	else if (((val & 0x3FF) >> 1) == 467 && ((val & 0xFC000000) >> 26) == 31) {
		debug_sigill(puts("is mtspr"));
		sprRaw = (val & 0x001FF800) >> 11;
		sprLow = sprRaw & 0x1F;
		sprHi  = (sprRaw >> 5) & 0x1F;
		spr    = sprHi | (sprLow << 5);
		debug_sigill(printf("sprRaw: 0x%08X, sprLow: 0x%08X, sprHi: 0x%08X, spr: %u\n", sprRaw, sprLow, sprHi, spr));
		regSrc = (val & 0x03E00000) >> 21;
		debug_sigill(printf("regSrc: %d, val: 0x%08X\n", regSrc, (uint32_t)ctx->uc_mcontext.regs->gpr[regSrc]));
		E_PPC_Emulate_mtspr(spr, ctx->uc_mcontext.regs->gpr[regSrc]);
		(*pc)++;
	}

	/* mfmsr? */
	else if (((val & 0x3FF) >> 1) == 83 && ((val & 0xFC000000) >> 26) == 31) {
		debug_sigill(puts("is mfmsr"));
		regDest = (val & 0x03E00000) >> 21;
		debug_sigill(printf("regDest: %d\n", regDest));
		ctx->uc_mcontext.regs->gpr[regDest] = E_PPC_Emulate_mfmsr();
		(*pc)++;
	}

	/* mtmsr? */
	else if (((val & 0x3FF) >> 1) == 146 && ((val & 0xFC000000) >> 26) == 31) {
		debug_sigill(puts("is mtmsr"));
		regSrc = (val & 0x03E00000) >> 21;
		debug_sigill(printf("regSrc: %d, val: 0x%08X\n", regSrc, (uint32_t)ctx->uc_mcontext.regs->gpr[regSrc]));
		E_PPC_Emulate_mtmsr(ctx->uc_mcontext.regs->gpr[regSrc]);
		(*pc)++;
	}

	/* rfi? */
	else if (val == 0x4C000064) {
		debug_sigill(puts("is rfi"));
		debug_sigill(printf("old msr = 0x%08X, new msr = ", msr));
		*pc = (uint32_t *)E_PPC_Emulate_rfi();
		debug_sigill(printf("0x%08X\n", E_State.cpu.msr));
	}

	/* mtsr? */
	else if (((val & 0x3FF) >> 1) == 210 && ((val & 0xFC000000) >> 26) == 31) {
		debug_sigill(puts("is mtsr"));
		regSrc = (val & 0x03E00000) >> 21;
		segmentReg = (val & 0xF0000) >> 16;
		debug_sigill(printf("regSrc: %d, val: 0x%08X, segmentReg: %d\n", regSrc, (uint32_t)ctx->uc_mcontext.regs->gpr[regSrc], segmentReg));
		E_PPC_Emulate_mtsr(segmentReg, ctx->uc_mcontext.regs->gpr[regSrc]);
		(*pc)++;
	}

	/* mfsr? */
	else if (((val & 0x3FF) >> 1) == 595 && ((val & 0xFC000000) >> 26) == 31) {
		debug_sigill(puts("is mfsr"));
		regDest = (val & 0x03E00000) >> 21;
		segmentReg = (val & 0xF0000) >> 16;
		debug_sigill(printf("regDest: %d, segmentReg: %d\n", regDest, segmentReg));
		ctx->uc_mcontext.regs->gpr[regDest] = E_PPC_Emulate_mfsr(segmentReg);
		(*pc)++;
	}

	/* dcbi? */
	else if (((val & 0x3FF) >> 1) == 470 && ((val & 0xFC000000) >> 26) == 31) {
		puts("is dcbi");
		puts("WARN: Can't emulate cache related functionality");
		(*pc)++;
	}

	/* well crap */
	else {
		printf("FATAL: EMU: Unhandled illegal instruction @ 0x%08X: 0x%08X\n", (uint32_t)*pc, val);
		E_State.fatalError = true;
	}

	if (E_State.needsMemMapUpdate)
		E_MemMapFixups(msr);

	if (E_State.fatalError) {
		printf("EMU: Uh oh!  Caught fatal error in handling this SIGILL @ 0x%08X - trying to recover...\n", (uint32_t)*pc);
		ctx->uc_mcontext.regs->gpr[1] = ((uint32_t)(&E_State.sigstk)); /* give it our stack - game may have clobbered it */

		/* try to give it sane TLS/SDA pointers */
		ctx->uc_mcontext.regs->gpr[2] = E_State.hostSDA[0];
		ctx->uc_mcontext.regs->gpr[13] = E_State.hostSDA[1];
		*pc = ((uint32_t *)emergencyBailOut);
		return;
	}

	debug_sigill(printf("Successfully handled instruction, returning to PC=0x%08X\n", (uint32_t)*pc));
	fflush(stdout);

	/* Restore guest TLS/SDA pointers */
	asm volatile(
		// "mr %%r2, %0\n\t"
		"mr %%r13, %1\n\t"
		: "=r"(r2), "=r"(r13) : : "memory"
	);
	return;
}
