/*
 * DOL Runner - Emulated Hardware - Memory handling
 * Copyright (C) 2025 Techflash
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>


/* XXX: yes this is horrific, but it appears to be necessary */
#ifndef __USE_GNU
#define __USE_GNU
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/mman.h>
#include "cpu.h"
#include "emu.h"
#include "mem.h"
#include "config.h"

#if CONFIG_VERBOSE_BATS
#define debug_bat(x) x
#else
#define debug_bat(x) (void)0
#endif


#if CONFIG_VERBOSE_MEM
#define debug_mem(x) x
#else
#define debug_mem(x) (void)0
#endif

static int maxBAT;

static void E_MemReloadMap(void) {
	int i, j, fd;
	uint32_t *ibatu, *ibatl, *dbatu, *dbatl, paddr, vaddr, size;
	uint8_t **map_i, **map_d;
	/* here we go... */

	/*
	 * For every BAT, determine if the BATs
	 * are still valid.
	 * If they aren't, make them valid.
	 */
	for (i = 0; i < maxBAT; i++) {
		ibatu = &E_State.cpu.ibatu[i];
		ibatl = &E_State.cpu.ibatl[i];
		dbatu = &E_State.cpu.dbatu[i];
		dbatl = &E_State.cpu.dbatl[i];

		/* FASTPATH: is this BAT empty? */
		if (*ibatu == 0  && *ibatl == 0 && *dbatu == 0 && *dbatl == 0)
			continue; /* skip it */

		/* we have at least 1 real BAT - do the thing */

		switch (E_State.consoleType) {
		case CONSOLE_TYPE_GAMECUBE: {
			if (*ibatu == 0 && *ibatl == 0)
				goto gcn_dbat;

			/* IBATs can only possibly point to MEM1 */
			paddr = *ibatl & BATL_BPRN;
			if (paddr == MEM1_PADDR) {
				map_i = E_State.mem.mem1_map_i;
				map_d = E_State.mem.mem1_map_d;
				size = E_State.mem.mem1_size;
				fd = E_State.mem.mem1_fd;
			}
			else {
				printf("FATAL: MEM: BATs: GCN: Trying to map IBAT%d to phys addr 0x%08X, which is not MEM1\n", i, paddr);
				E_State.fatalError = true;
				return;
			}

			/* Is it already mapped? */
			for (j = 0; j < maxBAT; j++) {
				if (map_i[j] == (uint8_t *)vaddr)
					goto gcn_dbat; /* Yes - ignore this mapping! */
			}
			/* Map MEM1 Instr @ BEPI */
			vaddr = *ibatu & BATU_BEPI; /* don't shift it to keep a full address */
			printf("MEM: BATs: GCN: Mapping IBAT%d, phys addr 0x%08X -> virt addr 0x%08X\n", i, paddr, vaddr);
			fflush(stdout);
			map_i[i] = mmap((void *)vaddr, size,
							PROT_READ | PROT_WRITE | PROT_EXEC,
							MAP_SHARED | MAP_FIXED,
							fd, 0);

			if (map_i[i] == MAP_FAILED) {
				printf("FATAL: MEM: BATs: GCN: Failed mapping IBAT%d with phys addr 0x%08X to virt addr 0x%08X\n", i, paddr, vaddr);
				E_State.fatalError = true;
				return;
			}

		gcn_dbat:
			if (*dbatu == 0 && *dbatl == 0)
				goto gcn_out;

			/* DBATs can point to either MEM1 or Flipper I/O */
			paddr = *dbatl & BATL_BPRN;
			if (paddr == MEM1_PADDR) {
				map_i = E_State.mem.mem1_map_i;
				map_d = E_State.mem.mem1_map_d;
				size = E_State.mem.mem1_size;
				fd = E_State.mem.mem1_fd;
			}
			else {
				printf("FATAL: MEM: BATs: GCN: Trying to map DBAT%d to phys addr 0x%08X, which is not MEM1\n", i, paddr);
				E_State.fatalError = true;
				return;
			}

			/* Map MEM1 Data @ BEPI */
			vaddr = *dbatu & BATU_BEPI; /* don't shift it to keep a full address */

			/* Is it already mapped? */
			for (j = 0; j < maxBAT; j++) {
				if (map_d[j] == (uint8_t *)vaddr)
					goto gcn_out; /* Yes - ignore it */

				/* Make sure we don't override an instruction mapping */
				if (map_i[j] == (uint8_t *)vaddr)
					goto gcn_out; /* Yes - ignore this mapping! */
			}

			/* No other mapping for this vaddr, map it */
			printf("MEM: BATs: GCN: Mapping DBAT%d, phys addr 0x%08X -> virt addr 0x%08X\n", i, paddr, vaddr);
			fflush(stdout);
			map_d[i] = mmap((void *)vaddr, size,
							PROT_READ | PROT_WRITE,
							MAP_SHARED | MAP_FIXED,
							fd, 0);

			if (map_d[i] == MAP_FAILED && vaddr < MAX_ALLOWED_VADDR) {
				printf("FATAL: MEM: BATs: GCN: Failed mapping DBAT%d with phys addr 0x%08X to virt addr 0x%08X: %s (%d)\n", i, paddr, vaddr, strerror(errno), errno);
				E_State.fatalError = true;
				return;
			}
		gcn_out:
			break;
		}
		case CONSOLE_TYPE_WII: {
			if (*ibatu == 0 && *ibatl == 0)
				goto wii_dbat;

			/* IBATs can only possibly point to MEM1 or MEM2 */
			paddr = *ibatl & BATL_BPRN;
			if (paddr == MEM1_PADDR) {
				map_i = E_State.mem.mem1_map_i;
				map_d = E_State.mem.mem1_map_d;
				size = E_State.mem.mem1_size;
				fd = E_State.mem.mem1_fd;
			}
			else if (paddr == MEM2_PADDR) {
				map_i = E_State.mem.mem2_map_i;
				map_d = E_State.mem.mem2_map_d;
				size = E_State.mem.mem2_size;
				fd = E_State.mem.mem2_fd;
			}
			else {
				printf("FATAL: MEM: BATs: Wii: Trying to map IBAT%d to phys addr 0x%08X, which is not MEM1 nor MEM2\n", i, paddr);
				E_State.fatalError = true;
				return;
			}

			/* Is it already mapped? */
			for (j = 0; j < maxBAT; j++) {
				if (map_i[j] == (uint8_t *)vaddr)
					goto wii_dbat; /* Yes - ignore this mapping! */
			}

			/* Map MEM1/2 Instr @ BEPI */
			vaddr = *ibatu & BATU_BEPI; /* don't shift it to keep a full address */
			printf("MEM: BATs: Wii: Mapping IBAT%d, phys addr 0x%08X -> virt addr 0x%08X\n", i, paddr, vaddr);
			fflush(stdout);
			map_i[i] = mmap((void *)vaddr, size,
							PROT_READ | PROT_WRITE | PROT_EXEC,
							MAP_SHARED | MAP_FIXED,
							fd, 0);

			if (map_i[i] == MAP_FAILED && vaddr < MAX_ALLOWED_VADDR) {
				printf("FATAL: MEM: BATs: Wii: Failed mapping IBAT%d with phys addr 0x%08X to virt addr 0x%08X: %s (%d)\n", i, paddr, vaddr, strerror(errno), errno);
				E_State.fatalError = true;
				return;
			}

		wii_dbat:
			if (*dbatu == 0 && *dbatl == 0)
				goto wii_out;

			/* DBATs can point to MEM1, MEM2, Flipper I/O, or Hollywood I/O */
			paddr = *dbatl & BATL_BPRN;
			if (paddr == MEM1_PADDR) {
				map_i = E_State.mem.mem1_map_i;
				map_d = E_State.mem.mem1_map_d;
				size = E_State.mem.mem1_size;
				fd = E_State.mem.mem1_fd;
			}
			else if (paddr == MEM2_PADDR) {
				map_i = E_State.mem.mem2_map_i;
				map_d = E_State.mem.mem2_map_d;
				size = E_State.mem.mem2_size;
				fd = E_State.mem.mem2_fd;
			}
			else {
				printf("FATAL: MEM: BATs: Wii: Trying to map DBAT%d to phys addr 0x%08X, which is not MEM1 nor MEM2\n", i, paddr);
				E_State.fatalError = true;
				return;
			}

			/* Map MEM1/2 Data @ BEPI */
			vaddr = *dbatu & BATU_BEPI; /* don't shift it to keep a full address */

			/* Is it already mapped? */
			for (j = 0; j < maxBAT; j++) {
				if (map_d[j] == (uint8_t *)vaddr)
					goto wii_out; /* Yes - ignore it */

				/* Make sure we don't override an instruction mapping */
				if (map_i[j] == (uint8_t *)vaddr)
					goto wii_out; /* Yes - ignore this mapping! */
			}

			/* No other mapping for this vaddr, map it */
			printf("MEM: BATs: Wii: Mapping DBAT%d, phys addr 0x%08X -> virt addr 0x%08X\n", i, paddr, vaddr);
			fflush(stdout);
			map_d[i] = mmap((void *)vaddr, size,
							PROT_READ | PROT_WRITE,
							MAP_SHARED | MAP_FIXED,
							fd, 0);

			if (map_d[i] == MAP_FAILED && vaddr < MAX_ALLOWED_VADDR) {
				printf("FATAL: MEM: BATs: Wii: Failed mapping DBAT%d with phys addr 0x%08X to virt addr 0x%08X: %s (%d)\n", i, paddr, vaddr, strerror(errno), errno);
				E_State.fatalError = true;
				return;
			}

		wii_out:
			break;
		}
		default: {
			puts("FATAL: MEM: BATs: Invalid console type!");
			E_State.fatalError = true;
			return;
		}
		}
	}

}

static char *E_MemPPCBlockLenToStr(uint32_t bl) {
	switch (bl) {
		case BATU_BL_128KB: return "128KB";
		case BATU_BL_256KB: return "256KB";
		case BATU_BL_512KB: return "512KB";
		case BATU_BL_1MB: return "1MB";
		case BATU_BL_2MB: return "2MB";
		case BATU_BL_4MB: return "4MB";
		case BATU_BL_8MB: return "8MB";
		case BATU_BL_16MB: return "16MB";
		case BATU_BL_32MB: return "32MB";
		case BATU_BL_64MB: return "64MB";
		case BATU_BL_128MB: return "128MB";
		case BATU_BL_256MB: return "256MB";
		default: return "!! Invalid !!";
	}
	return "!! Invalid !!";
}

static uint32_t E_MemPPCBlockLenToBytes(uint32_t bl) {
	switch (bl) {
		case BATU_BL_128KB: return 128 * 1024;
		case BATU_BL_256KB: return 256 * 1024;
		case BATU_BL_512KB: return 512 * 1024;
		case BATU_BL_1MB: return 1 * 1024 * 1024;
		case BATU_BL_2MB: return 2 * 1024 * 1024;
		case BATU_BL_4MB: return 4 * 1024 * 1024;
		case BATU_BL_8MB: return 8 * 1024 * 1024;
		case BATU_BL_16MB: return 16 * 1024 * 1024;
		case BATU_BL_32MB: return 32 * 1024 * 1024;
		case BATU_BL_64MB: return 64 * 1024 * 1024;
		case BATU_BL_128MB: return 128 * 1024 * 1024;
		case BATU_BL_256MB: return 256 * 1024 * 1024;
		default: return 0;
	}
	return 0;
}
static void E_MemDumpBATs(void) {
	int i, j, maxBAT;
	char wimgStr[5] = { 'W', 'I', 'M', 'G', '\0' };
	char attr[5];
	char batType;
	uint32_t *batu, *batl;

	if (E_State.consoleType == CONSOLE_TYPE_GAMECUBE)
		maxBAT = 4;
	else if (E_State.consoleType == CONSOLE_TYPE_WII)
		maxBAT = 8;

	puts("MEM: Dumping BATs state");
	batType = 'I';
	batu = E_State.cpu.ibatu;
	batl = E_State.cpu.ibatl;
logBATs:
	for (i = 0; i < maxBAT; i++) {
		printf("%cBAT%dU - Raw: %08X, BEPI (vaddr): 0x%X, BL: 0x%03X (%s), Vs: %d, Vp: %d\n",
			batType,
			i,
			batu[i],
			(batu[i] & BATU_BEPI),
			(batu[i] & BATU_BL) >> BATU_BL_SHIFT,
			E_MemPPCBlockLenToStr((batu[i] & BATU_BL) >> BATU_BL_SHIFT),
			(batu[i] & BATU_VS) >> BATU_VS_SHIFT,
			(batu[i] & BATU_VP) >> BATU_VP_SHIFT
		);

		for (j = 0; j < 4; j++) {
			if (((batl[i] & BATL_WIMG) >> (BATL_WIMG_SHIFT + j) & 1) == 1)
				attr[3 - j] = wimgStr[3 - j];
			else
				attr[3 - j] = '-';
		}
		attr[4] = '\0';
		printf("%cBAT%dL - Raw: %08X, BPRN: 0x%X, Attr: %s, PP: %d\n",
			batType,
			i,
			batl[i],
			(batl[i] & BATL_BPRN) >> BATL_BPRN_SHIFT,
			attr,
			(batl[i] & BATL_PP) >> BATL_PP_SHIFT
		);
	}
	if (batType != 'D') {
		batType = 'D';
		batu = E_State.cpu.dbatu;
		batl = E_State.cpu.dbatl;
		goto logBATs;
	}
}


int E_MemInit(void) {
	int ret, i;

	switch (E_State.consoleType) {
	case CONSOLE_TYPE_GAMECUBE: {
		maxBAT = 4;

		/* 24MB MEM1, no MEM2, 16MB ARAM */
		puts("Creating MEM1...");
		for (i = 0; i > 8; i++) {
			E_State.mem.mem1_map_i[i] = MAP_FAILED;
			E_State.mem.mem1_map_d[i] = MAP_FAILED;
		}
		E_State.mem.mem1_size = 24 * 1024 * 1024;
		E_State.mem.mem1_fd = memfd_create("dol-run_MEM1", MFD_CLOEXEC);
		if (E_State.mem.mem1_fd < 0) return -ENOMEM;
		ret = ftruncate(E_State.mem.mem1_fd, E_State.mem.mem1_size);
		if (ret != 0) return -ENOMEM;

		puts("Allocating ARAM...");
		fflush(stdout);
		E_State.mem.aram_size = 16 * 1024 * 1024;
		E_State.mem.aram = malloc(E_State.mem.aram_size);
		if (!E_State.mem.aram) return -ENOMEM;

		for (i = 0; i > 8; i++) {
			E_State.mem.mem2_map_i[i] = MAP_FAILED;
			E_State.mem.mem2_map_d[i] = MAP_FAILED;
		}
		E_State.mem.mem2_fd = -1;
		E_State.mem.mem2_size = 0;

		puts("Setting default BATs");
		fflush(stdout);
		/* MEM1 Insr */
		E_State.cpu.ibatl[0] = 0x0 /* BPRN */ | (0b0000 << BATL_WIMG_SHIFT) | (BATL_PP_RW << BATL_PP_SHIFT);
		E_State.cpu.ibatu[0] = 0x80000000 /* BEPI */ | (BATU_BL_256MB << BATU_BL_SHIFT) | BATU_VS | BATU_VP;

		/* MEM1 Data - Cached */
		E_State.cpu.dbatl[0] = 0x0 /* BPRN */ | (0b0000 << BATL_WIMG_SHIFT) | (BATL_PP_RW << BATL_PP_SHIFT);
		E_State.cpu.dbatu[0] = 0x80000000 /* BEPI */ | (BATU_BL_256MB << BATU_BL_SHIFT) | BATU_VS | BATU_VP;

		/* MEM1 Data - Uncached */
		E_State.cpu.dbatl[1] = 0x0 /* BPRN */ | (0b0101 << BATL_WIMG_SHIFT) | (BATL_PP_RW << BATL_PP_SHIFT);
		E_State.cpu.dbatu[1] = 0xC0000000 /* BEPI */ | (BATU_BL_256MB << BATU_BL_SHIFT) | BATU_VS | BATU_VP;

		break;
	}
	case CONSOLE_TYPE_WII: {
		maxBAT = 8;

		/* 24MB MEM1, 64MB MEM2, no ARAM */
		puts("Creating MEM1...");
		for (i = 0; i > 8; i++) {
			E_State.mem.mem1_map_i[i] = MAP_FAILED;
			E_State.mem.mem1_map_d[i] = MAP_FAILED;
		}
		E_State.mem.mem1_size = 24 * 1024 * 1024;
		E_State.mem.mem1_fd = memfd_create("dol-run_MEM1", MFD_CLOEXEC);
		if (E_State.mem.mem1_fd < 0) return -ENOMEM;
		ret = ftruncate(E_State.mem.mem1_fd, E_State.mem.mem1_size);
		if (ret != 0) return -ENOMEM;

		E_State.mem.aram_size = 0;
		E_State.mem.aram = NULL;

		puts("Creating MEM2...");
		for (i = 0; i > 8; i++) {
			E_State.mem.mem2_map_i[i] = MAP_FAILED;
			E_State.mem.mem2_map_d[i] = MAP_FAILED;
		}
		E_State.mem.mem2_size = 64 * 1024 * 1024;
		E_State.mem.mem2_fd = memfd_create("dol-run_MEM2", MFD_CLOEXEC);
		if (E_State.mem.mem2_fd < 0) return -ENOMEM;
		ret = ftruncate(E_State.mem.mem2_fd, E_State.mem.mem2_size);
		if (ret != 0) return -ENOMEM;

		puts("Setting default BATs");
		fflush(stdout);
		/* MEM1 Insr */
		E_State.cpu.ibatl[0] = 0x0 /* BPRN */ | (0b0000 << BATL_WIMG_SHIFT) | (BATL_PP_RW << BATL_PP_SHIFT);
		E_State.cpu.ibatu[0] = 0x80000000 /* BEPI */ | (BATU_BL_256MB << BATU_BL_SHIFT) | BATU_VS | BATU_VP;

		/* MEM1 Data - Cached */
		E_State.cpu.dbatl[0] = 0x0 /* BPRN */ | (0b0000 << BATL_WIMG_SHIFT) | (BATL_PP_RW << BATL_PP_SHIFT);
		E_State.cpu.dbatu[0] = 0x80000000 /* BEPI */ | (BATU_BL_256MB << BATU_BL_SHIFT) | BATU_VS | BATU_VP;

		/* MEM1 Data - Uncached */
		E_State.cpu.dbatl[1] = 0x0 /* BPRN */ | (0b0101 << BATL_WIMG_SHIFT) | (BATL_PP_RW << BATL_PP_SHIFT);
		E_State.cpu.dbatu[1] = 0xC0000000 /* BEPI */ | (BATU_BL_256MB << BATU_BL_SHIFT) | BATU_VS | BATU_VP;

		/* MEM2 Insr */
		E_State.cpu.ibatl[4] = 0x10000000 /* BPRN */ | (0b0000 << BATL_WIMG_SHIFT) | (BATL_PP_RW << BATL_PP_SHIFT);
		E_State.cpu.ibatu[4] = 0x90000000 /* BEPI */ | (BATU_BL_256MB << BATU_BL_SHIFT) | BATU_VS | BATU_VP;

		/* MEM2 Data - Cached */
		E_State.cpu.dbatl[4] = 0x10000000 /* BPRN */ | (0b0000 << BATL_WIMG_SHIFT) | (BATL_PP_RW << BATL_PP_SHIFT);
		E_State.cpu.dbatu[4] = 0x90000000 /* BEPI */ | (BATU_BL_256MB << BATU_BL_SHIFT) | BATU_VS | BATU_VP;

		/* MEM2 Data - Uncached */
		E_State.cpu.dbatl[5] = 0x10000000 /* BPRN */ | (0b0101 << BATL_WIMG_SHIFT) | (BATL_PP_RW << BATL_PP_SHIFT);
		E_State.cpu.dbatu[5] = 0xD0000000 /* BEPI */ | (BATU_BL_256MB << BATU_BL_SHIFT) | BATU_VS | BATU_VP;
		break;
	}
	default: {
		return -EINVAL;
	}
	}

	puts("Memory regions created - mapping...");
	fflush(stdout);
	/* MSR hasn't changed since we haven't booted - just pass oldMSR=MSR */
	E_MemMapFixups(E_State.cpu.msr);

	return 0;
}


void E_MemMapFixups(uint32_t old_msr) {
	bool irWasEnabled = (old_msr & MSR_IR) >> MSR_IR_SHIFT;
	bool drWasEnabled = (old_msr & MSR_DR) >> MSR_DR_SHIFT;
	bool irIsEnabled  = (E_State.cpu.msr & MSR_IR) >> MSR_IR_SHIFT;
	bool drIsEnabled  = (E_State.cpu.msr & MSR_DR) >> MSR_DR_SHIFT;
	bool wasTranslating = irWasEnabled && drWasEnabled;
	bool isTranslating = irIsEnabled && drIsEnabled;
	int i;

	printf("E_MemMapFixups entered, oldMSR=0x%08X, MSR=0x%08X oldMSR[IR]=%d, oldMSR[DR]=%d, MSR[IR]=%d, MSR[DR]=%d\n", old_msr, E_State.cpu.msr, irWasEnabled, drWasEnabled, irIsEnabled, drIsEnabled);
	fflush(stdout);

	/* clear the flag */
	E_State.needsMemMapUpdate = false;

	if (irIsEnabled != drIsEnabled) {
		printf("FATAL: MEM: Tried to map with MSR[IR] != MSR[DR]!  IR=%d, DR=%d\n", irIsEnabled, drIsEnabled);
		E_State.fatalError = true;
		return;
	}

	debug_bat(E_MemDumpBATs());

	if (wasTranslating && !isTranslating) {
		/* going to real mode, unmap all BATs */
		puts("MEM: Going from translated to real mode, unmapping all BATs...");
		fflush(stdout);
		for (i = 0; i < maxBAT; i++) {
			if (E_State.mem.mem1_map_i[i] != (void *)-1)
				munmap(E_State.mem.mem1_map_i[i], E_State.mem.mem1_size);
			if (E_State.mem.mem1_map_d[i] != (void *)-1)
				munmap(E_State.mem.mem1_map_d[i], E_State.mem.mem1_size);
			if (E_State.mem.mem2_map_i[i] != (void *)-1)
				munmap(E_State.mem.mem2_map_i[i], E_State.mem.mem2_size);
			if (E_State.mem.mem2_map_d[i] != (void *)-1)
				munmap(E_State.mem.mem2_map_d[i], E_State.mem.mem2_size);
			E_State.mem.mem1_map_i[i] = (void *)-1;
			E_State.mem.mem1_map_d[i] = (void *)-1;
			E_State.mem.mem2_map_i[i] = (void *)-1;
			E_State.mem.mem2_map_d[i] = (void *)-1;
		}

		puts("MEM: Going from translated to real mode, mapping phys addrs...");
		fflush(stdout);
		switch (E_State.consoleType) {
		case CONSOLE_TYPE_GAMECUBE: {
			puts("Mapping MEM1 @ 0x0...");
			fflush(stdout);
			E_State.mem.mem1_map_i[0] = mmap((void *)0x0,
											E_State.mem.mem1_size,
											PROT_READ | PROT_WRITE | PROT_EXEC,
											MAP_SHARED | MAP_FIXED,
											E_State.mem.mem1_fd, 0);
			E_State.mem.mem1_map_d[0] = E_State.mem.mem1_map_i[0];
			if (E_State.mem.mem1_map_i[0] == MAP_FAILED) {
				puts("MEM: GCN: Failed to map phys MEM1");
				E_State.fatalError = true;
				return;
			}
			break;
		}
		case CONSOLE_TYPE_WII: {
			puts("Mapping MEM1 @ 0x0...");
			fflush(stdout);
			E_State.mem.mem1_map_i[0] = mmap((void *)0x0,
											E_State.mem.mem1_size,
											PROT_READ | PROT_WRITE | PROT_EXEC,
											MAP_SHARED | MAP_FIXED,
											E_State.mem.mem1_fd, 0);
			E_State.mem.mem1_map_d[0] = E_State.mem.mem1_map_i[0];
			if (E_State.mem.mem1_map_i[0] == MAP_FAILED) {
				puts("MEM: Wii: Failed to map phys MEM1");
				E_State.fatalError = true;
				return;
			}

			puts("Mapping MEM2 @ 0x10000000...");
			fflush(stdout);
			E_State.mem.mem2_map_i[0] = mmap((void *)0x10000000,
											E_State.mem.mem2_size,
											PROT_READ | PROT_WRITE | PROT_EXEC,
											MAP_SHARED | MAP_FIXED,
											E_State.mem.mem2_fd, 0);
			E_State.mem.mem2_map_d[0] = E_State.mem.mem2_map_i[0];
			if (E_State.mem.mem2_map_i[0] == MAP_FAILED) {
				puts("MEM: Wii: Failed to map phys MEM2");
				E_State.fatalError = true;
				return;
			}
			break;
		}
		default: {
			puts("FATAL: MEM: Invalid console type");
			E_State.fatalError = true;
			return;
		}
		}
	}

	if (isTranslating)
		E_MemReloadMap();

	return;
}

memAddr_t E_MemVirtToPhys(uint32_t vaddr, bool isData) {
	memAddr_t result = {0};
	uint32_t *batu, *batl, size, bepi, phys, pp;
	int i;
	bool vs, vp;

	result.mapped = false;
	result.writable = false;

	batu = isData ? E_State.cpu.dbatu : E_State.cpu.ibatu;
	batl = isData ? E_State.cpu.dbatl : E_State.cpu.ibatl;

	for (i = 0; i < maxBAT; i++) {
		/* Skip inactive BATs (VS and VP bits both 0) */
		vs = (batu[i] & BATU_VS) != 0;
		vp = (batu[i] & BATU_VP) != 0;
		if (!vs && !vp) continue;

		/* Compute BAT block size */
		size = E_MemPPCBlockLenToBytes((batu[i] & BATU_BL) >> BATU_BL_SHIFT);

		/* Compute virtual base */
		bepi = (batu[i] & BATU_BEPI);
		debug_mem(printf("MEM: Checking if vaddr 0x%08X lives in %cBAT%d (%d bytes @ 0x%08X)...\n", vaddr, isData ? 'D' : 'I', i, size, bepi));

		/* Check if VA falls in this BAT */
		if (vaddr >= bepi && vaddr < bepi + size) {
			debug_mem(puts("MEM: It does!"));
			/* Compute physical base */
			phys = ((batl[i] & BATL_BPRN) & ~(size - 1)) + (vaddr - bepi);

			/* Determine access permissions */
			pp = (batl[i] & BATL_PP) >> BATL_PP_SHIFT;
			result.writable = (pp == BATL_PP_RW);
			result.addr = phys;
			result.mapped = true;
			return result;
		}
		debug_mem(puts("MEM: It does not, continuing..."));
	}

	/* No matching BAT */
	return result;
}

memAddr_t E_MemPhysToVirt(uint32_t paddr, bool isData) {
	memAddr_t result = {0};
	uint32_t *batu, *batl, size, bepi, phys, pp;
	int i;
	bool vs, vp;

	result.mapped = false;
	result.writable = false;

	batu = isData ? E_State.cpu.dbatu : E_State.cpu.ibatu;
	batl = isData ? E_State.cpu.dbatl : E_State.cpu.ibatl;

	for (i = 0; i < maxBAT; i++) {
		/* Skip inactive BATs (VS and VP bits both 0) */
		vs = (batu[i] & BATU_VS) != 0;
		vp = (batu[i] & BATU_VP) != 0;
		if (!vs && !vp) continue;

		/* Compute BAT block size */
		size = E_MemPPCBlockLenToBytes((batu[i] & BATU_BL) >> BATU_BL_SHIFT);

		/* Physical base */
		phys = (batl[i] & BATL_BPRN);

		/* Compute virtual base */
		bepi = (batu[i] & BATU_BEPI);
		debug_mem(printf("MEM: Checking if paddr 0x%08X lives in %cBAT%d (%d bytes @ 0x%08X)...\n", paddr, isData ? 'D' : 'I', i, size, bepi));

		/* Check if VA falls in this BAT */
		if (paddr >= phys && paddr < phys + size) {
			debug_mem(puts("MEM: It does!"));

			/* Determine access permissions */
			pp = (batl[i] & BATL_PP) >> BATL_PP_SHIFT;
			result.writable = (pp == BATL_PP_RW);
			result.addr = bepi + ((phys + size) - paddr);
			result.mapped = true;
			printf("MEM: paddr 0x%08X match to vaddr 0x%08X\n", paddr, result.addr);
			return result;
		}
		debug_mem(puts("MEM: It does not, continuing..."));
	}

	/* No matching BAT */
	return result;
}
