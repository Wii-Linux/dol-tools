/*
 * DOL Tools - Common types
 * Copyright (C) 2025 Techflash
 */

#ifndef _DOLTOOLS_COMMON_TYPES_H
#define _DOLTOOLS_COMMON_TYPES_H
#include <stdint.h>

typedef struct {
	/* Offset in bytes within file to text/data section [n] */
	uint32_t textOff[7];
	uint32_t dataOff[11];


	/* Desired load address for text/data section [n] */
	uint32_t textAddr[7];
	uint32_t dataAddr[11];

	/* Size of text/data section [n] */
	uint32_t textSize[7];
	uint32_t dataSize[11];

	/* BSS section address */
	uint32_t bssAddr;

	/* BSS section size */
	uint32_t bssSize;

	/* program entry point */
	uint32_t entry;

	uint8_t padding[0x1C];
} __attribute__((packed)) DOL_Hdr_t;

#endif
