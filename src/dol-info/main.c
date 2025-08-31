/*
 * DOL Info - Main source
 * Copyright (C) 2025 Techflash
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../common/swap.h"
#include "../common/types.h"

static void usage(char *name) {
	printf("Usage: %s <file>\n", name);
	exit(1);
}

int main(int argc, char *argv[]) {
	int fd, ret, i;
	char errBuf[256];
	DOL_Hdr_t hdr;

	if (argc != 2 || argv[1][0] == '\0')
		usage(argv[0]); /* bad args */

	/* try to open the file */
	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		sprintf(errBuf, "Failed to open %s", argv[1]);
		perror(errBuf);
		return 1;
	}

	ret = read(fd, &hdr, 0x100);
	close(fd);

	if (ret != 0x100) {
		perror("Failed to read data");
		return 1;
	}

	/* byteswap */
	DOL_Hdr_Byteswap(&hdr);

	puts(
		"=== DOL Info ===\n"
	);
	puts("  Text sections:");
	for (i = 0; i < 7; i++)
		printf("    %d: Address: 0x%08X, Size: 0x%X, File Offset: 0x%X\n", i, hdr.textAddr[i], hdr.textSize[i], hdr.textOff[i]);

	puts("  Data sections:");
	for (i = 0; i < 11; i++)
		printf("    %d: Address: 0x%08X, Size: 0x%X, File Offset: 0x%X\n", i, hdr.dataAddr[i], hdr.dataSize[i], hdr.dataOff[i]);
	printf("BSS Address: 0x%08X\n", hdr.bssAddr);
	printf("BSS Size: 0x%08X\n", hdr.bssSize);
	printf("Entry point: 0x%08X\n", hdr.entry);

	return 0;
}
