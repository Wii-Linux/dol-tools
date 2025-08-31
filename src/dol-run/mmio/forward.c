/*
 * DOL Runner - Emulated hardware - MMIO handling - Forwarding support
 * Copyright (C) 2025 Techflash
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "../emu.h"

static int fd = -1;
static uint32_t len = 0x00F00000;
static uint32_t *memFlipper = NULL;
static uint32_t *memHlwd = NULL;

int E_MMIO_Forwarding_Init(void) {
	fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd < 0) {
		perror("FATAL: MMIO: open() on /dev/mem failed");
		E_State.fatalError = true;
		return -1;
	}

	memFlipper = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x0C000000);
	if (memFlipper == MAP_FAILED) {
		perror("FATAL: MMIO: mmap() on /dev/mem for Flipper registers failed");
		E_State.fatalError = true;
		return -1;
	}

	if (E_State.hostType == CONSOLE_TYPE_WII) {
		memHlwd = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x0D000000);
		if (memHlwd == MAP_FAILED) {
			perror("FATAL: MMIO: mmap() on /dev/mem for Hollywood registers failed");
			E_State.fatalError = true;
			return -1;
		}
	}

	return 0;
}

void E_MMIO_Forwarding_Cleanup(void) {
	if (memFlipper)
		munmap(memFlipper, len);
	if (memHlwd)
		munmap(memHlwd, len);

	if (fd > 0)
		close(fd);
}

uint32_t E_MMIO_Forwarded_Read(uint32_t addr, int accessWidth) {
	uint32_t tmp, off, val, *u32ptr;
	uint16_t *u16ptr;
	uint8_t  *u8ptr;
	tmp = addr & 0xFFF70000;
	off = addr & ~(0xFFF70000);
	if (tmp == 0x0C000000)
		tmp = (uint32_t)memFlipper;
	else if (tmp == 0x0D000000)
		tmp = (uint32_t)memHlwd;
	else {
		printf("FATAL: MMIO: Trying to read from bogus address 0x%08X\n", addr);
		E_State.fatalError = true;
		return 0;
	}

	/* tmp has our memory region, off has the offset into the region */
	tmp += off;
	u32ptr = (uint32_t *)tmp;
	u16ptr = (uint16_t *)tmp;
	u8ptr = (uint8_t *)tmp;

	if (accessWidth == 4)
		val = *u32ptr;
	else if (accessWidth == 2)
		val = *u16ptr;
	else if (accessWidth == 1)
		val = *u8ptr;

	return val;
}

void E_MMIO_Forwarded_Write(uint32_t addr, uint32_t val, int accessWidth) {
	uint32_t tmp, off, *u32ptr;
	uint16_t *u16ptr;
	uint8_t  *u8ptr;
	tmp = addr & 0xFFF70000;
	off = addr & ~(0xFFF70000);
	if (tmp == 0x0C000000)
		tmp = (uint32_t)memFlipper;
	else if (tmp == 0x0D000000)
		tmp = (uint32_t)memHlwd;
	else {
		printf("FATAL: MMIO: Trying to write to bogus address 0x%08X\n", addr);
		E_State.fatalError = true;
		return;
	}

	/* tmp has our memory region, off has the offset into the region */
	tmp += off;
	u32ptr = (uint32_t *)tmp;
	u16ptr = (uint16_t *)tmp;
	u8ptr = (uint8_t *)tmp;

	if (accessWidth == 4)
		*u32ptr = val;
	else if (accessWidth == 2)
		*u16ptr = val & 0x0000FFFF;
	else if (accessWidth == 1)
		*u8ptr = val & 0x000000FF;

	return;
}
