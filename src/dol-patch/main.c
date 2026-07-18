/*
 * DOL Patch - Main source
 * Copyright (C) 2026 Techflash
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stddef.h>
#include "../common/types.h"

static void usage(const char *name) {
	fprintf(stderr,
		"Usage: %s <file> <field> <32-bit hex value>\n"
		"Fields:\n"
		"  textN_addr, textN_size, textN_offset  (N = 0..6)\n"
		"  dataN_addr, dataN_size, dataN_offset  (N = 0..10)\n"
		"  bss_addr, bss_size, entry\n",
		name);
}

static int parseIndexedField(const char *field, const char *prefix, int count, size_t addrBase, size_t sizeBase, size_t offsetBase, size_t *offset) {
	const char *p;
	char *end;
	long index;

	if (strncmp(field, prefix, strlen(prefix)) != 0)
		return 0;

	p = field + strlen(prefix);
	if (*p < '0' || *p > '9')
		return -1;

	errno = 0;
	index = strtol(p, &end, 10);
	if (errno != 0 || index < 0 || index >= count)
		return -1;

	if (strcmp(end, "_addr") == 0)
		*offset = addrBase + (size_t)index * sizeof(uint32_t);
	else if (strcmp(end, "_size") == 0)
		*offset = sizeBase + (size_t)index * sizeof(uint32_t);
	else if (strcmp(end, "_offset") == 0)
		*offset = offsetBase + (size_t)index * sizeof(uint32_t);
	else
		return -1;

	return 1;
}

static int fieldOffset(const char *field, size_t *offset) {
	int ret;

	ret = parseIndexedField(field, "text", 7,
		offsetof(DOL_Hdr_t, textAddr), offsetof(DOL_Hdr_t, textSize),
		offsetof(DOL_Hdr_t, textOff), offset);
	if (ret != 0)
		return ret > 0;

	ret = parseIndexedField(field, "data", 11,
		offsetof(DOL_Hdr_t, dataAddr), offsetof(DOL_Hdr_t, dataSize),
		offsetof(DOL_Hdr_t, dataOff), offset);
	if (ret != 0)
		return ret > 0;

	if (strcmp(field, "bss_addr") == 0)
		*offset = offsetof(DOL_Hdr_t, bssAddr);
	else if (strcmp(field, "bss_size") == 0)
		*offset = offsetof(DOL_Hdr_t, bssSize);
	else if (strcmp(field, "entry") == 0)
		*offset = offsetof(DOL_Hdr_t, entry);
	else
		return 0;

	return 1;
}

static int parseValue(const char *text, uint32_t *value) {
	const char *digits;
	char *end;
	unsigned long parsed;
	size_t i, length;

	digits = text;
	if (digits[0] == '0' && (digits[1] == 'x' || digits[1] == 'X'))
		digits += 2;
	length = strlen(digits);
	if (length == 0 || length > 8)
		return 0;
	for (i = 0; i < length; i++) {
		if (!((digits[i] >= '0' && digits[i] <= '9') ||
			(digits[i] >= 'a' && digits[i] <= 'f') ||
			(digits[i] >= 'A' && digits[i] <= 'F')))
			return 0;
	}

	errno = 0;
	parsed = strtoul(digits, &end, 16);
	if (errno != 0 || *end != '\0' || parsed > UINT32_MAX)
		return 0;

	*value = (uint32_t)parsed;
	return 1;
}

int main(int argc, char *argv[]) {
	int fd;
	size_t offset;
	uint32_t value, diskValue;
	ssize_t written;
	struct stat fileInfo;

	if (argc != 4 || argv[1][0] == '\0' || argv[2][0] == '\0' ||
		argv[3][0] == '\0') {
		usage(argv[0]);
		return 1;
	}

	if (!fieldOffset(argv[2], &offset)) {
		fprintf(stderr, "Invalid field: %s\n", argv[2]);
		usage(argv[0]);
		return 1;
	}
	if (!parseValue(argv[3], &value)) {
		fprintf(stderr, "Invalid 32-bit hexadecimal value: %s\n", argv[3]);
		return 1;
	}

	fd = open(argv[1], O_WRONLY);
	if (fd < 0) {
		perror(argv[1]);
		return 1;
	}
	if (fstat(fd, &fileInfo) < 0) {
		perror("Failed to inspect DOL file");
		close(fd);
		return 1;
	}
	if (fileInfo.st_size < (off_t)sizeof(DOL_Hdr_t)) {
		fprintf(stderr, "%s: file is too small to contain a DOL header\n",
			argv[1]);
		close(fd);
		return 1;
	}

	diskValue = htonl(value);
	written = pwrite(fd, &diskValue, sizeof(diskValue), (off_t)offset);
	if (written != (ssize_t)sizeof(diskValue)) {
		if (written >= 0)
			errno = EIO;
		perror("Failed to patch DOL header");
		close(fd);
		return 1;
	}
	if (close(fd) < 0) {
		perror("Failed to close DOL file");
		return 1;
	}

	return 0;
}
