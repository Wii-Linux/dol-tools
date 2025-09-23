/*
 * DOL Runner - Emulated hardware - MMIO handling - Global Flipper/Hollywood chipset state
 * Copyright (C) 2025 Techflash
 */
#include "chipset.h"
#include "vi.h"
#include "mi.h"
#include "pi.h"
#include "dsp.h"
#include "ai.h"
#include "forward.h"

int E_MMIO_Chipset_Init(void) {
	E_MMIO_VI_Init();
	E_MMIO_PI_Init();
	E_MMIO_MI_Init();
	E_MMIO_DSP_Init();
	E_MMIO_AI_Init();
	/*E_MMIO_IPC_Init();*/
	E_MMIO_Forwarding_Init();
	return 0;
}
