/*
 * DOL Runner - Emulated hardware
 * Copyright (C) 2025 Techflash
 */

#ifndef _DOLTOOLS_DOLRUN_EMU_H
#define _DOLTOOLS_DOLRUN_EMU_H

#include "cpu.h"
#include "mem.h"
#include "mmio/chipset.h"
#include <signal.h>

enum _consoleTypes {
	/* Retail Nintendo® GameCube® */
	CONSOLE_TYPE_GAMECUBE = 1,

	/* Retail Nintendo® Wii® */
	CONSOLE_TYPE_WII,

	/* TODO: Dev hardware? Wii U® vWii? */
};

/*
 * Get a string representing a given console type.
 */
extern char *E_ConsoleTypeToStr(enum _consoleTypes type);

struct _emu_state {
	/* Emulated console type, must be set before E_*() calls are made */
	enum _consoleTypes consoleType;

	/* Host console type, must be set before E_*() calls are made */
	enum _consoleTypes hostType;

	/* Emulated memory state */
	struct _memState mem;

	/* Emulated CPU state (at least, the small bits that need to be emulated */
	struct _cpuState cpu;

	/* Emulated Chipset state (Flipper/Hollywood) */
	struct _chipset_state chipset;

	/* true if the emulator encountered a fatal error and should stop */
	bool fatalError;

	/* true if we need to update our memory map before returning */
	bool needsMemMapUpdate;

	/* stack for signals, or in-case-of-emergency */
	stack_t sigstk;

	/* Host TLS/SDA pointers */
	uint32_t hostSDA[2];
};

/*
 * Global state for all emulated hardware.
 */
extern struct _emu_state E_State;

/*
 * Clean up all subsystems in preparation to exit.
 */
extern void E_Cleanup();

/* SIGSEGV handler */
extern void E_SIGSEGV_Handler(int sig, siginfo_t *info, void *uctx);

/* SIGILL handler */
extern void E_SIGILL_Handler(int sig, siginfo_t *info, void *uctx);

#endif
