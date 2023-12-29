/*
 * Copyright (c) 2014-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * Author: Alexey Malahov <Alexey.Malahov@baikalelectronics.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DDR_BAIKAL_H
#define DDR_BAIKAL_H

#include <stdint.h>

#include <bm1000_def.h>

#define DDRC_MSTR		0x0	/* Master Register */
#define DDRC_STAT		0x4	/* Operating Mode Status Register */
#define DDRC_MRCTRL0		0x10	/* Mode Register Read/Write Control Register 0 */
#define DDRC_MRCTRL1		0x14	/* Mode Register Read/Write Control Register 1 */
#define DDRC_MRSTAT		0x18	/* Mode Register Read/Write Status Register */
#define DDRC_MRCTRL2		0x1c	/* Mode Register Read/Write Control Register 2 */
#define DDRC_DERATEEN		0x20	/* Temperature Derate Enable Register */
#define DDRC_DERATEINT		0x24	/* Temperature Derate Interval Register */
#define DDRC_PWRCTL		0x30	/* Low Power Control Register */
#define DDRC_PWRTMG		0x34	/* Low Power Timing Register */
#define DDRC_HWLPCTL		0x38	/* Hardware Low Power Control Register */
#define DDRC_RFSHCTL0		0x50	/* Refresh Control Register 0 */
#define DDRC_RFSHCTL1		0x54	/* Refresh Control Register 1 */
#define DDRC_RFSHCTL2		0x58	/* Refresh Control Register 2 */
#define DDRC_RFSHCTL3		0x60	/* Refresh Control Register 0 */
#define DDRC_RFSHTMG		0x64	/* Refresh Timing Register */
#define DDRC_ECCCFG0		0x70	/* ECC Configuration Register */
#define DDRC_ECCCFG1		0x74	/* ECC Configuration Register */
#define DDRC_ECCSTAT		0x78	/* ECC Status Register */
#define DDRC_ECCCLR		0x7c	/* ECC Clear Register */
#define DDRC_ECCERRCNT		0x80	/* ECC Error Counter Register */
#define DDRC_ECCCADDR0		0x84	/* ECC Corrected Error Address Register 0 */
#define DDRC_ECCCADDR1		0x88	/* ECC Corrected Error Address Register 1 */
#define DDRC_ECCCSYN0		0x8c	/* ECC Corrected Syndrome Register 0 */
#define DDRC_ECCCSYN1		0x90	/* ECC Corrected Syndrome Register 1 */
#define DDRC_ECCCSYN2		0x94	/* ECC Corrected Syndrome Register 2 */
#define DDRC_ECCBITMASK0	0x98	/* ECC Corrected Data Bit Mask Register 0 */
#define DDRC_ECCBITMASK1	0x9c	/* ECC Corrected Data Bit Mask Register 1 */
#define DDRC_ECCBITMASK2	0xa0	/* ECC Corrected Data Bit Mask Register 2 */
#define DDRC_ECCUADDR0		0xa4	/* ECC Uncorrected Error Address Register 0 */
#define DDRC_ECCUADDR1		0xa8	/* ECC Uncorrected Error Address Register 1 */
#define DDRC_ECCUSYN0		0xac	/* ECC Uncorrected Syndrome Register 0 */
#define DDRC_ECCUSYN1		0xb0	/* ECC Uncorrected Syndrome Register 1 */
#define DDRC_ECCUSYN2		0xb4	/* ECC Uncorrected Syndrome Register 2 */
#define DDRC_ECCPOISONADDR0	0xb8	/* ECC Data Poisoning Address Register 0 */
#define DDRC_ECCPOISONADDR1	0xbc	/* ECC Data Poisoning Address Register 1 */
#define DDRC_CRCPARCTL0		0xc0	/* CRC Parity Control Register 0 */
#define DDRC_CRCPARCTL1		0xc4	/* CRC Parity Control Register 1 */
#define DDRC_CRCPARCTL2		0xc8	/* CRC Parity Control Register 2 */
#define DDRC_CRCPARSTAT		0xcc	/* CRC Parity Status Register */
#define DDRC_INIT0		0xd0	/* SDRAM Initialization Register 0 */
#define DDRC_INIT1		0xd4	/* SDRAM Initialization Register 1 */
#define DDRC_INIT2		0xd8	/* SDRAM Initialization Register 2 */
#define DDRC_INIT3		0xdc	/* SDRAM Initialization Register 3 */
#define DDRC_INIT4		0xe0	/* SDRAM Initialization Register 4 */
#define DDRC_INIT5		0xe4	/* SDRAM Initialization Register 5 */
#define DDRC_INIT6		0xe8	/* SDRAM Initialization Register 6 */
#define DDRC_INIT7		0xec	/* SDRAM Initialization Register 7 */
#define DDRC_DIMMCTL		0xf0	/* DIMM Control Register */
#define DDRC_RANKCTL		0xf4	/* Rank Control Register */
#define DDRC_DRAMTMG0		0x100	/* SDRAM Timing Register 0 */
#define DDRC_DRAMTMG1		0x104	/* SDRAM Timing Register 1 */
#define DDRC_DRAMTMG2		0x108	/* SDRAM Timing Register 2 */
#define DDRC_DRAMTMG3		0x10c	/* SDRAM Timing Register 3 */
#define DDRC_DRAMTMG4		0x110	/* SDRAM Timing Register 4 */
#define DDRC_DRAMTMG5		0x114	/* SDRAM Timing Register 5 */
#define DDRC_DRAMTMG6		0x118	/* SDRAM Timing Register 6 */
#define DDRC_DRAMTMG7		0x11c	/* SDRAM Timing Register 7 */
#define DDRC_DRAMTMG8		0x120	/* SDRAM Timing Register 8 */
#define DDRC_DRAMTMG9		0x124	/* SDRAM Timing Register 9 */
#define DDRC_DRAMTMG10		0x128	/* SDRAM Timing Register 10 */
#define DDRC_DRAMTMG11		0x12c	/* SDRAM Timing Register 11 */
#define DDRC_DRAMTMG12		0x130	/* SDRAM Timing Register 12 */
#define DDRC_ZQCTL0		0x180	/* ZQ Control Register 0 */
#define DDRC_ZQCTL1		0x184	/* ZQ Control Register 1 */
#define DDRC_ZQCTL2		0x188	/* ZQ Control Register 2 */
#define DDRC_ZQSTAT		0x18c	/* ZQ Status Register */
#define DDRC_DFITMG0		0x190	/* DFI Timing Register 0 */
#define DDRC_DFITMG1		0x194	/* DFI Timing Register 1 */
#define DDRC_DFILPCFG0		0x198	/* DFI Low Power Configuration Register 0 */
#define DDRC_DFILPCFG1		0x19c	/* DFI Low Power Configuration Register 1 */
#define DDRC_DFIUPD0		0x1a0	/* DFI Update Register 0 */
#define DDRC_DFIUPD1		0x1a4	/* DFI Update Register 1 */
#define DDRC_DFIUPD2		0x1a8	/* DFI Update Register 2 */
#define DDRC_DFIUPD3		0x1ac	/* DFI Update Register 3 */
#define DDRC_DFIMISC		0x1b0	/* DFI Miscellaneous Control Register */
#define DDRC_DFITMG2		0x1b4	/* DFI Timing Register 2 */
#define DDRC_DFIUPD4		0x1b8	/* DFI Update Register 4 */
#define DDRC_DBICTL		0x1c0	/* DM/DBI Control Register */
#define DDRC_TRAINCTL0		0x1d0	/* PHY Eval Training Control Register 0 */
#define DDRC_TRAINCTL1		0x1d4	/* PHY Eval Training Control Register 1 */
#define DDRC_TRAINCTL2		0x1d8	/* PHY Eval Training Control Register 2 */
#define DDRC_TRAINSTAT		0x1dc	/* PHY Eval Training Status Register */
#define DDRC_ADDRMAP0		0x200	/* Address Map Register 0 */
#define DDRC_ADDRMAP1		0x204	/* Address Map Register 1 */
#define DDRC_ADDRMAP2		0x208	/* Address Map Register 2 */
#define DDRC_ADDRMAP3		0x20c	/* Address Map Register 3 */
#define DDRC_ADDRMAP4		0x210	/* Address Map Register 4 */
#define DDRC_ADDRMAP5		0x214	/* Address Map Register 5 */
#define DDRC_ADDRMAP6		0x218	/* Address Map Register 6 */
#define DDRC_ADDRMAP7		0x21c	/* Address Map Register 7 */
#define DDRC_ADDRMAP8		0x220	/* Address Map Register 8 */
#define DDRC_ODTCFG		0x240	/* ODT Configuration Register */
#define DDRC_ODTMAP		0x244	/* ODT/Rank Map Register */
#define DDRC_SCHED		0x250	/* Scheduler Control Register */
#define DDRC_SCHED1		0x254	/* Scheduler Control Register 1 */
#define DDRC_PERFHPR1		0x25c	/* High Priority Read CAM Register 1 */
#define DDRC_PERFLPR1		0x264	/* Low Priority Read CAM Register 1 */
#define DDRC_PERFWR1		0x26c	/* Write CAM Register 1 */
#define DDRC_PERFVPR1		0x274	/* Variable Priority Read CAM Register 1 */
#define DDRC_PERFVPW1		0x278	/* Variable Priority Write CAM Register 1 */
#define DDRC_DQMAP0		0x280	/* DQ Map Register 0 */
#define DDRC_DQMAP1		0x284	/* DQ Map Register 1 */
#define DDRC_DQMAP2		0x288	/* DQ Map Register 2 */
#define DDRC_DQMAP3		0x28c	/* DQ Map Register 3 */
#define DDRC_DQMAP4		0x290	/* DQ Map Register 4 */
#define DDRC_DQMAP5		0x294	/* DQ Map Register 5 */
#define DDRC_DBG0		0x300	/* Debug Register 0 */
#define DDRC_DBG1		0x304	/* Debug Register 1 */
#define DDRC_DBGCAM		0x308	/* CAM Debug Register */
#define DDRC_DBGCMD		0x30c	/* Command Debug Register */
#define DDRC_DBGSTAT		0x310	/* Status Debug Register */
#define DDRC_SWCTL		0x320	/* Software Register Programming Control Enable */
#define DDRC_SWSTAT		0x324	/* Software Register Programming Control Status */
#define DDRC_OCPARCFG0		0x330	/* On-Chip Parity Configuration Register 0 */
#define DDRC_OCPARCFG1		0x334	/* On-Chip Parity Configuration Register 1 */
#define DDRC_OCPARCFG2		0x338	/* On-Chip Parity Configuration Register 2 */
#define DDRC_OCPARCFG3		0x33c	/* On-Chip Parity Configuration Register 3 */
#define DDRC_OCPARSTAT0		0x340	/* On-Chip Parity Status Register 0 */
#define DDRC_OCPARSTAT1		0x344	/* On-Chip Parity Status Register 1 */
#define DDRC_OCPARWLOG0		0x348	/* On-Chip Parity Write Data Log Register 0 */
#define DDRC_OCPARWLOG1		0x34c	/* On-Chip Parity Write Data Log Register 1 */
#define DDRC_OCPARWLOG2		0x350	/* On-Chip Parity Write Data Log Register 2 */
#define DDRC_OCPARAWLOG0	0x354	/* On-Chip Parity Write Address Log 0 Register */
#define DDRC_OCPARAWLOG1	0x358	/* On-Chip Parity Write Address Log 1 Register */
#define DDRC_OCPARRLOG0		0x35c	/* On-Chip Parity Read Data Log 0 Register */
#define DDRC_OCPARRLOG1		0x360	/* On-Chip Parity Read Data Log 1 Register */
#define DDRC_OCPARARLOG0	0x364	/* On-Chip Parity Read Address Log 0 Register */
#define DDRC_OCPARARLOG1	0x368	/* On-Chip Parity Read Address Log 1 Register */
#define DDRC_PSTAT		0x3fc	/* Port Status Register */
#define DDRC_PCCFG		0x400	/* Port Common Configuration Register */
#define DDRC_PCFGR(n)		(0x404 + n * 0xb0)	/* Port n Configuration Read Register (for n = 0; n <= 15) */
#define DDRC_PCFGW(n)		(0x408 + n * 0xb0)	/* Port n Configuration Write Register (for n = 0; n <= 15) */
#define DDRC_PCFGC(n)		(0x40c + n * 0xb0)	/* Port n Common Configuration Register (for n = 0; n <= 15) */
#define DDRC_PCFGIDMASKCH(m, n)	(0x410 + m * 0x08 + n * 0xb0)	/* Port n Channel m Configuration ID Mask Register (for m,n = 0; m,n <= 15) */
#define DDRC_PCFGIDVALUECH(m, n) (0x414 + m * 0x08 + n * 0xb0)	/* Port n Channel m Configuration ID Value Register (for m,n = 0; m,n <= 15) */
#define DDRC_PCTRL(n)		(0x490 + n * 0xb0)	/* Port n Control Register (for n = 0; n <= 15) */
#define DDRC_PCFGQOS0(n)	(0x494 + n * 0xb0)	/* Port n Read QoS Configuration Register 0 (for n = 0; n <= 15) */
#define DDRC_PCFGQOS1(n)	(0x498 + n * 0xb0)	/* Port n Read QoS Configuration Register 1 (for n = 0; n <= 15) */
#define DDRC_PCFGWQOS0(n)	(0x49c + n * 0xb0)	/* Port n Write QoS Configuration Register 0 (for n = 0; n <= 15) */
#define DDRC_PCFGWQOS1(n)	(0x4a0 + n * 0xb0)	/* Port n Write QoS Configuration Register 1 (for n = 0; n <= 15) */
#define DDRC_SARBASE(n)		(0xf04 + n * 0x08)	/* SAR Base Address Register n, (for n = 0; n <= 3) */
#define DDRC_SARSIZE(n)		(0xf08 + n * 0x08)	/* SAR Size Register n, (for n = 0; n <= 3) */
#define DDRC_SBRCTL		0xf24	/* Scrubber Control Register */
#define DDRC_SBRSTAT		0xf28	/* Scrubber Status Register */
#define DDRC_SBRWDATA0		0xf2c	/* Scrubber Write Data Pattern0 */
#define DDRC_SBRWDATA1		0xf30	/* Scrubber Write Data Pattern1 */

/* Operating Mode Status Register Bits */
#define DDRC_STAT_MODE_MASK	0x7
#define DDRC_STAT_MODE_POS	0
#define DDRC_STAT_MODE_BITS	3
#define DDRC_STAT_MODE_INIT	0x00	/* Init Mode */
#define DDRC_STAT_MODE_NORMAL	0x01	/* Normal Mode */
#define DDRC_STAT_MODE_PW_DOWN	0x10	/* Power-down Mode */
#define DDRC_STAT_MODE_SELF_RF	0x11	/* Self-refresh Mode */

/* Scrubber */
#define SCRUB_EN		BIT(0)	/* Enable ECC scrubber */
#define SCRUB_EN_SHIFT		0
#define SCRUB_LP		BIT(1)	/* Continue scrubbing during low power */
#define SCRUB_MODE_WR		BIT(2)	/* 0: ECC scrubber performs reads */
#define SCRUB_MODE_SHIFT	2
#define SCRUB_BURST_SHIFT	4	/* Scrub burst count */
#define SCRUB_INTVAL_SHIFT	8	/* Scrub interval */
#define SCRUB_INTVAL_LEN	16
#define SCRUB_INTVAL_DEF	0xff
#define SCRUB_PATTERN0		0xc001cafe
#define SCRUB_PATTERN1		0xc001babe

/* Scrubber status */
#define SCRUB_BUSY		BIT(0) /* Scrubber busy */
#define SCRUB_DONE		BIT(1) /* Scrubber done */

/* DDR4 multiPHY Utility Block (PUB) registers */
#define DDR_PUB_RIDR		(0x000 << 2) /* R Revision Identification Register */
#define DDR_PUB_PIR		(0x001 << 2) /* R/W PHY Initialization Register */
#define DDR_PUB_CGCR		(0x002 << 2) /* R/W PHY Clock Gating Configuration Register */
#define DDR_PUB_CGCR1		(0x003 << 2) /* R/W PHY Clock Gating Configuration Register 1 */
#define DDR_PUB_PGCR0		(0x004 << 2) /* R/W PHY General Configuration Register 0 */
#define DDR_PUB_PGCR1		(0x005 << 2) /* R/W PHY General Configuration Register 1 */
#define DDR_PUB_PGCR2		(0x006 << 2) /* R/W PHY General Configuration Register 2*/
#define DDR_PUB_PGCR3		(0x007 << 2) /* R/W PHY General Configuration Register 3 */
#define DDR_PUB_PGCR4		(0x008 << 2) /* R/W PHY General Configuration Register 4 */
#define DDR_PUB_PGCR5		(0x009 << 2) /* R/W PHY General Configuration Register 5 */
#define DDR_PUB_PGCR6		(0x00a << 2) /* R/W PHY General Configuration Register 6 */
#define DDR_PUB_PGCR7		(0x00b << 2) /* R/W PHY General Configuration Register 7 */
#define DDR_PUB_PGCR8		(0x00c << 2) /* R/W PHY General Configuration Register 8 */
#define DDR_PUB_PGSR0		(0x00d << 2) /* R PHY General Status Register 0 */
#define DDR_PUB_PGSR1		(0x00e << 2) /* R PHY General Status Register 1 */
#define DDR_PUB_PTR0		(0x010 << 2) /* R/W PHY Timing Register 0 */
#define DDR_PUB_PTR1		(0x011 << 2) /* R/W PHY Timing Register 1 */
#define DDR_PUB_PTR2		(0x012 << 2) /* R/W PHY Timing Register 2 */
#define DDR_PUB_PTR3		(0x013 << 2) /* R/W PHY Timing Register 3 */
#define DDR_PUB_PTR4		(0x014 << 2) /* R/W PHY Timing Register 4 */
#define DDR_PUB_PTR5		(0x015 << 2) /* R/W PHY Timing Register 5 */
#define DDR_PUB_PTR6		(0x016 << 2) /* R/W PHY Timing Register 6 */
#define DDR_PUB_PLLCR		(0x020 << 2) /* R/W PLL Control Register */
#define DDR_PUB_DXCCR		(0x022 << 2) /* R/W DATX8 Common Configuration Register */
#define DDR_PUB_DSGCR		(0x024 << 2) /* R/W DDR System General Configuration Register */
#define DDR_PUB_ODTCR		(0x026 << 2) /* R/W ODT Configuration Register */
#define DDR_PUB_AACR		(0x028 << 2) /* R/W Anti-Aging Control Register */
#define DDR_PUB_GPR0		(0x030 << 2) /* R/W General Purpose Register 0 */
#define DDR_PUB_GPR1		(0x031 << 2) /* R/W General Purpose Register 1 */
#define DDR_PUB_DCR		(0x040 << 2) /* R/W DRAM Configuration Register */
#define DDR_PUB_DTPR0		(0x044 << 2) /* R/W SDRAM Timing Parameters Register 0 */
#define DDR_PUB_DTPR1		(0x045 << 2) /* R/W SDRAM Timing Parameters Register 1 */
#define DDR_PUB_DTPR2		(0x046 << 2) /* R/W SDRAM Timing Parameters Register 2 */
#define DDR_PUB_DTPR3		(0x047 << 2) /* R/W SDRAM Timing Parameters Register 3*/
#define DDR_PUB_DTPR4		(0x048 << 2) /* R/W SDRAM Timing Parameters Register 4 */
#define DDR_PUB_DTPR5		(0x049 << 2) /* R/W SDRAM Timing Parameters Register 5 */
#define DDR_PUB_DTPR6		(0x04a << 2) /* R/W SDRAM Timing Parameters Register 6*/
#define DDR_PUB_RDIMMGCR0	(0x050 << 2) /* R/W R DIMM General Configuration Register 0*/
#define DDR_PUB_RDIMMGCR1	(0x051 << 2) /* R/W R DIMM General Configuration Register 1*/
#define DDR_PUB_RDIMMGCR2	(0x052 << 2) /* R/W R DIMM General Configuration Register 2 */
#define DDR_PUB_RDIMMCR0	(0x054 << 2) /* R/W R DIMM Control Register 0 */
#define DDR_PUB_RDIMMCR1	(0x055 << 2) /* R/W R DIMM Control Register 1 */
#define DDR_PUB_RDIMMCR2	(0x056 << 2) /* R/W R DIMM Control Register 2 */
#define DDR_PUB_RDIMMCR3	(0x057 << 2) /* R/W R DIMM Control Register 3 */
#define DDR_PUB_RDIMMCR4	(0x058 << 2) /* R/W R DIMM Control Register 4 */
#define DDR_PUB_SCHCR0		(0x05a << 2) /* R/W Scheduler Command Register 0 */
#define DDR_PUB_SCHCR1		(0x05b << 2) /* R/W Scheduler Command Register 1 */
#define DDR_PUB_MR0		(0x060 << 2) /* R/W Mode Register 0 */
#define DDR_PUB_MR1		(0x061 << 2) /* R/W Mode Register 1 */
#define DDR_PUB_MR2		(0x062 << 2) /* R/W Mode Register 2 */
#define DDR_PUB_MR3		(0x063 << 2) /* R/W Mode Register 3 */
#define DDR_PUB_MR4		(0x064 << 2) /* R/W Mode Register 4 */
#define DDR_PUB_MR5		(0x065 << 2) /* R/W Mode Register 5 */
#define DDR_PUB_MR6		(0x066 << 2) /* R/W Mode Register 6 */
#define DDR_PUB_DTCR0		(0x080 << 2) /* R/W Data Training Configuration Register 0 */
#define DDR_PUB_DTCR1		(0x081 << 2) /* R/W Data Training Configuration Register 1 */
#define DDR_PUB_DTAR0		(0x082 << 2) /* R/W Data Training Address Register 0 */
#define DDR_PUB_DTAR1		(0x083 << 2) /* R/W Data Training Address Register 1 */
#define DDR_PUB_DTAR2		(0x084 << 2) /* R/W Data Training Address Register 2 */
#define DDR_PUB_DTDR0		(0x086 << 2) /* R/W Data Training Data Register 0 */
#define DDR_PUB_DTDR1		(0x087 << 2) /* R/W Data Training Data Register 1*/
#define DDR_PUB_UDDR0		(0x088 << 2) /* R/W User Defined Data Register 0 */
#define DDR_PUB_UDDR1		(0x089 << 2) /* R/W User Defined Data Register 1 */
#define DDR_PUB_DTEDR0		(0x08c << 2) /* R/W Data Training Eye Data Register 0 */
#define DDR_PUB_DTEDR1		(0x08d << 2) /* R/W Data Training Eye Data Register 1 */
#define DDR_PUB_DTEDR2		(0x08e << 2) /* R/W Data Training Eye Data Register 2 */
#define DDR_PUB_VTDR		(0x08f << 2) /* R/W VREF Training Data Register */
#define DDR_PUB_CATR0		(0x090 << 2) /* R/W CA Training Register 0 */
#define DDR_PUB_CATR1		(0x091 << 2) /* R/W CA Training Register 1 */
#define DDR_PUB_DQSDR0		(0x094 << 2) /* R/W DQS Drift Register 0 */
#define DDR_PUB_DQSDR1		(0x095 << 2) /* R/W DQS Drift Register 1 */
#define DDR_PUB_DQSDR2		(0x096 << 2) /* R/W DQS Drift Register 2 */
#define DDR_PUB_DCUAR		(0x0c0 << 2) /* R/W DCU Address Register */
#define DDR_PUB_DCUDR		(0x0c1 << 2) /* R/W DCU Data Register */
#define DDR_PUB_DCURR		(0x0c2 << 2) /* R/W DCU R un Register */
#define DDR_PUB_DCULR		(0x0c3 << 2) /* R/W DCU Loop Register */
#define DDR_PUB_DCUGCR		(0x0c4 << 2) /* R/W DCU General Configuration Register */
#define DDR_PUB_DCUTPR		(0x0c5 << 2) /* R/W DCU Timing Parameter Register */
#define DDR_PUB_DCUSR0		(0x0c6 << 2) /* R/W DCU Status 0 Register */
#define DDR_PUB_DCUSR1		(0x0c7 << 2) /* R/W DCU Status 1 Register */
#define DDR_PUB_BISTRR		(0x100 << 2) /* R/W BIST R un Register */
#define DDR_PUB_BISTWCR		(0x101 << 2) /* R/W BIST Word Count Register*/
#define DDR_PUB_BISTMSKR0	(0x102 << 2) /* R/W BIST Mask 0 Register*/
#define DDR_PUB_BISTMSKR1	(0x103 << 2) /* R/W BIST Mask 1 Register*/
#define DDR_PUB_BISTMSKR2	(0x104 << 2) /* R/W BIST Mask 2 Register */
#define DDR_PUB_BISTLSR		(0x105 << 2) /* R/W BIST LFSR Seed Register */
#define DDR_PUB_BISTAR0		(0x106 << 2) /* R/W BIST Address 0 Register */
#define DDR_PUB_BISTAR1		(0x107 << 2) /* R/W BIST Address 1 Register*/
#define DDR_PUB_BISTAR2		(0x108 << 2) /* R/W BIST Address 2 Register */
#define DDR_PUB_BISTAR3		(0x109 << 2) /* R/W BIST Address 3 Register */
#define DDR_PUB_BISTAR4		(0x10a << 2) /* R/W BIST Address 4 Register */
#define DDR_PUB_BISTUDPR	(0x10b << 2) /* R/W BIST User Data Pattern 0 Register */
#define DDR_PUB_BISTGSR		(0x10c << 2) /* R/W BIST General Status Register */
#define DDR_PUB_BISTWER0	(0x10d << 2) /* R/W BIST Word Error 0 Register */
#define DDR_PUB_BISTWER1	(0x10e << 2) /* R/W BIST Word Error 1 Register */
#define DDR_PUB_BISTBER0	(0x10f << 2) /* R/W BIST Bit Error 0 Register */
#define DDR_PUB_BISTBER1	(0x110 << 2) /* R/W BIST Bit Error 1 Register */
#define DDR_PUB_BISTBER2	(0x111 << 2) /* R/W BIST Bit Error 2 Register */
#define DDR_PUB_BISTBER3	(0x112 << 2) /* R/W BIST Bit Error 3 Register */
#define DDR_PUB_BISTBER4	(0x113 << 2) /* R/W BIST Bit Error 4 Register */
#define DDR_PUB_BISTWCSR	(0x114 << 2) /* R/W BIST Word Count Status Register */
#define DDR_PUB_BISTFWR0	(0x115 << 2) /* R/W BIST Fail Word 0 Register */
#define DDR_PUB_BISTFWR1	(0x116 << 2) /* R/W BIST Fail Word 1 Register */
#define DDR_PUB_BISTFWR2	(0x117 << 2) /* R/W BIST Fail Word 2 Register */
#define DDR_PUB_BISTBER5	(0x118 << 2) /* R/W BIST Bit Error 5 Register */
#define DDR_PUB_RANKIDR		(0x137 << 2) /* R/W R ank ID Register */
#define DDR_PUB_RIOCR0		(0x138 << 2) /* R/W R ank I/O Configuration Register 0 */
#define DDR_PUB_RIOCR1		(0x139 << 2) /* R/W R ank I/O Configuration Register 1 */
#define DDR_PUB_RIOCR2		(0x13a << 2) /* R/W R ank I/O Configuration Register 2 */
#define DDR_PUB_RIOCR3		(0x13b << 2) /* R/W R ank I/O Configuration Register 3 */
#define DDR_PUB_RIOCR4		(0x13c << 2) /* R/W R ank I/O Configuration Register 4 */
#define DDR_PUB_RIOCR5		(0x13d << 2) /* R/W R ank I/O Configuration Register 5 */
#define DDR_PUB_ACIOCR0		(0x140 << 2) /* R/W AC I/O Configuration Register 0 */
#define DDR_PUB_ACIOCR1		(0x141 << 2) /* R/W AC I/O Configuration Register 1 */
#define DDR_PUB_ACIOCR2		(0x142 << 2) /* R/W AC I/O Configuration Register 2 */
#define DDR_PUB_ACIOCR3		(0x143 << 2) /* R/W AC I/O Configuration Register 3 */
#define DDR_PUB_ACIOCR4		(0x144 << 2) /* R/W AC I/O Configuration Register 4 */
#define DDR_PUB_IOVCR0		(0x148 << 2) /* R/W IO VREF Control Register 0 */
#define DDR_PUB_IOVCR1		(0x149 << 2) /* R/W IO VREF Control Register 1 */
#define DDR_PUB_VTCR0		(0x14a << 2) /* R/W VREF Training Control Register 0 */
#define DDR_PUB_VTCR1		(0x14b << 2) /* R/W VREF Training Control Register 1 */
#define DDR_PUB_ACBDLR0		(0x150 << 2) /* R/W AC Bit Delay Line Register 0 */
#define DDR_PUB_ACBDLR1		(0x151 << 2) /* R/W AC Bit Delay Line Register 1 */
#define DDR_PUB_ACBDLR2		(0x152 << 2) /* R/W AC Bit Delay Line Register 2 */
#define DDR_PUB_ACBDLR3		(0x153 << 2) /* R/W AC Bit Delay Line Register 3 */
#define DDR_PUB_ACBDLR4		(0x154 << 2) /* R/W AC Bit Delay Line Register 4 */
#define DDR_PUB_ACBDLR5		(0x155 << 2) /* R/W AC Bit Delay Line Register 5 */
#define DDR_PUB_ACBDLR6		(0x156 << 2) /* R/W AC Bit Delay Line Register 6 */
#define DDR_PUB_ACBDLR7		(0x157 << 2) /* R/W AC Bit Delay Line Register 7 */
#define DDR_PUB_ACBDLR8		(0x158 << 2) /* R/W AC Bit Delay Line Register 8 */
#define DDR_PUB_ACBDLR9		(0x159 << 2) /* R/W AC Bit Delay Line Register 9 */
#define DDR_PUB_ACBDLR10	(0x15a << 2) /* R/W AC Bit Delay Line Register 10 */
#define DDR_PUB_ACLCDLR		(0x160 << 2) /* R/W AC Local Calibrated Delay Line Register */
#define DDR_PUB_ACMDLR0		(0x168 << 2) /* R/W AC Master Delay Line Register 0 */
#define DDR_PUB_ACMDLR1		(0x169 << 2) /* R/W AC Master Delay Line Register 1 */
#define DDR_PUB_ZQCR		(0x1a0 << 2) /* R/W ZQ Impedance Control Register */
#define DDR_PUB_ZQ0PR		(0x1a1 << 2) /* R/W ZQ n Impedance Controller Program Register */
#define DDR_PUB_ZQ0DR		(0x1a2 << 2) /* R/W ZQ n Impedance Controller Data Register */
#define DDR_PUB_ZQ0SR		(0x1a3 << 2) /* R ZQ n Impedance Controller Status Register */
#define DDR_PUB_ZQ1PR		(0x1a5 << 2) /* R/W ZQ n Impedance Controller Program Register */
#define DDR_PUB_ZQ1DR		(0x1a6 << 2) /* R/W ZQ n Impedance Controller Data Register */
#define DDR_PUB_ZQ1SR		(0x1a7 << 2) /* R ZQ n Impedance Controller Status Register */
#define DDR_PUB_DX0GCR0		(0x1c0 << 2) /* R/W DATX8 n General Configuration Register 0 */
#define DDR_PUB_DX0GCR1		(0x1c1 << 2) /* R/W DATX8 n General Configuration Register 1 */
#define DDR_PUB_DX0GCR2		(0x1c2 << 2) /* R/W DATX8 n General Configuration Register 2 */
#define DDR_PUB_DX0GCR3		(0x1c3 << 2) /* R/W DATX8 n General Configuration Register 3 */
#define DDR_PUB_DX0GCR4		(0x1c4 << 2) /* R/W DATX8 n General Configuration Register 4 */
#define DDR_PUB_DX0GCR5		(0x1c5 << 2) /* R/W DATX8 n General Configuration Register 5 */
#define DDR_PUB_DX0GCR6		(0x1c6 << 2) /* R/W DATX8 n General Configuration Register 6 */
#define DDR_PUB_DX0BDLR0	(0x1d0 << 2) /* R/W DATX8 n Bit Delay Line Register 0 */
#define DDR_PUB_DX0BDLR1	(0x1d1 << 2) /* R/W DATX8 n Bit Delay Line Register 1 */
#define DDR_PUB_DX0BDLR2	(0x1d2 << 2) /* R/W DATX8 n Bit Delay Line Register 2 */
#define DDR_PUB_DX0BDLR3	(0x1d4 << 2) /* R/W DATX8 n Bit Delay Line Register 3 */
#define DDR_PUB_DX0BDLR4	(0x1d5 << 2) /* R/W DATX8 n Bit Delay Line Register 4 */
#define DDR_PUB_DX0BDLR5	(0x1d6 << 2) /* R/W DATX8 n Bit Delay Line Register 5 */
#define DDR_PUB_DX0BDLR6	(0x1d8 << 2) /* R/W DATX8 n Bit Delay Line Register 6*/
#define DDR_PUB_DX0LCDLR0	(0x1e0 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 0*/
#define DDR_PUB_DX0LCDLR1	(0x1e1 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 1*/
#define DDR_PUB_DX0LCDLR2	(0x1e2 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 2*/
#define DDR_PUB_DX0LCDLR3	(0x1e3 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 3*/
#define DDR_PUB_DX0LCDLR4	(0x1e4 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 4*/
#define DDR_PUB_DX0LCDLR5	(0x1e5 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 5 */
#define DDR_PUB_DX0MDLR0	(0x1e8 << 2) /* R/W DATX8 n Master Delay Register 0 */
#define DDR_PUB_DX0MDLR1	(0x1e9 << 2) /* R/W DATX8 n Master Delay Register 1 */
#define DDR_PUB_DX0GTR		(0x1f0 << 2) /* R/W DATX8 n General Timing Register */
#define DDR_PUB_DX0RSR0		(0x1f4 << 2) /* R DATX8 n Rank Status Register 0 */
#define DDR_PUB_DX0RSR1		(0x1f5 << 2) /* R DATX8 n Rank Status Register 1 */
#define DDR_PUB_DX0RSR2		(0x1f6 << 2) /* R DATX8 n Rank Status Register 2 */
#define DDR_PUB_DX0RSR3		(0x1f7 << 2) /* R DATX8 n Rank Status Register 3 */
#define DDR_PUB_DX0GSR0		(0x1f8 << 2) /* R DATX8 n General Status Register 0 */
#define DDR_PUB_DX0GSR1		(0x1f9 << 2) /* R DATX8 n General Status Register 1 */
#define DDR_PUB_DX0GSR2		(0x1fa << 2) /* R DATX8 n General Status Register 2 */
#define DDR_PUB_DX0GSR3		(0x1fb << 2) /* R DATX8 n General Status Register 3 */
#define DDR_PUB_DX1GCR0		(0x200 << 2) /* R/W DATX8 n General Configuration Register 0 */
#define DDR_PUB_DX1GCR1		(0x201 << 2) /* R/W DATX8 n General Configuration Register 1 */
#define DDR_PUB_DX1GCR2		(0x202 << 2) /* R/W DATX8 n General Configuration Register 2 */
#define DDR_PUB_DX1GCR3		(0x203 << 2) /* R/W DATX8 n General Configuration Register 3 */
#define DDR_PUB_DX1GCR4		(0x204 << 2) /* R/W DATX8 n General Configuration Register 4 */
#define DDR_PUB_DX1GCR5		(0x205 << 2) /* R/W DATX8 n General Configuration Register 5 */
#define DDR_PUB_DX1GCR6		(0x206 << 2) /* R/W DATX8 n General Configuration Register 6 */
#define DDR_PUB_DX1BDLR0	(0x210 << 2) /* R/W DATX8 n Bit Delay Line Register 0 */
#define DDR_PUB_DX1BDLR1	(0x211 << 2) /* R/W DATX8 n Bit Delay Line Register 1 */
#define DDR_PUB_DX1BDLR2	(0x212 << 2) /* R/W DATX8 n Bit Delay Line Register 2 */
#define DDR_PUB_DX1BDLR3	(0x214 << 2) /* R/W DATX8 n Bit Delay Line Register 3 */
#define DDR_PUB_DX1BDLR4	(0x215 << 2) /* R/W DATX8 n Bit Delay Line Register 4 */
#define DDR_PUB_DX1BDLR5	(0x216 << 2) /* R/W DATX8 n Bit Delay Line Register 5 */
#define DDR_PUB_DX1BDLR6	(0x218 << 2) /* R/W DATX8 n Bit Delay Line Register 6*/
#define DDR_PUB_DX1LCDLR0	(0x220 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 0*/
#define DDR_PUB_DX1LCDLR1	(0x221 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 1*/
#define DDR_PUB_DX1LCDLR2	(0x222 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 2*/
#define DDR_PUB_DX1LCDLR3	(0x223 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 3*/
#define DDR_PUB_DX1LCDLR4	(0x224 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 4*/
#define DDR_PUB_DX1LCDLR5	(0x225 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 5 */
#define DDR_PUB_DX1MDLR0	(0x228 << 2) /* R/W DATX8 n Master Delay Register 0 */
#define DDR_PUB_DX1MDLR1	(0x229 << 2) /* R/W DATX8 n Master Delay Register 1 */
#define DDR_PUB_DX1GTR0		(0x230 << 2) /* R/W DATX8 n General Timing Register */
#define DDR_PUB_DX1RSR0		(0x234 << 2) /* R DATX8 n Rank Status Register 0 */
#define DDR_PUB_DX1RSR1		(0x235 << 2) /* R DATX8 n Rank Status Register 1 */
#define DDR_PUB_DX1RSR2		(0x236 << 2) /* R DATX8 n Rank Status Register 2 */
#define DDR_PUB_DX1RSR3		(0x237 << 2) /* R DATX8 n Rank Status Register 3 */
#define DDR_PUB_DX1GSR0		(0x238 << 2) /* R DATX8 n General Status Register 0 */
#define DDR_PUB_DX1GSR1		(0x239 << 2) /* R DATX8 n General Status Register 1 */
#define DDR_PUB_DX1GSR2		(0x23a << 2) /* R DATX8 n General Status Register 2 */
#define DDR_PUB_DX1GSR3		(0x23b << 2) /* R DATX8 n General Status Register 3 */
#define DDR_PUB_DX2GCR0		(0x240 << 2) /* R/W DATX8 n General Configuration Register 0 */
#define DDR_PUB_DX2GCR1		(0x241 << 2) /* R/W DATX8 n General Configuration Register 1 */
#define DDR_PUB_DX2GCR2		(0x242 << 2) /* R/W DATX8 n General Configuration Register 2 */
#define DDR_PUB_DX2GCR3		(0x243 << 2) /* R/W DATX8 n General Configuration Register 3 */
#define DDR_PUB_DX2GCR4		(0x244 << 2) /* R/W DATX8 n General Configuration Register 4 */
#define DDR_PUB_DX2GCR5		(0x245 << 2) /* R/W DATX8 n General Configuration Register 5 */
#define DDR_PUB_DX2GCR6		(0x246 << 2) /* R/W DATX8 n General Configuration Register 6 */
#define DDR_PUB_DX2BDLR0	(0x250 << 2) /* R/W DATX8 n Bit Delay Line Register 0 */
#define DDR_PUB_DX2BDLR1	(0x251 << 2) /* R/W DATX8 n Bit Delay Line Register 1 */
#define DDR_PUB_DX2BDLR2	(0x252 << 2) /* R/W DATX8 n Bit Delay Line Register 2 */
#define DDR_PUB_DX2BDLR3	(0x254 << 2) /* R/W DATX8 n Bit Delay Line Register 3 */
#define DDR_PUB_DX2BDLR4	(0x255 << 2) /* R/W DATX8 n Bit Delay Line Register 4 */
#define DDR_PUB_DX2BDLR5	(0x256 << 2) /* R/W DATX8 n Bit Delay Line Register 5 */
#define DDR_PUB_DX2BDLR6	(0x258 << 2) /* R/W DATX8 n Bit Delay Line Register 6*/
#define DDR_PUB_DX2LCDLR0	(0x260 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 0*/
#define DDR_PUB_DX2LCDLR1	(0x261 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 1*/
#define DDR_PUB_DX2LCDLR2	(0x262 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 2*/
#define DDR_PUB_DX2LCDLR3	(0x263 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 3*/
#define DDR_PUB_DX2LCDLR4	(0x264 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 4 */
#define DDR_PUB_DX2LCDLR5	(0x265 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 5 */
#define DDR_PUB_DX2MDLR0	(0x268 << 2) /* R/W DATX8 n Master Delay Register 0 */
#define DDR_PUB_DX2MDLR1	(0x269 << 2) /* R/W DATX8 n Master Delay Register 1 */
#define DDR_PUB_DX2GTR0		(0x270 << 2) /* R/W DATX8 n General Timing Register */
#define DDR_PUB_DX2RSR0		(0x274 << 2) /* R DATX8 n Rank Status Register 0 */
#define DDR_PUB_DX2RSR1		(0x275 << 2) /* R DATX8 n Rank Status Register 1 */
#define DDR_PUB_DX2RSR2		(0x276 << 2) /* R DATX8 n Rank Status Register 2 */
#define DDR_PUB_DX2RSR3		(0x277 << 2) /* R DATX8 n Rank Status Register 3 */
#define DDR_PUB_DX2GSR0		(0x278 << 2) /* R DATX8 n General Status Register 0 */
#define DDR_PUB_DX2GSR1		(0x279 << 2) /* R DATX8 n General Status Register 1 */
#define DDR_PUB_DX2GSR2		(0x27a << 2) /* R DATX8 n General Status Register 2 */
#define DDR_PUB_DX2GSR3		(0x27b << 2) /* R DATX8 n General Status Register 3 */
#define DDR_PUB_DX3GCR0		(0x280 << 2) /* R/W DATX8 n General Configuration Register 0 */
#define DDR_PUB_DX3GCR1		(0x281 << 2) /* R/W DATX8 n General Configuration Register 1 */
#define DDR_PUB_DX3GCR2		(0x282 << 2) /* R/W DATX8 n General Configuration Register 2 */
#define DDR_PUB_DX3GCR3		(0x283 << 2) /* R/W DATX8 n General Configuration Register 3 */
#define DDR_PUB_DX3GCR4		(0x284 << 2) /* R/W DATX8 n General Configuration Register 4 */
#define DDR_PUB_DX3GCR5		(0x285 << 2) /* R/W DATX8 n General Configuration Register 5 */
#define DDR_PUB_DX3GCR6		(0x286 << 2) /* R/W DATX8 n General Configuration Register 6 */
#define DDR_PUB_DX3BDLR0	(0x290 << 2) /* R/W DATX8 n Bit Delay Line Register 0 */
#define DDR_PUB_DX3BDLR1	(0x291 << 2) /* R/W DATX8 n Bit Delay Line Register 1 */
#define DDR_PUB_DX3BDLR2	(0x292 << 2) /* R/W DATX8 n Bit Delay Line Register 2 */
#define DDR_PUB_DX3BDLR3	(0x294 << 2) /* R/W DATX8 n Bit Delay Line Register 3 */
#define DDR_PUB_DX3BDLR4	(0x295 << 2) /* R/W DATX8 n Bit Delay Line Register 4 */
#define DDR_PUB_DX3BDLR5	(0x296 << 2) /* R/W DATX8 n Bit Delay Line Register 5 */
#define DDR_PUB_DX3BDLR6	(0x298 << 2) /* R/W DATX8 n Bit Delay Line Register 6*/
#define DDR_PUB_DX3LCDLR0	(0x2a0 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 0*/
#define DDR_PUB_DX3LCDLR1	(0x2a1 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 1*/
#define DDR_PUB_DX3LCDLR2	(0x2a2 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 2*/
#define DDR_PUB_DX3LCDLR3	(0x2a3 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 3*/
#define DDR_PUB_DX3LCDLR4	(0x2a4 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 4*/
#define DDR_PUB_DX3LCDLR5	(0x2a5 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 5 */
#define DDR_PUB_DX3MDLR0	(0x2a8 << 2) /* R/W DATX8 n Master Delay Register 0 */
#define DDR_PUB_DX3MDLR1	(0x2a9 << 2) /* R/W DATX8 n Master Delay Register 1 */
#define DDR_PUB_DX3GTR0		(0x2b0 << 2) /* R/W DATX8 n General Timing Register */
#define DDR_PUB_DX3RSR0		(0x2b4 << 2) /* R DATX8 n Rank Status Register 0 */
#define DDR_PUB_DX3RSR1		(0x2b5 << 2) /* R DATX8 n Rank Status Register 1 */
#define DDR_PUB_DX3RSR2		(0x2b6 << 2) /* R DATX8 n Rank Status Register 2 */
#define DDR_PUB_DX3RSR3		(0x2b7 << 2) /* R DATX8 n Rank Status Register 3 */
#define DDR_PUB_DX3GSR0		(0x2b8 << 2) /* R DATX8 n General Status Register 0 */
#define DDR_PUB_DX3GSR1		(0x2b9 << 2) /* R DATX8 n General Status Register 1 */
#define DDR_PUB_DX3GSR2		(0x2ba << 2) /* R DATX8 n General Status Register 2 */
#define DDR_PUB_DX3GSR3		(0x2bb << 2) /* R DATX8 n General Status Register 3 */
#define DDR_PUB_DX4GCR0		(0x2c0 << 2) /* R/W DATX8 n General Configuration Register 0 */
#define DDR_PUB_DX4GCR1		(0x2c1 << 2) /* R/W DATX8 n General Configuration Register 1 */
#define DDR_PUB_DX4GCR2		(0x2c2 << 2) /* R/W DATX8 n General Configuration Register 2 */
#define DDR_PUB_DX4GCR3		(0x2c3 << 2) /* R/W DATX8 n General Configuration Register 3 */
#define DDR_PUB_DX4GCR4		(0x2c4 << 2) /* R/W DATX8 n General Configuration Register 4 */
#define DDR_PUB_DX4GCR5		(0x2c5 << 2) /* R/W DATX8 n General Configuration Register 5 */
#define DDR_PUB_DX4GCR6		(0x2c6 << 2) /* R/W DATX8 n General Configuration Register 6 */
#define DDR_PUB_DX4BDLR0	(0x2d0 << 2) /* R/W DATX8 n Bit Delay Line Register 0 */
#define DDR_PUB_DX4BDLR1	(0x2d1 << 2) /* R/W DATX8 n Bit Delay Line Register 1 */
#define DDR_PUB_DX4BDLR2	(0x2d2 << 2) /* R/W DATX8 n Bit Delay Line Register 2 */
#define DDR_PUB_DX4BDLR3	(0x2d4 << 2) /* R/W DATX8 n Bit Delay Line Register 3 */
#define DDR_PUB_DX4BDLR4	(0x2d5 << 2) /* R/W DATX8 n Bit Delay Line Register 4 */
#define DDR_PUB_DX4BDLR5	(0x2d6 << 2) /* R/W DATX8 n Bit Delay Line Register 5 */
#define DDR_PUB_DX4BDLR6	(0x2d8 << 2) /* R/W DATX8 n Bit Delay Line Register 6*/
#define DDR_PUB_DX4LCDLR0	(0x2e0 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 0*/
#define DDR_PUB_DX4LCDLR1	(0x2e1 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 1*/
#define DDR_PUB_DX4LCDLR2	(0x2e2 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 2*/
#define DDR_PUB_DX4LCDLR3	(0x2e3 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 3*/
#define DDR_PUB_DX4LCDLR4	(0x2e4 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 4*/
#define DDR_PUB_DX4LCDLR5	(0x2e5 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 5 */
#define DDR_PUB_DX4MDLR0	(0x2e8 << 2) /* R/W DATX8 n Master Delay Register 0 */
#define DDR_PUB_DX4MDLR1	(0x2e9 << 2) /* R/W DATX8 n Master Delay Register 1 */
#define DDR_PUB_DX4GTR0		(0x2f0 << 2) /* R/W DATX8 n General Timing Register */
#define DDR_PUB_DX4RSR0		(0x2f4 << 2) /* R DATX8 n Rank Status Register 0 */
#define DDR_PUB_DX4RSR1		(0x2f5 << 2) /* R DATX8 n Rank Status Register 1 */
#define DDR_PUB_DX4RSR2		(0x2f6 << 2) /* R DATX8 n Rank Status Register 2 */
#define DDR_PUB_DX4RSR3		(0x2f7 << 2) /* R DATX8 n Rank Status Register 3 */
#define DDR_PUB_DX4GSR0		(0x2f8 << 2) /* R DATX8 n General Status Register 0 */
#define DDR_PUB_DX4GSR1		(0x2f9 << 2) /* R DATX8 n General Status Register 1 */
#define DDR_PUB_DX4GSR2		(0x2fa << 2) /* R DATX8 n General Status Register 2 */
#define DDR_PUB_DX4GSR3		(0x2fb << 2) /* R DATX8 n General Status Register 3 */
#define DDR_PUB_DX5GCR0		(0x300 << 2) /* R/W DATX8 n General Configuration Register 0 */
#define DDR_PUB_DX5GCR1		(0x301 << 2) /* R/W DATX8 n General Configuration Register 1 */
#define DDR_PUB_DX5GCR2		(0x302 << 2) /* R/W DATX8 n General Configuration Register 2 */
#define DDR_PUB_DX5GCR3		(0x303 << 2) /* R/W DATX8 n General Configuration Register 3 */
#define DDR_PUB_DX5GCR4		(0x304 << 2) /* R/W DATX8 n General Configuration Register 4 */
#define DDR_PUB_DX5GCR5		(0x305 << 2) /* R/W DATX8 n General Configuration Register 5 */
#define DDR_PUB_DX5GCR6		(0x306 << 2) /* R/W DATX8 n General Configuration Register 6 */
#define DDR_PUB_DX5BDLR0	(0x310 << 2) /* R/W DATX8 n Bit Delay Line Register 0 */
#define DDR_PUB_DX5BDLR1	(0x311 << 2) /* R/W DATX8 n Bit Delay Line Register 1 */
#define DDR_PUB_DX5BDLR2	(0x312 << 2) /* R/W DATX8 n Bit Delay Line Register 2 */
#define DDR_PUB_DX5BDLR3	(0x314 << 2) /* R/W DATX8 n Bit Delay Line Register 3 */
#define DDR_PUB_DX5BDLR4	(0x315 << 2) /* R/W DATX8 n Bit Delay Line Register 4 */
#define DDR_PUB_DX5BDLR5	(0x316 << 2) /* R/W DATX8 n Bit Delay Line Register 5 */
#define DDR_PUB_DX5BDLR6	(0x318 << 2) /* R/W DATX8 n Bit Delay Line Register 6*/
#define DDR_PUB_DX5LCDLR0	(0x320 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 0*/
#define DDR_PUB_DX5LCDLR1	(0x321 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 1*/
#define DDR_PUB_DX5LCDLR2	(0x322 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 2*/
#define DDR_PUB_DX5LCDLR3	(0x323 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 3*/
#define DDR_PUB_DX5LCDLR4	(0x324 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 4*/
#define DDR_PUB_DX5LCDLR5	(0x325 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 5 */
#define DDR_PUB_DX5MDLR0	(0x328 << 2) /* R/W DATX8 n Master Delay Register 0 */
#define DDR_PUB_DX5MDLR1	(0x329 << 2) /* R/W DATX8 n Master Delay Register 1 */
#define DDR_PUB_DX5GTR0		(0x330 << 2) /* R/W DATX8 n General Timing Register */
#define DDR_PUB_DX5RSR0		(0x334 << 2) /* R DATX8 n Rank Status Register 0 */
#define DDR_PUB_DX5RSR1		(0x335 << 2) /* R DATX8 n Rank Status Register 1 */
#define DDR_PUB_DX5RSR2		(0x336 << 2) /* R DATX8 n Rank Status Register 2 */
#define DDR_PUB_DX5RSR3		(0x337 << 2) /* R DATX8 n Rank Status Register 3 */
#define DDR_PUB_DX5GSR0		(0x338 << 2) /* R DATX8 n General Status Register 0 */
#define DDR_PUB_DX5GSR1		(0x339 << 2) /* R DATX8 n General Status Register 1 */
#define DDR_PUB_DX5GSR2		(0x33a << 2) /* R DATX8 n General Status Register 2 */
#define DDR_PUB_DX5GSR3		(0x33b << 2) /* R DATX8 n General Status Register 3 */
#define DDR_PUB_DX6GCR0		(0x340 << 2) /* R/W DATX8 n General Configuration Register 0 */
#define DDR_PUB_DX6GCR1		(0x341 << 2) /* R/W DATX8 n General Configuration Register 1 */
#define DDR_PUB_DX6GCR2		(0x342 << 2) /* R/W DATX8 n General Configuration Register 2 */
#define DDR_PUB_DX6GCR3		(0x343 << 2) /* R/W DATX8 n General Configuration Register 3 */
#define DDR_PUB_DX6GCR4		(0x344 << 2) /* R/W DATX8 n General Configuration Register 4 */
#define DDR_PUB_DX6GCR5		(0x345 << 2) /* R/W DATX8 n General Configuration Register 5 */
#define DDR_PUB_DX6GCR6		(0x346 << 2) /* R/W DATX8 n General Configuration Register 6 */
#define DDR_PUB_DX6BDLR0	(0x350 << 2) /* R/W DATX8 n Bit Delay Line Register 0 */
#define DDR_PUB_DX6BDLR1	(0x351 << 2) /* R/W DATX8 n Bit Delay Line Register 1 */
#define DDR_PUB_DX6BDLR2	(0x352 << 2) /* R/W DATX8 n Bit Delay Line Register 2 */
#define DDR_PUB_DX6BDLR3	(0x354 << 2) /* R/W DATX8 n Bit Delay Line Register 3 */
#define DDR_PUB_DX6BDLR4	(0x355 << 2) /* R/W DATX8 n Bit Delay Line Register 4 */
#define DDR_PUB_DX6BDLR5	(0x356 << 2) /* R/W DATX8 n Bit Delay Line Register 5 */
#define DDR_PUB_DX6BDLR6	(0x358 << 2) /* R/W DATX8 n Bit Delay Line Register 6*/
#define DDR_PUB_DX6LCDLR0	(0x360 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 0*/
#define DDR_PUB_DX6LCDLR1	(0x361 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 1*/
#define DDR_PUB_DX6LCDLR2	(0x362 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 2*/
#define DDR_PUB_DX6LCDLR3	(0x363 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 3*/
#define DDR_PUB_DX6LCDLR4	(0x364 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 4*/
#define DDR_PUB_DX6LCDLR5	(0x365 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 5 */
#define DDR_PUB_DX6MDLR0	(0x368 << 2) /* R/W DATX8 n Master Delay Register 0 */
#define DDR_PUB_DX6MDLR1	(0x369 << 2) /* R/W DATX8 n Master Delay Register 1 */
#define DDR_PUB_DX6GTR0		(0x370 << 2) /* R/W DATX8 n General Timing Register */
#define DDR_PUB_DX6RSR0		(0x374 << 2) /* R DATX8 n Rank Status Register 0 */
#define DDR_PUB_DX6RSR1		(0x375 << 2) /* R DATX8 n Rank Status Register 1 */
#define DDR_PUB_DX6RSR2		(0x376 << 2) /* R DATX8 n Rank Status Register 2 */
#define DDR_PUB_DX6RSR3		(0x377 << 2) /* R DATX8 n Rank Status Register 3 */
#define DDR_PUB_DX6GSR0		(0x378 << 2) /* R DATX8 n General Status Register 0 */
#define DDR_PUB_DX6GSR1		(0x379 << 2) /* R DATX8 n General Status Register 1 */
#define DDR_PUB_DX6GSR2		(0x37a << 2) /* R DATX8 n General Status Register 2 */
#define DDR_PUB_DX6GSR3		(0x37b << 2) /* R DATX8 n General Status Register 3 */
#define DDR_PUB_DX7GCR0		(0x380 << 2) /* R/W DATX8 n General Configuration Register 0 */
#define DDR_PUB_DX7GCR1		(0x381 << 2) /* R/W DATX8 n General Configuration Register 1 */
#define DDR_PUB_DX7GCR2		(0x382 << 2) /* R/W DATX8 n General Configuration Register 2 */
#define DDR_PUB_DX7GCR3		(0x383 << 2) /* R/W DATX8 n General Configuration Register 3 */
#define DDR_PUB_DX7GCR4		(0x384 << 2) /* R/W DATX8 n General Configuration Register 4 */
#define DDR_PUB_DX7GCR5		(0x385 << 2) /* R/W DATX8 n General Configuration Register 5 */
#define DDR_PUB_DX7GCR6		(0x386 << 2) /* R/W DATX8 n General Configuration Register 6 */
#define DDR_PUB_DX7BDLR0	(0x390 << 2) /* R/W DATX8 n Bit Delay Line Register 0 */
#define DDR_PUB_DX7BDLR1	(0x391 << 2) /* R/W DATX8 n Bit Delay Line Register 1 */
#define DDR_PUB_DX7BDLR2	(0x392 << 2) /* R/W DATX8 n Bit Delay Line Register 2 */
#define DDR_PUB_DX7BDLR3	(0x394 << 2) /* R/W DATX8 n Bit Delay Line Register 3 */
#define DDR_PUB_DX7BDLR4	(0x395 << 2) /* R/W DATX8 n Bit Delay Line Register 4 */
#define DDR_PUB_DX7BDLR5	(0x396 << 2) /* R/W DATX8 n Bit Delay Line Register 5 */
#define DDR_PUB_DX7BDLR6	(0x398 << 2) /* R/W DATX8 n Bit Delay Line Register 6*/
#define DDR_PUB_DX7LCDLR0	(0x3a0 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 0*/
#define DDR_PUB_DX7LCDLR1	(0x3a1 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 1*/
#define DDR_PUB_DX7LCDLR2	(0x3a2 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 2*/
#define DDR_PUB_DX7LCDLR3	(0x3a3 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 3*/
#define DDR_PUB_DX7LCDLR4	(0x3a4 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 4*/
#define DDR_PUB_DX7LCDLR5	(0x3a5 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 5 */
#define DDR_PUB_DX7MDLR0	(0x3a8 << 2) /* R/W DATX8 n Master Delay Register 0 */
#define DDR_PUB_DX7MDLR1	(0x3a9 << 2) /* R/W DATX8 n Master Delay Register 1 */
#define DDR_PUB_DX7GTR0		(0x3b0 << 2) /* R/W DATX8 n General Timing Register */
#define DDR_PUB_DX7RSR0		(0x3b4 << 2) /* R DATX8 n Rank Status Register 0 */
#define DDR_PUB_DX7RSR1		(0x3b5 << 2) /* R DATX8 n Rank Status Register 1 */
#define DDR_PUB_DX7RSR2		(0x3b6 << 2) /* R DATX8 n Rank Status Register 2 */
#define DDR_PUB_DX7RSR3		(0x3b7 << 2) /* R DATX8 n Rank Status Register 3 */
#define DDR_PUB_DX7GSR0		(0x3b8 << 2) /* R DATX8 n General Status Register 0 */
#define DDR_PUB_DX7GSR1		(0x3b9 << 2) /* R DATX8 n General Status Register 1 */
#define DDR_PUB_DX7GSR2		(0x3ba << 2) /* R DATX8 n General Status Register 2 */
#define DDR_PUB_DX7GSR3		(0x3bb << 2) /* R DATX8 n General Status Register 3 */
#define DDR_PUB_DX8GCR0		(0x3c0 << 2) /* R/W DATX8 n General Configuration Register 0 */
#define DDR_PUB_DX8GCR1		(0x3c1 << 2) /* R/W DATX8 n General Configuration Register 1 */
#define DDR_PUB_DX8GCR2		(0x3c2 << 2) /* R/W DATX8 n General Configuration Register 2 */
#define DDR_PUB_DX8GCR3		(0x3c3 << 2) /* R/W DATX8 n General Configuration Register 3 */
#define DDR_PUB_DX8GCR4		(0x3c4 << 2) /* R/W DATX8 n General Configuration Register 4 */
#define DDR_PUB_DX8GCR5		(0x3c5 << 2) /* R/W DATX8 n General Configuration Register 5 */
#define DDR_PUB_DX8GCR6		(0x3c6 << 2) /* R/W DATX8 n General Configuration Register 6 */
#define DDR_PUB_DX8BDLR0	(0x3d0 << 2) /* R/W DATX8 n Bit Delay Line Register 0 */
#define DDR_PUB_DX8BDLR1	(0x3d1 << 2) /* R/W DATX8 n Bit Delay Line Register 1 */
#define DDR_PUB_DX8BDLR2	(0x3d2 << 2) /* R/W DATX8 n Bit Delay Line Register 2 */
#define DDR_PUB_DX8BDLR3	(0x3d4 << 2) /* R/W DATX8 n Bit Delay Line Register 3 */
#define DDR_PUB_DX8BDLR4	(0x3d5 << 2) /* R/W DATX8 n Bit Delay Line Register 4 */
#define DDR_PUB_DX8BDLR5	(0x3d6 << 2) /* R/W DATX8 n Bit Delay Line Register 5 */
#define DDR_PUB_DX8BDLR6	(0x3d8 << 2) /* R/W DATX8 n Bit Delay Line Register 6*/
#define DDR_PUB_DX8LCDLR0	(0x3e0 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 0*/
#define DDR_PUB_DX8LCDLR1	(0x3e1 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 1*/
#define DDR_PUB_DX8LCDLR2	(0x3e2 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 2*/
#define DDR_PUB_DX8LCDLR3	(0x3e3 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 3*/
#define DDR_PUB_DX8LCDLR4	(0x3e4 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 4*/
#define DDR_PUB_DX8LCDLR5	(0x3e5 << 2) /* R/W DATX8 n Local Calibrated Delay Line Register 5 */
#define DDR_PUB_DX8MDLR0	(0x3e8 << 2) /* R/W DATX8 n Master Delay Register 0 */
#define DDR_PUB_DX8MDLR1	(0x3e9 << 2) /* R/W DATX8 n Master Delay Register 1 */
#define DDR_PUB_DX8GTR0		(0x3f0 << 2) /* R/W DATX8 n General Timing Register */
#define DDR_PUB_DX8RSR0		(0x3f4 << 2) /* R DATX8 n Rank Status Register 0 */
#define DDR_PUB_DX8RSR1		(0x3f5 << 2) /* R DATX8 n Rank Status Register 1 */
#define DDR_PUB_DX8RSR2		(0x3f6 << 2) /* R DATX8 n Rank Status Register 2 */
#define DDR_PUB_DX8RSR3		(0x3f7 << 2) /* R DATX8 n Rank Status Register 3 */
#define DDR_PUB_DX8GSR0		(0x3f8 << 2) /* R DATX8 n General Status Register 0 */
#define DDR_PUB_DX8GSR1		(0x3f9 << 2) /* R DATX8 n General Status Register 1 */
#define DDR_PUB_DX8GSR2		(0x3fa << 2) /* R DATX8 n General Status Register 2 */
#define DDR_PUB_DX8GSR3		(0x3fb << 2) /* R DATX8 n General Status Register 3*/

/* PHY General Status Register Bits, register number = 12 */
#define DDR_PUB_PGSR_IDONE		BIT(0)	/* Initialization Done */
#define DDR_PUB_PGSR_ZCERR		BIT(20)	/* Impedance Calibration Error */
#define DDR_PUB_PGSR_TRAIN_ERRBIT_START	19	/* Start error training bit (VERR) */
#define DDR_PUB_PGSR_TRAIN_ERRBIT_LEN	12	/* Length of the error bit field */

#define DDRC_MSTR_DDR3_BIT	BIT(0)
#define DDRC_ECCCFG0_DDR3_BIT	BIT(2)

/* PHY Initialization Register, register number = 1 */
#define DDR_PUB_PIR_INIT	BIT(0) /* Init */
#define DDR_PUB_PIR_ZCAL	BIT(1) /* ZCAL */
#define DDR_PUB_PIR_PLLINIT	BIT(4) /* PLLINIT */
#define DDR_PUB_PIR_DLLCAL	BIT(5) /* DLLCAL */
#define DDR_PUB_PIR_PHYRST	BIT(6) /* PHYRST */
#define DDR_PUB_PIR_DRAMRST	BIT(7)
#define DDR_PUB_PIR_DRAMINIT	BIT(8)
#define DDR_PUB_PIR_WL		BIT(9)
#define DDR_PUB_PIR_DQSGATE	BIT(10)
#define DDR_PUB_PIR_WLADJ	BIT(11)
#define DDR_PUB_PIR_RDDSKW	BIT(12)
#define DDR_PUB_PIR_WRDSKW	BIT(13)
#define DDR_PUB_PIR_RDEYE	BIT(14)
#define DDR_PUB_PIR_WREYE	BIT(15)
#define DDR_PUB_PIR_SRD		BIT(16)
#define DDR_PUB_PIR_VREF	BIT(17)
#define DDR_PUB_PIR_CTLD	BIT(18)
#define DDR_PUB_PIR_RDIMM	BIT(19)

#define DDR_PUB_PIR_PHY_INIT	(DDR_PUB_PIR_ZCAL | DDR_PUB_PIR_PLLINIT | DDR_PUB_PIR_DLLCAL | DDR_PUB_PIR_PHYRST)

#define DDR_PUB_PIR_DRAM_INIT	(DDR_PUB_PIR_DRAMRST | DDR_PUB_PIR_DRAMINIT | DDR_PUB_PIR_WL | DDR_PUB_PIR_DQSGATE |\
				DDR_PUB_PIR_WLADJ | DDR_PUB_PIR_RDDSKW | DDR_PUB_PIR_WRDSKW | DDR_PUB_PIR_RDEYE |\
				DDR_PUB_PIR_WREYE)

#define DDR_PUB_PIR_DRAM_STEP0	(DDR_PUB_PIR_DRAMINIT | DDR_PUB_PIR_DRAMRST)
#define DDR_PUB_PIR_DRAM_STEP1	(DDR_PUB_PIR_WL)
#define DDR_PUB_PIR_DRAM_STEP2	(DDR_PUB_PIR_WLADJ | DDR_PUB_PIR_RDDSKW |\
				 DDR_PUB_PIR_WRDSKW | DDR_PUB_PIR_RDEYE | DDR_PUB_PIR_WREYE)

/* PHY General Status Register Bits, register number = 12 */
#define DDR_PUB_PGSR_IDONE		BIT(0)		/* Initialization Done */
#define DDR_PUB_PGSR_ZCERR		BIT(20)		/* Impedance Calibration Error */
#define DDR_PUB_PGSR_TRAIN_ERRBIT_START 19		/* Start error training bit (VERR) */
#define DDR_PUB_PGSR_TRAIN_ERRBIT_LEN	12		/* Length of the error bit field */
#define DDR_PUB_PGSR_TRAIN_ERRBIT_MASK	0x7ff80000	/* Mask of the error bit field */

/* DDR System General Configuration Register, register number = 36 */
#define DDR_PUB_DSGCR_PUREN	BIT(0)

#define DDR_WAIT_RETRIES	0xff00
#define DDR_ERROR_PORT_SHIFT	16
#define DDR_ERROR(p, c)		((c) << ((p) * DDR_ERROR_PORT_SHIFT))
#define DDR_ERROR_PHY_IDONE	1
#define DDR_ERROR_PHY_TRAIN	2
#define DDR_ERROR_DDRC_MODE	4

#define DDR_PORT_NUM		2

#define DDR_SINGLE_MODE		0
#define DDR_DUAL_MODE		1

#define DDR_CTRL_BASE_OFF	(MMDDR1_CTRL_BASE - MMDDR0_CTRL_BASE)
#define DDR_PHY_BASE_OFF	(MMDDR1_PHY_BASE  - MMDDR0_PHY_BASE)

#define DDR_CTRL(p)		(MMDDR0_CTRL_BASE + ((unsigned long)p) * DDR_CTRL_BASE_OFF)
#define DDR_PHY(p)		(MMDDR0_PHY_BASE  + ((unsigned long)p) * DDR_PHY_BASE_OFF)

/* DCU Command Cache line fields */
#define DCU_DFIELD_DATA		0  /* field width:  5 */
#define DCU_DFIELD_MASK		5  /* field width:  4 */
#define DCU_DFIELD_ADDR		9  /* field width: 18 */
#define DCU_DFIELD_BANK		27 /* field width:  4 */
#define DCU_DFIELD_RANK		31 /* field width:  2 */
#define DCU_DFIELD_CMD		33 /* field width:  4 */
#define DCU_DFIELD_TAG		37 /* field width:  2 */
#define DCU_DFIELD_DTP		39 /* field width:  5 */
#define DCU_DFIELD_RDT		44 /* field width:  3 */

#endif /* DDR_BAIKAL_H */
