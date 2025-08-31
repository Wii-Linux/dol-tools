/*
 * DOL Runner - Emulated hardware - CPU handling - Common
 * Copyright (C) 2025 Techflash
 */

#ifndef _DOLTOOLS_DOLRUN_CPU_COMMON_H
#define _DOLTOOLS_DOLRUN_CPU_COMMON_H

#include <stdint.h>
#include "../cpu.h"
#include "../emu.h"
extern void E_PPC_Validate_MSR(void);
extern int E_PPC_Validate_HighBATAccess(char *op, char type);
#endif
