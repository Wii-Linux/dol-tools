/*
 * DOL Tools - Byteswapping for header
 * Copyright (C) 2025 Techflash
 */

#ifndef _DOLTOOLS_COMMON_SWAP_H
#define _DOLTOOLS_COMMON_SWAP_H
#include "types.h"
#include <arpa/inet.h>
static void DOL_Hdr_Byteswap(DOL_Hdr_t *hdr) {
	int i;

	for (i = 0; i < 7; i++)
		hdr->textOff[i] = ntohl(hdr->textOff[i]);
	for (i = 0; i < 11; i++)
		hdr->dataOff[i] = ntohl(hdr->dataOff[i]);
	for (i = 0; i < 7; i++)
		hdr->textAddr[i] = ntohl(hdr->textAddr[i]);
	for (i = 0; i < 11; i++)
		hdr->dataAddr[i] = ntohl(hdr->dataAddr[i]);
	for (i = 0; i < 7; i++)
		hdr->textSize[i] = ntohl(hdr->textSize[i]);
	for (i = 0; i < 11; i++)
		hdr->dataSize[i] = ntohl(hdr->dataSize[i]);
	hdr->bssAddr = ntohl(hdr->bssAddr);
	hdr->bssSize = ntohl(hdr->bssSize);
	hdr->entry = ntohl(hdr->entry);
}

#endif
