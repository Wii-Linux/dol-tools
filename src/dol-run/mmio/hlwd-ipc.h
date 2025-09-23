/*
 * DOL Runner - Emulated hardware - MMIO handling - Hollywood IPC
 * Copyright (C) 2025 Techflash
 */

#ifndef _DOLTOOLS_DOLRUN_MMIO_IPC_H
#define _DOLTOOLS_DOLRUN_MMIO_IPC_H

#include <stdint.h>

struct _ipc_state {
	uint32_t ppcmsg;
	uint32_t ppcctrl;
	uint32_t armmsg;
	uint32_t armctrl;
};

extern void E_MMIO_IPC_Init(void);
extern uint32_t E_MMIO_IPC_Read(uint32_t addr, int accessWidth);
extern void E_MMIO_IPC_Write(uint32_t addr, uint32_t val, int accessWidth);

#endif
