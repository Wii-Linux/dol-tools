/*
 * DOL Runner - Emulated hardware - CPU handling
 * Copyright (C) 2025 Techflash
 */

#ifndef _DOLTOOLS_DOLRUN_CPU_H
#define _DOLTOOLS_DOLRUN_CPU_H

#include <stdint.h>

/*
 * Strictly adhere to the 750CL manual, rather than
 * what Broadway actually does
 */
/* #define STRICT_750CL */

/*
 * SPR numbers
 */
/* Decrementer */
#define SPR_DEC         22
/* Timebase */
#define SPR_TBL         284
#define SPR_TBU         285
/* SRRn */
#define SPR_SRR0        26
#define SPR_SRR1        27
/* HIDn */
#define SPR_HID0        1008
#define SPR_HID1        1009
#define SPR_HID2        920
#define SPR_HID4        1011
/* IBATn[U/L] (Gekko/Broadway) */
#define SPR_IBAT0U      528
#define SPR_IBAT0L      529
#define SPR_IBAT1U      530
#define SPR_IBAT1L      531
#define SPR_IBAT2U      532
#define SPR_IBAT2L      533
#define SPR_IBAT3U      534
#define SPR_IBAT3L      535
/* DBATn[U/L] (Gekko/Broadway) */
#define SPR_DBAT0U      536
#define SPR_DBAT0L      537
#define SPR_DBAT1U      538
#define SPR_DBAT1L      539
#define SPR_DBAT2U      540
#define SPR_DBAT2L      541
#define SPR_DBAT3U      542
#define SPR_DBAT3L      543
/* IBATn[U/L] (Broadway) */
#define SPR_IBAT4U      560
#define SPR_IBAT4L      561
#define SPR_IBAT5U      562
#define SPR_IBAT5L      563
#define SPR_IBAT6U      564
#define SPR_IBAT6L      565
#define SPR_IBAT7U      566
#define SPR_IBAT7L      567
/* DBATn[U/L] (Broadway) */
#define SPR_DBAT4U      568
#define SPR_DBAT4L      569
#define SPR_DBAT5U      570
#define SPR_DBAT5L      571
#define SPR_DBAT6U      572
#define SPR_DBAT6L      573
#define SPR_DBAT7U      574
#define SPR_DBAT7L      575
/* Misc */
#define SPR_SPRG0       272
#define SPR_SPRG1       273
#define SPR_SPRG2       274
#define SPR_SPRG3       275
#define SPR_L2CR        1017
#define SPR_UMMCR0      936
#define SPR_UPMC1       937
#define SPR_UPMC2       938
#define SPR_USIA        939
#define SPR_UMMCR1      940
#define SPR_UPMC3       941
#define SPR_UPMC4       942
#define SPR_MMCR0       952
#define SPR_PMC1        953
#define SPR_PMC2        954
#define SPR_SIA         955
#define SPR_MMCR1       956
#define SPR_PMC3        957
#define SPR_PMC4        958

/*
 * HID0 settings
 */
#define HID0_DOZE_SHIFT (32 - 8)
#define HID0_DOZE       (1 << HID0_DOZE_SHIFT)
#define HID0_NAP_SHIFT  (32 - 9)
#define HID0_NAP        (1 << HID0_NAP_SHIFT)
#define HID0_SLEEP_SHIFT (32 - 10)
#define HID0_SLEEP      (1 << HID0_SLEEP_SHIFT)

/*
 * HID2 settings
 */
#ifdef STRICT_750CL
	#define HID2_RSRVD1_SHIFT (31 - 0)
	#define HID2_RSRVD1       (1 << HID2_RSRVD1_SHIFT)
#else
	#define HID2_RSRVD1_SHIFT (0)
	#define HID2_RSRVD1       (0)
	#define HID2_LSQE_SHIFT   (31 - 0)
	#define HID2_LSQE         (1 << HID2_LSQE_SHIFT)
#endif
#define HID2_RSRVD0_SHIFT     (31 - 31)
#define HID2_RSRVD0           (65535 << HID2_RSRVD0_SHIFT)
#define HID2_WPE_SHIFT        (31 - 1)
#define HID2_WPE              (1 << HID2_WPE_SHIFT)
#define HID2_DMAQL_SHIFT      (31 - 7)
#define HID2_DMAQL            (15 << HID2_DMAQL_SHIFT)

/*
 * HID4 settings
 */
#define HID4_RSRVD1_SHIFT  (31 - 0)
#define HID4_RSRVD1        (1 << HID4_RSRVD1_SHIFT)
#define HID4_SBE_SHIFT     (31 - 6)
#define HID4_SBE           (1 << HID4_SBE_SHIFT)
#ifdef STRICT_750CL
	#define HID4_PS1_CTL_SHIFT (31 - 7)
	#define HID4_PS1_CTL       (1 << HID4_PS1_CTL_SHIFT)
	#define HID4_PS2_CTL_SHIFT (31 - 12)
	#define HID4_PS2_CTL       (1 << HID4_PS2_CTL_SHIFT)
	#define HID4_RSRVD0        ((1 << (31 - 8)) | 0x7FFFF)
#else
	#define HID4_RSRVD0        ((1 << (31 - 5)) | 0xFFFFF)
#endif

/*
 * L2CR settings
 */
#define L2CR_L2E_SHIFT     (31 - 0)
#define L2CR_L2E           (1 << L2CR_L2E_SHIFT)
#define L2CR_L2CE_SHIFT    (31 - 1)
#define L2CR_L2CE          (1 << L2CR_L2CE_SHIFT)
#define L2CR_RSRVD0_SHIFT1 (31 - 8)
#define L2CR_L2DO_SHIFT    (31 - 9)
#define L2CR_L2DO          (1 << L2CR_L2DO_SHIFT)
#define L2CR_L2I_SHIFT     (31 - 10)
#define L2CR_L2I           (1 << L2CR_L2I_SHIFT)
#define L2CR_RSRVD0_SHIFT2 (31 - 11)
#define L2CR_L2WT_SHIFT    (31 - 12)
#define L2CR_L2WT          (1 << L2CR_L2WT_SHIFT)
#define L2CR_L2TS_SHIFT    (31 - 13)
#define L2CR_L2TS          (1 << L2CR_L2TS_SHIFT)
#define L2CR_RSRVD0_SHIFT3 (31 - 30)
#define L2CR_RSRVD0        ((127 << L2CR_RSRVD0_SHIFT1) | (1 << L2CR_RSRVD0_SHIFT2) | (131071 << L2CR_RSRVD0_SHIFT3))
#define L2CR_L2IP_SHIFT    (31 - 31)
#define L2CR_L2IP          (1 << L2CR_L2IP_SHIFT)

/*
 * BAT values
 */
#define BATU_BEPI_SHIFT (31 - 14)
#define BATU_BEPI       (32767 << BATU_BEPI_SHIFT)
#define BATU_BL_SHIFT   (31 - 29)
#define BATU_BL         (2047 << BATU_BL_SHIFT)
#define BATU_BL_128KB   0x000
#define BATU_BL_256KB   0x001
#define BATU_BL_512KB   0x003
#define BATU_BL_1MB     0x007
#define BATU_BL_2MB     0x00F
#define BATU_BL_4MB     0x01F
#define BATU_BL_8MB     0x03F
#define BATU_BL_16MB    0x07F
#define BATU_BL_32MB    0x0FF
#define BATU_BL_64MB    0x1FF
#define BATU_BL_128MB   0x3FF
#define BATU_BL_256MB   0x7FF
#define BATU_VS_SHIFT   (31 - 30)
#define BATU_VS         (1 << BATU_VS_SHIFT)
#define BATU_VP_SHIFT   (31 - 31)
#define BATU_VP         (1 << BATU_VP_SHIFT)

#define BATL_BPRN_SHIFT (31 - 14)
#define BATL_BPRN       (32767 << BATL_BPRN_SHIFT)
#define BATL_WIMG_SHIFT (31 - 28)
#define BATL_WIMG       (15 << BATL_WIMG_SHIFT)
#define BATL_PP_SHIFT   (31 - 31)
#define BATL_PP         (3 << BATL_PP_SHIFT)
#define BATL_PP___      0 /* No access */
#define BATL_PP_R_      1 /* Read-only */
#define BATL_PP_RW      2 /* Read-write */

/*
 * MSR settings
 */
#define MSR_POW_SHIFT   (31 - 13)
#define MSR_POW         (1 << MSR_POW_SHIFT)
#define MSR_IR_SHIFT    (31 - 26)
#define MSR_IR          (1 << MSR_IR_SHIFT)
#define MSR_DR_SHIFT    (31 - 27)
#define MSR_DR          (1 << MSR_DR_SHIFT)


struct _cpuState {
	uint32_t srr0;
	uint32_t srr1;
	uint32_t msr;
	uint32_t hid0;
	uint32_t hid1;
	uint32_t hid2;
	uint32_t hid4;
	uint32_t l2cr;
	uint32_t mmcr0;
	uint32_t mmcr1;
	uint32_t pmc[4];
	uint32_t sprg[4];
	uint32_t ibatl[8];
	uint32_t ibatu[8];
	uint32_t dbatl[8];
	uint32_t dbatu[8];
	uint32_t segmentRegs[16];
	uint32_t decrementer;
	uint32_t tbl;
	uint32_t tbu;
};

/*
 * Initialize the emulated CPU state
 */
extern void E_CPU_Init(void);
extern uint32_t E_PPC_Emulate_mfspr(uint32_t spr);
extern void E_PPC_Emulate_mtspr(uint32_t spr, uint32_t val);
extern uint32_t E_PPC_Emulate_rfi(void);
extern void E_PPC_Emulate_mtmsr(uint32_t val);
extern uint32_t E_PPC_Emulate_mfmsr(void);
extern void E_PPC_Emulate_mtsr(uint32_t reg, uint32_t val);
extern uint32_t E_PPC_Emulate_mfsr(uint32_t reg);

#endif
