/*
 * DOL Runner - Emulated hardware - CPU handling - 'mtspr' instruction
 * Copyright (C) 2025 Techflash
 */

#include <stdio.h>
#include "common.h"
void E_PPC_Emulate_mtspr(uint32_t spr, uint32_t val) {
	uint32_t pmcNum;
	switch (spr) {
	case SPR_DEC: {
		E_State.cpu.decrementer = val;
		break;
	}
	case SPR_TBL: {
		E_State.cpu.tbl = val;
		break;
	}
	case SPR_TBU: {
		E_State.cpu.tbu = val;
		break;
	}
	case SPR_SRR0: {
		E_State.cpu.srr0 = val;
		break;
	}
	case SPR_SRR1: {
		E_State.cpu.srr1 = val;
		break;
	}
	case SPR_HID0: {
		E_State.cpu.hid0 = val;
		if ((val & HID0_DOZE) != 0 || (val & HID0_NAP) != 0 || (val & HID0_SLEEP) != 0) {
			if ((E_State.cpu.msr & MSR_POW) != 0) {
				puts("FATAL: PPC: Attempted to enter low power (Doze/Nap/Sleep) state with MSR[POW] active");
				E_State.fatalError = true;
			}
			else {
				puts("WARN: PPC: Attempted to set low power (Doze/Nap/Sleep) state, but MSR[POW] is cleared.");
				puts("WARN: PPC: This will become fatal if MSR[POW] gets set.");
			}

			break;
		}
		break;
	}
	case SPR_HID1: {
		puts("FATAL: PPC: Attempted to write to HID1 (read-only)");
		E_State.fatalError = true;
		break;
	}
	case SPR_HID2: {
		if ((val & HID2_RSRVD0) != 0) {
			puts("WARN: PPC: Attempt to write to reserved HID2 bits blocked");
			val &= ~HID2_RSRVD0;
		}
		if ((val & HID2_DMAQL) != 0) {
			puts("WARN: PPC: Attempt to write to HID2 DMAQL bits blocked");

			/* unset all bits of DMAQL*/
			val &= ~HID2_DMAQL;

			/* set DMAQL bits from existing state */
			val |= (E_State.cpu.hid2 & HID2_DMAQL);
		}

		E_State.cpu.hid2 = val;
		break;
	}
	case SPR_HID4: {
		/* HID4 is illegal on Gekko */
		if (E_State.consoleType == CONSOLE_TYPE_GAMECUBE) {
			puts("FATAL: PPC: Attempt to write to HID4 on Gekko (only supported on Broadway)");
			E_State.fatalError = true;
			break;
		}

		if ((val & HID4_RSRVD1) != 1) {
			puts("WARN: PPC: Attempt to set HID4 bit 0 (reserved, always 1) to 0");
			val |= HID4_RSRVD1;
		}
		if ((val & HID4_RSRVD0) != 0) {
			puts("WARN: PPC: Attempt to set HID4 reserved (always 0) bits to 1");
			val &= ~HID4_RSRVD0;
		}
		E_State.cpu.hid4 = val;
		break;
	}
	case SPR_SPRG0:
	case SPR_SPRG1:
	case SPR_SPRG2:
	case SPR_SPRG3: {
		E_State.cpu.sprg[spr - SPR_SPRG0] = val;
		break;
	}
	case SPR_L2CR: {
		if ((val & L2CR_RSRVD0) != 0) {
			puts("WARN: PPC: Attempt to set L2CR reserved (always 0) bits to 1");
			val &= ~L2CR_RSRVD0;
		}
		E_State.cpu.l2cr = val;
		break;
	}
	case SPR_UMMCR0:
	case SPR_UMMCR1: {
		puts("FATAL: PPC: Attempted to write to UMMCR[0/1] (read-only)");
		E_State.fatalError = true;
		break;
	}
	case SPR_MMCR0: {
		if (val != 0) {
			puts("FATAL: PPC: Attempted to write non-zero value to MMCR0 - this is not supported");
			E_State.fatalError = true;
			break;
		}

		E_State.cpu.mmcr0 = val;
		break;
	}
	case SPR_MMCR1: {
		if (val != 0) {
			puts("FATAL: PPC: Attempted to write non-zero value to MMCR1 - this is not supported");
			E_State.fatalError = true;
			break;
		}

		E_State.cpu.mmcr1 = val;
		break;
	}
	case SPR_UPMC1:
	case SPR_UPMC2:
	case SPR_UPMC3:
	case SPR_UPMC4: {
		pmcNum = spr - SPR_UPMC1;
		if (pmcNum > 1) pmcNum = spr - SPR_UPMC3;
		printf("FATAL: PPC: Attempted to write to UPMC%d (read-only)\n", pmcNum);
		E_State.fatalError = true;
		break;
	}
	case SPR_PMC1:
	case SPR_PMC2:
	case SPR_PMC3:
	case SPR_PMC4: {
		pmcNum = spr - SPR_PMC1;
		if (pmcNum > 1) pmcNum = spr - SPR_PMC3;
		if (val != 0) {
			printf("FATAL: PPC: Attempted to write non-zero value to PMC%d - this is not supported\n", pmcNum);
			E_State.fatalError = true;
			break;
		}

		E_State.cpu.pmc[pmcNum] = val;
		break;
	}
	case SPR_DBAT0U:
	case SPR_DBAT0L:
	case SPR_DBAT1U:
	case SPR_DBAT1L:
	case SPR_DBAT2U:
	case SPR_DBAT2L:
	case SPR_DBAT3U:
	case SPR_DBAT3L: {
		E_State.needsMemMapUpdate = true;
		if ((spr % 2) == 0)
			E_State.cpu.dbatu[(spr - SPR_DBAT0U) / 2] = val;
		else
			E_State.cpu.dbatl[(spr - SPR_DBAT0L) / 2] = val;

		break;
	}
	case SPR_DBAT4U:
	case SPR_DBAT4L:
	case SPR_DBAT5U:
	case SPR_DBAT5L:
	case SPR_DBAT6U:
	case SPR_DBAT6L:
	case SPR_DBAT7U:
	case SPR_DBAT7L: {
		E_State.needsMemMapUpdate = true;
		if (E_PPC_Validate_HighBATAccess("write", 'D'))
			break;

		if ((spr % 2) == 0)
			E_State.cpu.dbatu[(spr - SPR_DBAT4U) / 2] = val;
		else
			E_State.cpu.dbatl[(spr - SPR_DBAT4L) / 2] = val;
		break;
	}
	case SPR_IBAT0U:
	case SPR_IBAT0L:
	case SPR_IBAT1U:
	case SPR_IBAT1L:
	case SPR_IBAT2U:
	case SPR_IBAT2L:
	case SPR_IBAT3U:
	case SPR_IBAT3L: {
		E_State.needsMemMapUpdate = true;
		if ((spr % 2) == 0)
			E_State.cpu.ibatu[(spr - SPR_IBAT0U) / 2] = val;
		else
			E_State.cpu.ibatl[(spr - SPR_IBAT0L) / 2] = val;

		break;
	}
	case SPR_IBAT4U:
	case SPR_IBAT4L:
	case SPR_IBAT5U:
	case SPR_IBAT5L:
	case SPR_IBAT6U:
	case SPR_IBAT6L:
	case SPR_IBAT7U:
	case SPR_IBAT7L: {
		E_State.needsMemMapUpdate = true;
		if (E_PPC_Validate_HighBATAccess("write", 'D'))
			break;

		if ((spr % 2) == 0)
			E_State.cpu.ibatu[(spr - SPR_IBAT4U) / 2] = val;
		else
			E_State.cpu.ibatl[(spr - SPR_IBAT4L) / 2] = val;
		break;
	}
	default: {
		printf("FATAL: PPC: Attempted write to unknown SPR %d\n", spr);
		E_State.fatalError = true;
		break;
	}
	}

	return;
}
