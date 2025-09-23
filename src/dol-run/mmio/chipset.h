/*
 * DOL Runner - Emulated hardware - MMIO handling - Global Flipper/Hollywood chipset state
 * Copyright (C) 2025 Techflash
 */

#ifndef _DOLTOOLS_DOLRUN_MMIO_CHIPSET_H
#define _DOLTOOLS_DOLRUN_MMIO_CHIPSET_H

#include <stdint.h>
#include "pi.h"
#include "mi.h"
#include "vi.h"
#include "dsp.h"
#include "ai.h"
#include "hlwd-ipc.h"

struct _chipset_state {
	struct _pi_state pi;
	struct _mi_state mi;
	struct _vi_state vi;
	struct _dsp_state dsp;
	struct _ai_state ai;
	struct _ipc_state ipc;
};
extern int E_MMIO_Chipset_Init(void);

#endif
