/*
 * Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>

#include "ddr_phy_main.h"
#include "ddr_phy_tmp_regs.h"

#define DDRPHY_PMU_DCCM_ADDR	0x00054000

/**
 * @brief reads training results
 * Read the Firmware Message Block via APB read commands to the DMEM address to
 * obtain training results.
 */
static uint8_t getMBdata_u8(int port, size_t offset)
{
	uint16_t mem = DDRPHY_READ_REG16(port, (DDRPHY_PMU_DCCM_ADDR + (offset / 2)));

	return (offset % 2) ? (uint8_t)(mem >> 8) : (uint8_t)mem;
}

static uint16_t getMBdata_u16(int port, size_t offset)
{
	return (uint16_t)DDRPHY_READ_REG16(port, (DDRPHY_PMU_DCCM_ADDR + (offset / 2)));
}

/* return: PHY training result - TxDqsDly maximum, in PHY clocks */
static unsigned int phy_getTxDqsDlyMax(int port)
{
	unsigned int dly_max = 0;
	const int pstate = 0;
	const uint32_t TxDqsDlyTgX_addr[4] = {
		csr_TxDqsDlyTg0_ADDR, csr_TxDqsDlyTg1_ADDR,
		csr_TxDqsDlyTg2_ADDR, csr_TxDqsDlyTg3_ADDR
	};

	for (unsigned int byte = 0; byte < 9; byte++) {
		for (unsigned int lane = 0; lane <= 1; lane++) {
			unsigned int b_addr = lane << 8;
			unsigned int c_addr = byte << 12;
			unsigned int p_addr = pstate << 20;

			for (int i = 0; i < 4; i++) {
				uint16_t val = DDRPHY_READ_REG16(port,
					(p_addr | tDBYTE | c_addr | b_addr | TxDqsDlyTgX_addr[i]));
				unsigned int dly_frac = val & 0x1f;	  /* [4..0] bits for fractional value */
				unsigned int dly_int  = (val >> 6) & 0xf; /* [9..6] bits for integer value */

				if (dly_frac != 0) {
					dly_int += 1;
				}

				if (dly_int > dly_max) {
					dly_max = dly_int;
				}
			}
		}
	}

	/* convert result from UI to PHY clk value */
	return (dly_max + 1) / 2;
}

/* get rd2wr training result for single-rank DDR (just get R0-R0 result) */
/* see phy pub databook "Rank-to-Rank Spacing" */
static unsigned int phy_getRd2WrCDDsr(int port, unsigned int cdd_00_offset)
{
	int8_t rd2wr = getMBdata_u8(port, cdd_00_offset);

	return (rd2wr >= 0) ? rd2wr : -rd2wr;
}

/* get rd2wr training result for multi-ranks DDR (seek maximum for all ranks) */
/* see phy pub databook "Rank-to-Rank Spacing" */
static unsigned int phy_getRd2WrCDDmr(int port,
				      unsigned int cdd_33_offset,
				      unsigned int cdd_00_offset,
				      unsigned int cs_mask)
{
	unsigned int rd2wr_max = 0;

	/* read uint8_t CDD data by uint16_t bus read transaction */
	unsigned int start_offset_u16 = cdd_33_offset & ~0x1;
	unsigned int end_offset_u16 = cdd_00_offset & ~0x1;
	int cs_high = 3;
	int cs_low = 3;

	for (unsigned int offset = start_offset_u16; offset <= end_offset_u16; offset += 2) {
		uint16_t data = getMBdata_u16(port, offset);

		if ((offset + 0) >= cdd_33_offset) {
			if ((cs_mask & (1 << cs_high)) && (cs_mask & (1 << cs_low))) {
				/* low byte data process */
				int8_t rd2wr = data & 0xff;
				unsigned int rd2wr_abs = (rd2wr >= 0) ? rd2wr : -rd2wr;

				if (rd2wr_abs > rd2wr_max) {
					rd2wr_max = rd2wr_abs;
				}
			}
			cs_low -= 1;
			if (cs_low < 0) {
				cs_low = 3;
				cs_high -= 1;
			}
		}
		if ((offset + 1) <= cdd_00_offset) {
			if ((cs_mask & (1 << cs_high)) && (cs_mask & (1 << cs_low))) {
				/* high byte data process */
				int8_t rd2wr = data >> 8;
				unsigned int rd2wr_abs = (rd2wr >= 0) ? rd2wr : -rd2wr;

				if (rd2wr_abs > rd2wr_max) {
					rd2wr_max = rd2wr_abs;
				}
			}
			cs_low -= 1;
			if (cs_low < 0) {
				cs_low = 3;
				cs_high -= 1;
			}
		}
	}

	return rd2wr_max;
}

/* get wr2wr training result for multi-ranks DDR (seek maximum for all ranks) */
/* see phy pub databook "Rank-to-Rank Spacing" */
static unsigned int phy_getWr2WrCDDmr(int port,
				      unsigned int cdd_32_offset,
				      unsigned int cdd_01_offset,
				      unsigned int cs_mask)
{
	int wr2wr_max = 0;

	/* read uint8_t CDD data by uint16_t bus read transaction */
	unsigned int start_offset_u16 = cdd_32_offset & ~0x1;
	unsigned int end_offset_u16 = cdd_01_offset & ~0x1;
	int cs_high = 3;
	int cs_low = 2;

	for (unsigned int offset = start_offset_u16; offset <= end_offset_u16; offset += 2) {
		uint16_t data = getMBdata_u16(port, offset);

		if ((offset + 0) >= cdd_32_offset) {
			if ((cs_mask & (1 << cs_high)) && (cs_mask & (1 << cs_low))) {
				/* low byte data process */
				int8_t wr2wr = data & 0xff;

				if (wr2wr > wr2wr_max) {
					wr2wr_max = wr2wr;
				}
			}
			cs_low -= 1;
			if (cs_low < 0) {
				cs_low = 3;
				cs_high -= 1;
			}
			if (cs_low == cs_high) {
				cs_low -= 1;
			}
		}
		if ((offset + 1) <= cdd_01_offset) {
			if ((cs_mask & (1 << cs_high)) && (cs_mask & (1 << cs_low))) {
				/* high byte data process */
				int8_t wr2wr = data >> 8;

				if (wr2wr > wr2wr_max) {
					wr2wr_max = wr2wr;
				}
			}
			cs_low -= 1;
			if (cs_low < 0) {
				cs_low = 3;
				cs_high -= 1;
			}
			if (cs_low == cs_high) {
				cs_low -= 1;
			}
		}
	}

	return wr2wr_max >= 0 ? wr2wr_max : 0;
}

/* get rd2rd training result for multi-ranks DDR (seek maximum for all ranks) */
/* see phy pub databook "Rank-to-Rank Spacing" */
static unsigned int phy_getRd2RdCDDmr(int port,
				      unsigned int cdd_32_offset,
				      unsigned int cdd_01_offset,
				      unsigned int cs_mask)
{
	int rd2rd_max = 0;

	/* read uint8_t CDD data by uint16_t bus read transaction */
	unsigned int start_offset_u16 = cdd_32_offset & ~0x1;
	unsigned int end_offset_u16 = cdd_01_offset & ~0x1;
	int cs_high = 3;
	int cs_low = 2;

	for (unsigned int offset = start_offset_u16; offset <= end_offset_u16; offset += 2) {
		uint16_t data = getMBdata_u16(port, offset);

		if ((offset + 0) >= cdd_32_offset) {
			if ((cs_mask & (1 << cs_high)) && (cs_mask & (1 << cs_low))) {
				/* low byte data process */
				int8_t rd2rd = data & 0xff;

				if (rd2rd > rd2rd_max) {
					rd2rd_max = rd2rd;
				}
			}
			cs_low -= 1;
			if (cs_low < 0) {
				cs_low = 3;
				cs_high -= 1;
			}
			if (cs_low == cs_high) {
				cs_low -= 1;
			}
		}
		if ((offset + 1) <= cdd_01_offset) {
			if ((cs_mask & (1 << cs_high)) && (cs_mask & (1 << cs_low))) {
				/* high byte data process */
				int8_t rd2rd = data >> 8;

				if (rd2rd > rd2rd_max) {
					rd2rd_max = rd2rd;
				}
			}
			cs_low -= 1;
			if (cs_low < 0) {
				cs_low = 3;
				cs_high -= 1;
			}
			if (cs_low == cs_high) {
				cs_low -= 1;
			}
		}
	}

	return rd2rd_max >= 0 ? rd2rd_max : 0;
}

int phyinit_H_readMsgBlock(int port, struct ddr_configuration *data)
{
	/*
	 * 1. Enable access to the internal CSRs by setting the MicroContMuxSel CSR to 0.
	 * This allows the memory controller unrestricted access to the configuration CSRs.
	 */
	DDRPHY_WRITE_REG16(port, (tAPBONLY | csr_MicroContMuxSel_ADDR), 0x0);

	uint8_t cs_fail = getMBdata_u8(port, 0x14); /* offsetof(pmu_smb_ddr4_t, CsTestFail) */
	/* This field will be set if training fails on any rank. */
	if (cs_fail) {
		ERROR("CS fail mask=0x%X\n", cs_fail);
		return -1;
	}

	/* save extra delay constrains for umctl2 timing correct */
	data->trn_res.TxDqsDlyMax = phy_getTxDqsDlyMax(port);

	uint8_t CsPresent = data->ranks == 1 ? 0x1 : 0x3;

	if (data->dimms == 2) {
		CsPresent = CsPresent | (CsPresent << 2);
	}
	data->trn_res.rd2wr_CDD_sr = phy_getRd2WrCDDsr(port, 0x4c); /* offsetof(pmu_smb_ddr4_t, CDD_RW_0_0) */
	/* offsetof(pmu_smb_ddr4_t, CDD_RW_3_3), offsetof(pmu_smb_ddr4_t, CDD_RW_0_0) */
	data->trn_res.rd2wr_CDD_mr = phy_getRd2WrCDDmr(port, 0x3d, 0x4c, CsPresent);
	/* offsetof(pmu_smb_ddr4_t, CDD_WW_3_2), offsetof(pmu_smb_ddr4_t, CDD_WW_0_1) */
	data->trn_res.wr2wr_CDD_mr = phy_getWr2WrCDDmr(port, 0x31, 0x3c, CsPresent);
	/* offsetof(pmu_smb_ddr4_t, CDD_RR_3_2), offsetof(pmu_smb_ddr4_t, CDD_RR_0_1) */
	data->trn_res.rd2rd_CDD_mr = phy_getRd2RdCDDmr(port, 0x25, 0x30, CsPresent);

	/* 2. Isolate the APB access from the internal CSRs by setting the MicroContMuxSel CSR to 1. */
	/*	This allows the firmware unrestricted access to the configuration CSRs. */
	DDRPHY_WRITE_REG16(port, (tAPBONLY | csr_MicroContMuxSel_ADDR), 0x1);

	return 0;
}
