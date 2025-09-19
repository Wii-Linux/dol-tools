/*
 * DOL Runner - Emulated hardware - Memory handling
 * Copyright (C) 2025 Techflash
 */

#ifndef _DOLTOOLS_DOLRUN_MEM_H
#define _DOLTOOLS_DOLRUN_MEM_H
#include <stdint.h>
#include <stdbool.h>

/* Constants */
#define MEM1_PADDR 0x00000000
#define MEM2_PADDR 0x10000000
#define FLIPPER_PADDR 0x0C000000
#define HOLLYWOOD_PADDR 0x0D000000
#define HOLLYWOOD_MIRR_PADDR 0x0D800000

struct _memState {
	uint8_t *mem1_map_i[8]; /* MEM1 / "Splash" / "Napa" - mappings (Instr) */
	uint8_t *mem1_map_d[8]; /* MEM1 / "Splash" / "Napa" - mappings (Data) */
	uint32_t mem1_size;     /* MEM1 / "Splash" / "Napa" - size */
	int      mem1_fd;       /* MEM1 / "Splash" / "Napa" - memfd */
	uint8_t *mem2_map_i[8]; /* MEM2 / GDDR3 / DDR - mappings (Instr) */
	uint8_t *mem2_map_d[8]; /* MEM2 / GDDR3 / DDR - mappings (Data) */
	uint32_t mem2_size;     /* MEM2 / GDDR3 / DDR - size */
	int      mem2_fd;       /* MEM2 / GDDR3 / DDR - memfd */
	uint8_t *aram;          /* ARAM / Auxilary RAM */
	uint32_t aram_size;     /* ARAM / Auxilary RAM - size */
};

/*
 * Struct representing a given memory address
 */
typedef struct {
	uint32_t addr;
	bool mapped;
	bool writable;
} memAddr_t;

/*
 * Initialize the emulated system memory.
 * Returns 0 on success, negative value on error.
 * errno may contain the relevant error, for example
 * if malloc() failed.
 */
extern int E_MemInit(void);


/*
 * Fix up the memory mappings for use in the current state.
 * Takes the old MSR value as a parameter (to identify what the old state was).
 * Takes the new MSR value in E_State.
 * Sets E_State.fatalError in the event of a mapping error.
 */
extern void E_MemMapFixups(uint32_t old_msr);

/*
 * Translate a virtual address into a physical
 * one, using the CPU's BATs.
 */
extern memAddr_t E_MemVirtToPhys(uint32_t vaddr, bool isData);

/*
 * Get a valid mapped address for a given physical address.
 */
extern memAddr_t E_MemPhysToVirt(uint32_t paddr, bool isData);
#endif
