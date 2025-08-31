/*
 * DOL Runner - Emulated hardware - CPU handling - 'mfspr' instruction
 * Copyright (C) 2025 Techflash
 */

#include <stdio.h>
#include "common.h"
uint32_t E_PPC_Emulate_mfspr(uint32_t spr) {
	switch (spr) {
	case SPR_DEC:
		return E_State.cpu.decrementer;
	case SPR_TBL:
		return E_State.cpu.tbl;
	case SPR_TBU:
		return E_State.cpu.tbu;
	case SPR_SRR0:
		return E_State.cpu.srr0;
	case SPR_SRR1:
		return E_State.cpu.srr1;
	case SPR_HID0:
		return E_State.cpu.hid0;
	case SPR_HID1:
		return E_State.cpu.hid1;
	case SPR_HID2:
		return E_State.cpu.hid2;
	case SPR_HID4: {
		/* HID4 is illegal on Gekko */
		if (E_State.consoleType == CONSOLE_TYPE_GAMECUBE) {
			puts("FATAL: PPC: Attempt to read to HID4 on Gekko (only supported on Broadway)");
			E_State.fatalError = true;
			return (uint32_t)-1;
		}
		return E_State.cpu.hid4;
	}
	case SPR_SPRG0:
	case SPR_SPRG1:
	case SPR_SPRG2:
	case SPR_SPRG3: {
		return E_State.cpu.sprg[spr - SPR_SPRG0];
		break;
	}
	case SPR_L2CR:
		return E_State.cpu.l2cr;
	case SPR_UMMCR0:
	case SPR_MMCR0:
		return E_State.cpu.mmcr0;
	case SPR_UMMCR1:
	case SPR_MMCR1:
		return E_State.cpu.mmcr1;
	case SPR_DBAT0U:
		return E_State.cpu.dbatu[0];
	case SPR_DBAT0L:
		return E_State.cpu.dbatl[0];
	case SPR_DBAT1U:
		return E_State.cpu.dbatu[1];
	case SPR_DBAT1L:
		return E_State.cpu.dbatl[1];
	case SPR_DBAT2U:
		return E_State.cpu.dbatu[2];
	case SPR_DBAT2L:
		return E_State.cpu.dbatl[2];
	case SPR_DBAT3U:
		return E_State.cpu.dbatu[3];
	case SPR_DBAT3L:
		return E_State.cpu.dbatl[3];
	case SPR_DBAT4U:
		return E_PPC_Validate_HighBATAccess("read", 'D') ? (uint32_t)-1 : E_State.cpu.dbatu[4];
	case SPR_DBAT4L:
		return E_PPC_Validate_HighBATAccess("read", 'D') ? (uint32_t)-1 : E_State.cpu.dbatl[4];
	case SPR_DBAT5U:
		return E_PPC_Validate_HighBATAccess("read", 'D') ? (uint32_t)-1 : E_State.cpu.dbatu[5];
	case SPR_DBAT5L:
		return E_PPC_Validate_HighBATAccess("read", 'D') ? (uint32_t)-1 : E_State.cpu.dbatl[5];
	case SPR_DBAT6U:
		return E_PPC_Validate_HighBATAccess("read", 'D') ? (uint32_t)-1 : E_State.cpu.dbatu[6];
	case SPR_DBAT6L:
		return E_PPC_Validate_HighBATAccess("read", 'D') ? (uint32_t)-1 : E_State.cpu.dbatl[6];
	case SPR_DBAT7U:
		return E_PPC_Validate_HighBATAccess("read", 'D') ? (uint32_t)-1 : E_State.cpu.dbatu[7];
	case SPR_DBAT7L:
		return E_PPC_Validate_HighBATAccess("read", 'D') ? (uint32_t)-1 : E_State.cpu.dbatl[7];
	case SPR_IBAT0U:
		return E_State.cpu.ibatu[0];
	case SPR_IBAT0L:
		return E_State.cpu.ibatl[0];
	case SPR_IBAT1U:
		return E_State.cpu.ibatu[1];
	case SPR_IBAT1L:
		return E_State.cpu.ibatl[1];
	case SPR_IBAT2U:
		return E_State.cpu.ibatu[2];
	case SPR_IBAT2L:
		return E_State.cpu.ibatl[2];
	case SPR_IBAT3U:
		return E_State.cpu.ibatu[3];
	case SPR_IBAT3L:
		return E_State.cpu.ibatl[3];
	case SPR_IBAT4U:
		return E_PPC_Validate_HighBATAccess("read", 'I') ? (uint32_t)-1 : E_State.cpu.ibatu[4];
	case SPR_IBAT4L:
		return E_PPC_Validate_HighBATAccess("read", 'I') ? (uint32_t)-1 : E_State.cpu.ibatl[4];
	case SPR_IBAT5U:
		return E_PPC_Validate_HighBATAccess("read", 'I') ? (uint32_t)-1 : E_State.cpu.ibatu[5];
	case SPR_IBAT5L:
		return E_PPC_Validate_HighBATAccess("read", 'I') ? (uint32_t)-1 : E_State.cpu.ibatl[5];
	case SPR_IBAT6U:
		return E_PPC_Validate_HighBATAccess("read", 'I') ? (uint32_t)-1 : E_State.cpu.ibatu[6];
	case SPR_IBAT6L:
		return E_PPC_Validate_HighBATAccess("read", 'I') ? (uint32_t)-1 : E_State.cpu.ibatl[6];
	case SPR_IBAT7U:
		return E_PPC_Validate_HighBATAccess("read", 'I') ? (uint32_t)-1 : E_State.cpu.ibatu[7];
	case SPR_IBAT7L:
		return E_PPC_Validate_HighBATAccess("read", 'I') ? (uint32_t)-1 : E_State.cpu.ibatl[7];
	default: {
		printf("FATAL: PPC: Attempted read from unknown SPR %d\n", spr);
		E_State.fatalError = true;
		break;
	}
	}

	return (uint32_t)-1;
}
