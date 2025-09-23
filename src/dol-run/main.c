/*
 * DOL Runner - Main source
 * Copyright (C) 2025 Techflash
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "../common/swap.h"
#include "../common/types.h"
#include "emu.h"
#include "mem.h"
#include "timer.h"
#include "mmio/forward.h"

struct _emu_state E_State;

#if ! (defined(__PPC__) || defined(_M_PPC) || defined(_ARCH_PPC) || defined(__powerpc__))
#  error "dol-run cannot operate on a machine that is not PowerPC."
#endif

static void usage(char *name) {
	printf("Usage: %s <file>\n", name);
	exit(1);
}


void emergencyBailOut(void) {
	printf("Bailing out in main thread due to fatal error\n");
	exit(1);
}

int main(int argc, char *argv[]) {
	int fd, ret, i, j;
	char errBuf[256], modelBuf[64];
	uint32_t instr, r2, r13;
	uint8_t *file, *destAddr;
	DOL_Hdr_t *hdr;
	FILE *fp;
	void (*entry)(void);
	struct stat statbuf;
	bool usePhys = false;
	struct sigaction sa;
	sigset_t sa_set;

	if (argc != 2 || argv[1][0] == '\0')
		usage(argv[0]); /* bad args */

	/* detect real hardware platform, so we can know where to map register writes to */
	fp = fopen("/sys/firmware/devicetree/base/model", "r");
	if (!fp) {
		perror("Failed to open /sys/firmware/devicetree/base/model");
		return 1;
	}
	ret = fread(modelBuf, 64, 1, fp);
	/* will not necessarily read an entire 64B */
	if (ferror(fp)) {
		perror("Failed to read /sys/firmware/devicetree/base/model");
		return 1;
	}

	fclose(fp);

	/* we read the entire file, check the model */
	if (strcmp(modelBuf, "nintendo,wii") == 0)
		E_State.hostType = CONSOLE_TYPE_WII;
	else if (strcmp(modelBuf, "nintendo,gamecube") == 0)
		E_State.hostType = CONSOLE_TYPE_GAMECUBE;
	else {
		printf("Host device type (%s) is neither Wii nor GameCube - cannot continue.\n", modelBuf);
		return 1;
	}

	/* try to open the provided file */
	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		sprintf(errBuf, "Failed to open %s", argv[1]);
		perror(errBuf);
		return 1;
	}

	/* get file size */
	ret = fstat(fd, &statbuf);
	if (ret != 0) {
		sprintf(errBuf, "Failed to get size of %s", argv[1]);
		perror(errBuf);
		return 1;
	}

	/* mmap the thing */
	file = mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (file == MAP_FAILED) {
		sprintf(errBuf, "Failed to mmap() %s", argv[1]);
		perror(errBuf);
		return 1;
	}

	/* set header addr to start of file */
	hdr = (DOL_Hdr_t *)file;

	/* byteswap header - equates to no-op on BE PPC,
	 * but for potential future portability
	 */
	DOL_Hdr_Byteswap(hdr);

	/*
	 * so... let's talk about this, because this is a *really* bad way to do this.
	 * basically, we loop over every text section that has a non-zero size, and
	 * then... yeah, we scan the *entire* code blob for a specific magic value.
	 * this is.... very slow, we could be scanning *MEGABYTES* of code here.
	 *
	 * this approach to conole detection is what Dolphin does, and I'll admit it....
	 * does actually work.  it's horridly clunky and probably makes startup take way
	 * longer than it should, but it *does* work, so I'm doing it here.
	 * if someone can think of a better way to detect it, please do send a PR.
	 */
	E_State.consoleType = CONSOLE_TYPE_GAMECUBE;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < hdr->textSize[i] / sizeof(uint32_t); j++) {
			instr = *(uint32_t *)(file + hdr->textOff[i] + (j * sizeof(uint32_t)));
			if ((instr & 0xfc1fffff) == 0x7c13fba6) {
				E_State.consoleType = CONSOLE_TYPE_WII;
				goto determinedConsoleType;
			}
		}
	}
determinedConsoleType:

	/* make sure we are not in an error state */
	E_State.fatalError = false;

	/* Do timer init */
	E_Timer_Init();

	printf("Setting up memory map for target: %s\n", E_ConsoleTypeToStr(E_State.consoleType));

	/*
	 * set default entry point to provided one;
	 * if we see BS1 below, we may override this
	 */
	entry = (void (*)(void))hdr->entry;

	/* XXX: HACK!  Do we have BS1?  if so, save it as the entry, and start with mapping disabled */
	if (hdr->textAddr[0] == 0x80003400) {
		entry = (void (*)(void))0x00003400;
		usePhys = true;
	}

	/* use virtual mappings, unless usePhys is set */
	if (usePhys)
		E_State.cpu.msr = 0;
	else
		E_State.cpu.msr = MSR_IR | MSR_DR;

	/* set up our mappings */
	E_MemInit();
	if (E_State.fatalError)
	    return 1;

	puts("Setting up Chipset MMIO...");
	ret = E_MMIO_Chipset_Init();
	if (ret) {
		printf("FATAL: E_MMIO_Chipset_Init() failed with return code %d\n", ret);
		return 1;
	}
	if (E_State.fatalError)
	    return 1;

	printf("Loading %s for console type: %s, host console: %s...\n", argv[1], E_ConsoleTypeToStr(E_State.consoleType), E_ConsoleTypeToStr(E_State.hostType));

	/* copy Text sections */
	for (i = 0; i < 7; i++) {
		if (!hdr->textSize[i] || !hdr->textOff[i])
			continue;
		/* flush stdout in case we crash */
		destAddr = (uint8_t *)hdr->textAddr[i];
		printf("About to load %u bytes from 0x%08X to 0x%08X\n", hdr->textSize[i], hdr->textOff[i], (uint32_t)destAddr);
		fflush(stdout);

		memcpy(destAddr, file + hdr->textOff[i], hdr->textSize[i]);
	}

	/* copy Data sections */
	for (i = 0; i < 11; i++) {
		if (!hdr->dataSize[i] || !hdr->dataOff[i])
			continue;
		destAddr = (uint8_t *)hdr->dataAddr[i];
		printf("About to load %u bytes from 0x%08X to 0x%08X\n", hdr->dataSize[i], hdr->dataOff[i], (uint32_t)destAddr);
		fflush(stdout);
		memcpy(destAddr, file + hdr->dataOff[i], hdr->dataSize[i]);
	}

	printf("Successfully loaded DOL, going to begin native execution @ 0x%08X...\n", (uint32_t)entry);
	puts("Installing necessary signal handlers...");
	sa.sa_sigaction = E_SIGSEGV_Handler;
	sigemptyset(&sa_set);
	sa.sa_mask = sa_set;
	sa.sa_flags = SA_SIGINFO | SA_ONSTACK;

	/* set up alt stack */
	E_State.sigstk.ss_sp = malloc(SIGSTKSZ);
	if (!E_State.sigstk.ss_sp) {
		perror("Failed to allocate memory for signal alt-stack");
		E_Cleanup();
		return 1;
	}
	E_State.sigstk.ss_size = SIGSTKSZ;
	E_State.sigstk.ss_flags = 0;
	ret = sigaltstack(&E_State.sigstk, NULL);
	if (ret < 0) {
		perror("sigaltstack failed");
		E_Cleanup();
		return 1;
	}

	ret = sigaction(SIGSEGV, &sa, NULL);
	if (ret) {
		perror("Failed to set SIGSEGV handler");
		E_Cleanup();
		return 1;
	}

	sa.sa_sigaction = E_SIGILL_Handler;
	ret = sigaction(SIGILL, &sa, NULL);
	if (ret) {
		perror("Failed to set SIGILL handler");
		E_Cleanup();
		return 1;
	}
	puts("Signal handlers installed!");

	/* if usePhys is set, swap back to physical mappings before we boot */
	if (usePhys) {
		puts("Remapping memory to physical address space...");
		fflush(stdout);
		E_State.cpu.msr = 0;
		E_MemMapFixups(MSR_IR | MSR_DR);
	}

	/* Set up some more sane CPU defaults */
	E_CPU_Init();
	if (E_State.fatalError)
	    return 1;

	/* Save TLS/SDA pointers */
	asm volatile(
		"mr %0, %%r2\n\t"
		"mr %1, %%r13\n\t"
		: "=r"(r2), "=r"(r13) : : "memory"
	);

	E_State.hostSDA[0] = r2;
	E_State.hostSDA[1] = r13;
	puts("Really beginning emulation... now!");

	fflush(stdout);

	/* YOLO! */
	entry();

	return 0;
}
