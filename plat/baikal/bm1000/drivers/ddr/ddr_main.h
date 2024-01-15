/*
 * Copyright (c) 2021-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DDR_MAIN_H
#define DDR_MAIN_H

#include <stdbool.h>
#include <stdint.h>

#include <baikal_def.h>
#include <platform_def.h>

enum ddr_speed_bin {DDR4_1600, DDR4_1866, DDR4_2133, DDR4_2400, DDR4_2666};

struct ddr_configuration {
	uint32_t single_ddr;	/* single DDR controller configuration */
	uint32_t ecc_on;	/* this is user defined flag to turn on/off ECC support */
	uint32_t dbus_half;	/* this is user defined flag to turn on/off half DDR data bus mode */
	uint32_t clock_mhz;	/* DRAM clock (MHz) (for example 1200 MHz for DDR4-2400) */

	uint32_t dimms;
	uint32_t ranks;
	uint32_t registered_dimm;
	uint32_t registered_ca_stren;
	uint32_t registered_clk_stren;
	uint32_t device_width;	/* x4, x8, x16 components */
	uint32_t bank_groups;	/* bank groups: 4 or 2 (for x16 SDRAM) */
	uint32_t row_address;	/* row address width */
	uint32_t burst_lengths_bitmask;	/* (?) BL=4 bit 2, BL=8 = bit 3 */
	uint32_t mirrored_dimm;
	uint32_t DQ_map[5];	/* DQ Mapping */
	uint32_t DQ_swap_rank;	/* DQ Swap Rank */

	/* DRAM timings */
	uint32_t tCK;		/* Cycle Time */
	uint32_t tWR;		/* WRITE recovery time */
	uint32_t tRP;		/* PRE command period */
	uint32_t tRAS;		/* ACT to PRE command period */
	uint32_t tRC;		/* ACT to ACT or REF command period */
	uint32_t tRCD;		/* ACT to internal read or write delay time */
	uint32_t tWTR_S;	/* Write to Read time for different bank group */
	uint32_t tWTR_L;	/* Write to Read time for same bank group */
	uint32_t tRRD_S;	/* ACTIVATE to ACTIVATE Command delay to different bank group */
	uint32_t tRRD_L;	/* ACTIVATE to ACTIVATE Command delay to same bank group */
	uint32_t tFAW;		/* Four Activate Window delay time */
	uint32_t tRFC1;		/* Refresh Cycle time in 1x mode */
	uint32_t tRFC4;		/* Refresh Cycle time in 4x mode */
	uint32_t tREFI;		/* Periodic Refresh Interval */
	uint32_t tRTP;		/* Read to Precharge for autoprecharge */
	uint32_t tWR_CRC_DM;	/* Write Recovery time when CRC and DM are enabled */
	uint32_t tWTR_S_CRC_DM;	/* Write to Read time for different bank group with both CRC and DM enabled */
	uint32_t tWTR_L_CRC_DM;	/* Write to Read time for same bank group with both CRC and DM enabled */
	uint32_t tDLLK;		/* DLL Locking time */
	uint32_t tXP;		/* Exit Power Down with DLL on */
	uint32_t tMOD;		/* Mode Register Set time */
	uint32_t tCCD_S;	/* CAS_n to CAS_n command Delay for different bank group */
	uint32_t tCCD_L;	/* CAS_n to CAS_n command Delay for same bank group */
	uint32_t tCKSRX;	/* Valid Clock Requirement before Self Refresh Exit or Power-Down Exit or Reset Exit */
	uint32_t tCKE;		/* CKE minimum pulse width */
	uint32_t tMRD_PDA;	/* Mode Register Set command cycle time in PDA mode */
	uint32_t tRASmax;	/* Maximum ACT to PRE command period */
	uint32_t tMPX_S;	/* CS setup time to CKE */
	uint32_t tMPX_LH;	/* CS_n Low hold time to CKE rising edge */
	uint32_t tCPDED;	/* Command pass disable delay */
	uint32_t tXS;		/* Exit Self Refresh to commands not requiring a locked DLL */
	uint32_t tXS_FAST;	/* Exit Self Refresh to ZQCL,ZQCS and MRS */
	uint32_t tXS_ABORT;	/* SRX to commands not requiring a locked DLL in Self Refresh ABORT */
	uint32_t tXSDLL;	/* Exit Self Refresh to tXSDLL commands requiring a locked DLL */
	uint32_t tXMP;		/* Exit MPSM to commands not requiring a locked DLL */
	uint32_t tXMPDLL;	/* Exit MPSM to commands requiring a locked DLL */
	uint32_t tCKMPE;	/* Valid clock requirement after MPSM entry */

	uint32_t CL;		/* CAS Latency */
	uint32_t CWL;		/* CAS Write Latency */
	uint32_t WCL;		/* Write Command Latency when CRC and DM are both enabled */
	uint32_t PL;		/* C/A Parity Latency */
	uint32_t AL;		/* Additive Latency */
	uint32_t RL;		/* Read Latency */
	uint32_t WL;		/* Write Latency */

	/* DRAM impedance settings */
	uint32_t RTT_PARK;	/* PARK pullup impedance */
	uint32_t RTT_NOM;	/* NOM pullup impedance */
	uint32_t RTT_WR;	/* WR pullup impedance */
	uint32_t DIC;		/* output driver impedance */

	/* PHY impedance settings */
	uint32_t PHY_ODI_PU;
	uint32_t PHY_ODI_PD;
	uint32_t PHY_ODT;

	uint32_t PHY_HOST_VREF;
	uint32_t PHY_DRAM_VREF;
};

/*
 * DDR fine tune config
 */

struct ddr_local_conf {
	uint8_t 	magic[4];
#define ELP_DDR_CONF_MAGIC	{'E', 'L', 'P', '0' + BOARD_VER}
	uint16_t	freq; /* 0 - default */
	uint8_t		dimms; /* bit 0 - channel number, bit 1 - two DIMMS in channel */
	uint8_t		cl; /* CAS Latency. 0 - default, 1 - increase by 1 */
	uint8_t		al; /* 0 - not used, 1 - CL-1, ... */
	uint8_t		rtt_wr;
	uint8_t		rtt_nom;
	uint8_t		rtt_park;
	uint8_t		odi;
	uint8_t		phy_odt;
	uint8_t		phy_odi_pu;
	uint16_t	crc;
};

int ddr_conf(unsigned dimm_cfg);
int dram_init(void);
int ddr_init(int port, bool dual_mode, struct ddr_configuration *data);
int ddr_lcru_initport(int port, uint32_t clock_mhz);
int ddr_init_ecc_memory(int port);

#endif /* DDR_MAIN_H */
