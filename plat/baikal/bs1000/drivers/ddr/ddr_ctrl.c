/*
 * Copyright (c) 2022-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include "ddr_ctrl.h"
#include "ddr_master.h"

#include <platform_def.h>

void umctl2_enter_SR(int port)
{
	uint32_t reg_val;
	/* enter DRAM self-refresh mode */
	reg_val = BS_DDRC_READ(port, PWRCTL);
	reg_val |= (1 << 5);
	BS_DDRC_WRITE(port, PWRCTL, reg_val);

	/* delay 1 us */
	udelay(1);

	/* stop DRAM clock */
	reg_val = BS_DDRC_READ(port, PWRCTL);
	reg_val |= (1 << 3);
	BS_DDRC_WRITE(port, PWRCTL, reg_val);
}

void umctl2_exit_SR(int port)
{
	uint32_t reg_val;
	/* run DRAM clock */
	reg_val = BS_DDRC_READ(port, PWRCTL);
	reg_val &= ~(1 << 3);
	BS_DDRC_WRITE(port, PWRCTL, reg_val);

	/* delay 1 us */
	udelay(1);

	/* exit DRAM self-refresh mode */
	reg_val = BS_DDRC_READ(port, PWRCTL);
	reg_val &= ~(1 << 5);
	BS_DDRC_WRITE(port, PWRCTL, reg_val);

	/* delay 1 us */
	udelay(1);
}

static void ctrl_set_sar(int port)
{
	uint32_t chip_offset = PLATFORM_CHIP_MEM_OFFSET(DDR_PORT_CHIP(port)) >> 28;

	/* program SAR registers (UMCTL2_SARMINSIZE = 256 MiB) */
	/* configure a SARBASE0 reg. */
	BS_DDRC_WRITE(port, SARBASE0, chip_offset + 2 * 4); /* region 0: 2 GiB base address */
	/* configure a SARSIZE0 reg. */
	BS_DDRC_WRITE(port, SARSIZE0, 2 * 4 - 1); /* region 0: 2 GiB size */

	/* configure a SARBASE1 reg. */
	BS_DDRC_WRITE(port, SARBASE1, chip_offset + 34 * 4); /* region 1: 34 GiB base address */
	/* configure a SARSIZE1 reg. */
	BS_DDRC_WRITE(port, SARSIZE1, 30 * 4 - 1); /* region 1: 30 GiB size */

	/* configure a SARBASE2 reg. */
	BS_DDRC_WRITE(port, SARBASE2, chip_offset + 544 * 4); /* region 2: 544 GiB base address */
	/* configure a SARSIZE2 reg. */
	BS_DDRC_WRITE(port, SARSIZE2, 64 * 4 - 1); /* region 2: 64 GiB size (note: it's a max value for SARSIZE) */

	/* configure a SARBASE3 reg. */
	/* note: we can't set 8 TiB region, SARSIZE have a max value 64 GiB */
	BS_DDRC_WRITE(port, SARBASE3, chip_offset + 608 * 4); /* region 3: 544 + 64 GiB base address */
	/* configure a SARSIZE3 reg. */
	BS_DDRC_WRITE(port, SARSIZE3, 64 * 4 - 1); /* region 3: 64 GiB size (note: it's a max value for SARSIZE) */
}

/** @brief increments HIF address and strips address by mask in DRAMconfig
 */
static uint32_t hif_addr_increment(uint32_t hif_addr, struct ddr_configuration *data)
{
	/* note: shift input address by -3 for APP_ADDR => HIF_ADDR conversion */
	uint64_t strip_addr_mask = data->addr_strip_bit >> 3;

	/*
	 * note: be careful for max HIF_ADDR value 33 bit
	 * (HIF_ADDR width is 34 bits - 128 GiB address space is maximum)
	 */
	if (hif_addr > 33) {
		/* max value = 33, but gets incremented once more later */
		ERROR("Maximum memory size per channel exceeded\n");
	}

	hif_addr++;
	/* note: be careful by SAR address bit conversion for bit number calculation */
	while ((strip_addr_mask & (1ULL << hif_addr))) {
		hif_addr++; /* strip address */
	}

	return hif_addr;
}

static void ctrl_set_addr_map(int port, struct ddr_configuration *data)
{
	uint32_t map0_val = 0x00001818; /* map CS1->hifA31, CS0->hifA30 */
	uint32_t map1_val = 0x003f0909; /* map BA2->none, BA1->hifA12, BA0->hifA11 */
	uint32_t map2_val = 0x01010100; /* map COL5,4,3,2->hifA6,A5,A4,A2 */
	uint32_t map3_val = 0x01010101; /* map COL9,8,7,6->hifA10,A9,A8,A7 */
	uint32_t map4_val = 0x00001f1f; /* map COL11,10->none */
	uint32_t map5_val = 0x08080808; /* map ROW11->hifA25, ROW10..2->hifA24,A16, ROW1,0->hifA15,A14 */
	uint32_t map6_val = 0x08080808; /* map ROW15,14,13,12->hifA29,A28,A27,A26 */
	uint32_t map7_val = 0x00000f0f; /* map ROW17->None, ROW16->none */
	uint32_t map8_val = 0x00000a01; /* map BG1,0->hifA13,A3 */

	/* we set default mapping (HIF->DRAM) (see Table2-20 BL=8,DDR4 recommendation in umctl2 DataBook) */
	/* COL2  ->HIF A2 */
	/* BG0	->HIF A3 */
	uint32_t hif_addr = 4; /* next empty HIF address */

	map2_val &= ~GENMASK(12, 8);
	map2_val |= GENMASK(12, 8) & ((hif_addr - 3) << 8); /* map COL3->HIF */
	hif_addr = hif_addr_increment(hif_addr, data);
	map2_val &= ~GENMASK(19, 16);
	map2_val |= GENMASK(19, 16) & ((hif_addr - 4) << 16); /* map COL4->HIF */
	hif_addr = hif_addr_increment(hif_addr, data);
	map2_val &= ~GENMASK(27, 24);
	map2_val |= GENMASK(27, 24) & ((hif_addr - 5) << 24); /* map COL5->HIF */
	hif_addr = hif_addr_increment(hif_addr, data);

	map3_val &= ~GENMASK(4, 0);
	map3_val |= GENMASK(4, 0) & (hif_addr - 6); /* map COL6->HIF */
	hif_addr = hif_addr_increment(hif_addr, data);
	map3_val &= ~GENMASK(12, 8);
	map3_val |= GENMASK(12, 8) & ((hif_addr - 7) << 8); /* map COL7->HIF */
	hif_addr = hif_addr_increment(hif_addr, data);
	map3_val &= ~GENMASK(20, 16);
	map3_val |= GENMASK(20, 16) & ((hif_addr - 8) << 16); /* map COL8->HIF */
	hif_addr = hif_addr_increment(hif_addr, data);
	map3_val &= ~GENMASK(28, 24);
	map3_val |= GENMASK(28, 24) & ((hif_addr - 9) << 24); /* map COL9->HIF */
	hif_addr = hif_addr_increment(hif_addr, data);

	if (data->bank_groups > 2) {
		map8_val &= ~GENMASK(13, 8);
		map8_val |= GENMASK(13, 8) & ((hif_addr - 3) << 8); /* map BG1->HIF */
		hif_addr = hif_addr_increment(hif_addr, data);
	} else {
		map8_val &= ~GENMASK(13, 8);
		map8_val |= GENMASK(13, 8) & (63 << 8); /* map BG1->none */
	}

#if DDR_DEBUG_TEST_MAPPING
#warning "DEBUG: special DDR address map for DIMM testing"
	int verbose = ddrlib_verbose;

	if (data->ranks == 2) {
		INFO("DEBUG: special DDR address map for 2R DIMM testing\n");
		map0_val &= ~GENMASK(4, 0);
		map0_val |= GENMASK(4, 0) & (hif_addr - 6); /* CS0->hifXX */
		hif_addr = hif_addr_increment(hif_addr, data);
	} else {
		map0_val &= ~GENMASK(4, 0);
		/* CS0->none: map to none for 1-rank DIMMs (that's not external CS0 signal) */
		map0_val |= 31;
	}

	if (data->dimms == 2) {
		INFO("DEBUG: special DDR address map for double DIMM(x2) testing\n");
		map0_val &= ~GENMASK(12, 8);
		map0_val |= GENMASK(12, 8) & ((hif_addr - 7) << 8); /* CS1->hifXX */
		hif_addr = hif_addr_increment(hif_addr, data);
	} else {
		map0_val &= ~GENMASK(12, 8);
		map0_val |= GENMASK(12, 8) & (31 << 8); /* CS1->none */
	}
#endif
	map1_val &= ~GENMASK(5, 0);
	map1_val |= GENMASK(5, 0) & (hif_addr - 2); /* map BA0->HIF */
	hif_addr = hif_addr_increment(hif_addr, data);
	map1_val &= ~GENMASK(13, 8);
	map1_val |= GENMASK(13, 8) & ((hif_addr - 3) << 8); /* map BA1->HIF */
	hif_addr = hif_addr_increment(hif_addr, data);

	map5_val &= ~GENMASK(3, 0);
	map5_val |= GENMASK(3, 0) & (hif_addr  - 6); /* map ROW0->HIF */
	hif_addr = hif_addr_increment(hif_addr, data);
	map5_val &= ~GENMASK(11, 8);
	map5_val |= GENMASK(11, 8) & ((hif_addr  - 7) << 8); /* map ROW1->HIF */
	hif_addr = hif_addr_increment(hif_addr, data);
	map5_val &= ~GENMASK(19, 16);
	map5_val |= GENMASK(19, 16) & ((hif_addr - 8) << 16); /* map ROW10..2->HIF */
	hif_addr += 8;
	hif_addr = hif_addr_increment(hif_addr, data);
	map5_val &= ~GENMASK(27, 24);
	map5_val |= GENMASK(27, 24) & ((hif_addr - 17) << 24); /* map ROW11->HIF */
	hif_addr = hif_addr_increment(hif_addr, data);

	map6_val &= ~GENMASK(3, 0);
	map6_val |= GENMASK(3, 0) & (hif_addr - 18); /* map ROW12->HIF */
	hif_addr = hif_addr_increment(hif_addr, data);
	map6_val &= ~GENMASK(11, 8);
	map6_val |= GENMASK(11, 8) & ((hif_addr - 19) << 8); /* map ROW13->HIF */
	hif_addr = hif_addr_increment(hif_addr, data);

	if ((data->row_address - 1) >= 14) {
		map6_val &= ~GENMASK(19, 16);
		map6_val |= GENMASK(19, 16) & ((hif_addr - 20) << 16); /* ROW14->HIF */
		hif_addr = hif_addr_increment(hif_addr, data);
	} else {
		map6_val &= ~GENMASK(19, 16);
		map6_val |= GENMASK(19, 16) & (15 << 16); /* ROW14->none */
	}
	if ((data->row_address - 1) >= 15) {
		map6_val &= ~GENMASK(27, 24);
		map6_val |= GENMASK(27, 24) & ((hif_addr - 21) << 24); /* ROW15->HIF */
		hif_addr = hif_addr_increment(hif_addr, data);
	} else {
		map6_val &= ~GENMASK(27, 24);
		map6_val |= GENMASK(27, 24) & (15 << 24); /* ROW15->none */
	}
	if ((data->row_address - 1) >= 16) {
		map7_val &= ~GENMASK(3, 0);
		map7_val |= GENMASK(3, 0) & (hif_addr - 22); /* ROW16->HIF */
		hif_addr = hif_addr_increment(hif_addr, data);
	} else {
		map7_val &= ~GENMASK(3, 0);
		map7_val |= GENMASK(3, 0) & 15; /* ROW16->none */
	}
	if ((data->row_address - 1) == 17) {
		map7_val &= ~GENMASK(11, 8);
		map7_val |= GENMASK(11, 8) & ((hif_addr - 23) << 8); /* ROW17->HIF */
		hif_addr = hif_addr_increment(hif_addr, data);
	} else {
		map7_val &= ~GENMASK(11, 8);
		map7_val |= GENMASK(11, 8) & (15 << 8); /* ROW17->none */
	}

	/* DIMM_0 have 00b and 01b rank addresses on the board; DIMM_1 have 10b and 11b rank addresses on the Board */
#if !DDR_DEBUG_TEST_MAPPING
	if (data->ranks == 2) {
		map0_val &= ~GENMASK(4, 0);
		map0_val |= GENMASK(4, 0) & (hif_addr - 6); /* CS0->hifXX */
		hif_addr = hif_addr_increment(hif_addr, data);
	} else {
		map0_val &= ~GENMASK(4, 0);
		map0_val |= GENMASK(4, 0) & 31; /* CS0->none: map to none for 1-rank DIMMs (that's not external CS0 signal) */
	}

	if (data->dimms == 2) {
		map0_val &= ~GENMASK(12, 8);
		map0_val |= GENMASK(12, 8) & ((hif_addr - 7) << 8); /* CS1->hifXX */
		hif_addr = hif_addr_increment(hif_addr, data);
	} else {
		map0_val &= ~GENMASK(12, 8);
		map0_val |= GENMASK(12, 8) & (31 << 8); /* CS1->none */
	}
#endif
	/* configure a ADDRMAP0 reg. */
	BS_DDRC_WRITE(port, ADDRMAP0, map0_val);
	/* configure a ADDRMAP1 reg. */
	BS_DDRC_WRITE(port, ADDRMAP1, map1_val);
	/* configure a ADDRMAP2 reg. */
	BS_DDRC_WRITE(port, ADDRMAP2, map2_val);
	/* configure a ADDRMAP3 reg. */
	BS_DDRC_WRITE(port, ADDRMAP3, map3_val);
	/* configure a ADDRMAP4 reg. */
	BS_DDRC_WRITE(port, ADDRMAP4, map4_val);
	/* configure a ADDRMAP5 reg. */
	BS_DDRC_WRITE(port, ADDRMAP5, map5_val);
	/* configure a ADDRMAP6 reg. */
	BS_DDRC_WRITE(port, ADDRMAP6, map6_val);
	/* configure a ADDRMAP7 reg. */
	BS_DDRC_WRITE(port, ADDRMAP7, map7_val);
	/* configure a ADDRMAP8 reg. */
	BS_DDRC_WRITE(port, ADDRMAP8, map8_val);
}

int ctrl_init(int port, struct ddr_configuration *data)
{
	uint32_t regval = 0;

	/* configure a MSTR reg. */
	regval |= 1 << 4; /* ctl operates as ddr4 */
	regval |= GENMASK(19, 16) & (0x4 << 16); /* BL8 */
	/*
	 * We have two CS signals (two ranks) per DIMM slot on DBM board.
	 * Valid values for Mstr.active_ranks are: 0x1, 0x11, 0x1111
	 */
	if (data->dimms == 1) {
		regval |=  GENMASK(27, 24) & (((1 << data->ranks) - 1) << 24);
	} else {
		regval |=  GENMASK(27, 24) & (0xf << 24);
	}
	if (data->timing_2t) {
		regval |=  1 << 10; /* enable 2T mode */
	}
	if (data->device_width == 8) {
		regval |= GENMASK(31, 30) & (1 << 30);
	}
	BS_DDRC_WRITE(port, MSTR, regval);

	if (data->crc_on) {
		/* enable interrupt for DFI alert error */
		BS_DDRC_WRITE(port, CRCPARCTL0, 0x1);
		BS_DDRC_WRITE(port, CRCPARCTL1, (MR5_DM << 7) | (1 << 4) | !!data->par_on);
	}

	/* configure a DIMMCTL reg. */
	regval = (1 << 0); /* Send MRS commands to each ranks separately */
	if (data->mirrored_dimm) {
		regval |=  (1 << 1);
	} else {
		regval &= ~(1 << 1);
	}
	if (data->registered_dimm) {
		regval |=  (1 << 2);
	} else {
		regval &= ~(1 << 2);
	}
	BS_DDRC_WRITE(port, DIMMCTL, regval);

	/* configure a DBICTL reg. */
	BS_DDRC_WRITE(port, DBICTL, 0); /* DBI=off, DM=off */

	/* configure a RANKCTL reg. (MARK) */
	BS_DDRC_WRITE(port, RANKCTL, 0x33f);

	/* configure a DFITMG0 reg. */
	regval  = (data->WL - 5 + data->registered_dimm); /* see PHY PUB DataBook, Table5-3 (p.91) */
	regval |= (2 << 8); /* see PHY PUB DataBook, Table5-3 (p.91) */
	regval |= (1 << 15);
	regval |= ((data->RL - 5 + data->registered_dimm) << 16); /* see PHY PUB DataBook, Table5-5 (p.92) */
	regval |= (1 << 23);
	regval |= (((6 + data->registered_dimm + 1) / 2) << 24); /* see PHY PUB DataBook, Table5-1 (p.90) */
	BS_DDRC_WRITE(port, DFITMG0, regval);

	/* configure a DFITMG1 reg. */
	regval  = 2; /* see PHY PUB DataBook, Table5-12 (p.97) */
	regval |= (2 << 8); /* see PHY PUB DataBook, Table5-12 (p.97) */
	 /* see PHY PUB DataBook, Table5-3 (p.91) << 16); (t_ctrl_delay + (6 + 4) + TxDqsDly) / 2 */
	regval |= GENMASK(20, 16) & ((((DDR_PHY_TWRDATA_DELAY + 1) / 2 + 1)) << 16);
	/* add one additional clk for RDIMM (see F0RC0F settings: 1 clk delay for parity by default) */
	regval |= ((data->registered_dimm ? 1 : 0) << 24);
	regval &= ~GENMASK(31, 28); /* note: have to set non-zero value for CAL mode (we don't use CAL mode) */
	const uint32_t dfitmg1_val = regval;

	BS_DDRC_WRITE(port, DFITMG1, regval);

	/* configure a DFITMG2 reg. */
	regval  = GENMASK(5, 0) & (data->WL - 5 + data->registered_dimm); /* see PHY PUB DataBook, Table5-3 (p.91) */
	regval |= GENMASK(14, 8) & ((data->RL - 5 + data->registered_dimm) << 8); /* see PHY PUB DataBook, Table5-5 (p.92) */
	BS_DDRC_WRITE(port, DFITMG2, regval);

	/* configure a DRAMTMG0 reg. */
	unsigned int t_ras_min = data->timing_2t ? (data->tRAS + 1) / 2 : data->tRAS / 2;

	regval  = GENMASK(5, 0) & t_ras_min;
	regval |= GENMASK(14, 8) & (((data->tRASmax / 1024 - 1) / 2) << 8);
	regval |= GENMASK(21, 16) & (((data->tFAW + 1) / 2) << 16);
	unsigned int wr2pre = (data->WL + 4 + data->tWR);

	regval |= GENMASK(30, 24) & ((data->timing_2t ? (wr2pre + 1) / 2 : wr2pre / 2) << 24);
	BS_DDRC_WRITE(port, DRAMTMG0, regval);

	/* configure a DRAMTMG1 reg. */
	regval  = GENMASK(6, 0) & ((data->tRC + 1) / 2);
	unsigned int rd2pre = ((data->AL + data->tRTP) > (data->RL + 4 - data->tRP)) ?
							(data->AL + data->tRTP) : (data->RL + 4 - data->tRP);
	regval |= GENMASK(13, 8) & ((data->timing_2t ? (rd2pre + 1) / 2 : rd2pre / 2) << 8);
	regval |= GENMASK(20, 16) & (((data->tXP + data->PL + 1) / 2) << 16);
	BS_DDRC_WRITE(port, DRAMTMG1, regval);

	/* configure a DRAMTMG2 reg. */
	unsigned int wr2rd = data->CWL + data->PL + 4 + data->tWTR_L; /* CRC off */

	regval  = GENMASK(5, 0) & ((wr2rd + 1) / 2);
	/* see umctl2 Databook for "rd2wr" description */
	regval |= GENMASK(13, 8) & (((data->RL + 4 + 1 + (data->wr_preamble_2CK ? 2 : 1) - data->WL + 1) / 2) << 8);
	regval |= GENMASK(21, 16) & (((data->RL + 1 + data->registered_dimm) / 2) << 16);
	regval |= GENMASK(29, 24) & (((data->WL + 1 + data->registered_dimm) / 2) << 24);
	BS_DDRC_WRITE(port, DRAMTMG2,  regval);

	/* configure a DRAMTMG3 reg. */
	regval  = GENMASK(9, 0) & (DIV_ROUND_UP_2EVAL(data->tMOD + data->PL, 2) + !!data->registered_dimm);
	regval |= GENMASK(17, 12) & (DIV_ROUND_UP_2EVAL(data->tMOD + data->PL, 2) << 12); /* tMRD_PAR */
	BS_DDRC_WRITE(port, DRAMTMG3,  regval);

	/* configure a DRAMTMG4 reg. */
	regval  = GENMASK(4, 0) & (data->tRP / 2 + 1);
	regval |= GENMASK(11, 8) & (((data->tRRD_L + 1) / 2) << 8);
	regval |= GENMASK(19, 16) & (((data->tCCD_L + 1) / 2) << 16);
	regval |= GENMASK(28, 24) & (((data->tRCD > data->AL) ? (data->tRCD - data->AL + 1) / 2 : 1) << 24);
	BS_DDRC_WRITE(port, DRAMTMG4,  regval);

	/* configure a DRAMTMG5 reg. */
	regval  = GENMASK(4, 0) & DIV_ROUND_UP_2EVAL(data->tCKE, 2);
	/* DDR4: tCKE + 1 (+ PL(parity latency)(*)) */
	regval |= GENMASK(13, 8) & (DIV_ROUND_UP_2EVAL(data->tCKE + 1 + data->PL, 2) << 8);
	/* DDR4: max (10 ns, 5 tCK) (+ PL(parity latency)(*)) */
	regval |= GENMASK(19, 16) & (DIV_ROUND_UP_2EVAL(CLOCK_NS(10) + data->PL, 2) << 16);
	regval |= GENMASK(27, 24) & (DIV_ROUND_UP_2EVAL(data->tCKSRX, 2) << 24); /* DDR4: tCKSRX */
	BS_DDRC_WRITE(port, DRAMTMG5,  regval);

	/* configure a DRAMTMG8 reg. */
	regval  = GENMASK(6, 0) & (((data->tXS + 31) / 32 + 1) / 2 + 1); /* note: we see bug by VCS testing: */
	/* tXS = 320 tCK, but measured delay is 306 tCK */
	regval |= GENMASK(14, 8) & ((((data->tXSDLL + 31) / 32 + 1) / 2 + 1) << 8);
	regval |= GENMASK(22, 16) & ((((data->tXS_ABORT + 31) / 32 + 1) / 2 + 1) << 16); /* add +1 for all register fields */
	regval |= GENMASK(30, 24) & ((((data->tXS_FAST + 31) / 32 + 1) / 2 + 1) << 24);
	BS_DDRC_WRITE(port, DRAMTMG8, regval);

	/* configure a DRAMTMG9 reg. */
	/* note: don't correct wr2rd_s timing by PHY training results: umctl2 databook don't say nothing */
	regval  = GENMASK(5, 0) & ((data->CWL + data->PL + 4 + data->tWTR_S + 1) / 2);
	regval |= GENMASK(11, 8) & (((data->tRRD_S + 1) / 2) << 8);
	regval |= GENMASK(18, 16) & (((data->tCCD_S + 1) / 2) << 16);
	regval |= GENMASK(30, 30) & ((data->wr_preamble_2CK ? 1 : 0) << 30); /* Write Preamble = 1tCK / 2 tCK */
	BS_DDRC_WRITE(port, DRAMTMG9, regval);

	/* configure a DRAMTMG11 reg. */
	regval  = GENMASK(4, 0) & ((data->tCKMPE + 1) / 2);
	regval |= GENMASK(9, 8) & (((data->tMPX_S + 1) / 2) << 8);
	regval |= GENMASK(20, 16) & ((data->tMPX_LH / 2 + 1) << 16);
	regval |= GENMASK(30, 24) & ((((data->tXMPDLL + 31) / 32 + 1) / 2) << 24);
	BS_DDRC_WRITE(port, DRAMTMG11, regval);

	/* configure a DRAMTMG12 reg. */
	regval  = GENMASK(4, 0) & ((data->tMRD_PDA + 1) / 2);
	regval |= GENMASK(29, 24) & (((data->tMOD + data->PL + data->AL + 1) / 2) << 24);
	BS_DDRC_WRITE(port, DRAMTMG12, regval);

	/* program SAR registers */
	ctrl_set_sar(port);

	/* set HIF to DRAM address mapping */
	ctrl_set_addr_map(port, data);

	/* configure a RFSHTMG reg. */
	regval  = GENMASK(9, 0) & ((data->tRFC1 + 1) / 2); /* tRFC1 for Refresh Mode normal(1x)); */
	regval |= GENMASK(27, 16) & ((data->tREFI / 32 / 2) << 16);
	BS_DDRC_WRITE(port, RFSHTMG, regval); /* 7800; 350 */ /* cons. incr. tRFC1? */

	/* program ODT timings */
	/* configure a ODTCFG reg. */
	unsigned int dfi_t_cmd_lat = GENMASK(4, 0) & (dfitmg1_val >> 28);
	int rd_odt_delay = data->CL - data->CWL - (data->rd_preamble_2CK ? 2 : 1) +
			(data->wr_preamble_2CK ? 2 : 1) + dfi_t_cmd_lat;
	if (rd_odt_delay < 0) {
		rd_odt_delay = 0;
	}
	regval  = GENMASK(6, 2) & (rd_odt_delay << 2);
	regval |= GENMASK(11, 8) & ((5 + (data->rd_preamble_2CK ? 2 : 1)) << 8); /* DDR4 BL8: 5 + RD_PREAMBLE */
	regval |= GENMASK(20, 16) & (dfi_t_cmd_lat << 16);
	/* DDR4 BL8: 5 + WR_PREAMBLE + CRC_MODE */
	regval |= GENMASK(27, 24) & ((5 + (data->wr_preamble_2CK ? 2 : 1) + (data->crc_on ? 1 : 0)) << 24);
	BS_DDRC_WRITE(port, ODTCFG, regval); /* cons. using wr_preamble */

	/* configure a ODTMAP reg. */
	BS_DDRC_WRITE(port, ODTMAP, data->odt_map);

	/* Program DQ Map */
	/* configure a DQMAP0 reg. */
	BS_DDRC_WRITE(port, DQMAP0, data->DQ_map[0]); /* cons. changing? */
	/* configure a DQMAP1 reg. */
	BS_DDRC_WRITE(port, DQMAP1, data->DQ_map[1]); /* cons. changing? */
	/* configure a DQMAP2 reg. */
	BS_DDRC_WRITE(port, DQMAP2, data->DQ_map[2]);
	/* configure a DQMAP3 reg. */
	BS_DDRC_WRITE(port, DQMAP3, data->DQ_map[3]);
	/* configure a DQMAP4 reg. */
	BS_DDRC_WRITE(port, DQMAP4, data->DQ_map[4]);
	/* configure a DQMAP5 reg. */
	BS_DDRC_WRITE(port, DQMAP5, !data->DQ_swap_rank);

	/* configure a ZQCTL0 reg. */
	BS_DDRC_WRITE(port, ZQCTL0, 0x01000040);

	if (data->ecc_on) {
		BS_DDRC_WRITE(port, ECCCFG0, 0x4);
	}

	/* set PageClose policy = true, with timer; set QoS LPR storage size */
	regval = BS_DDRC_READ(port, SCHED);
	regval |= (1 << 2);
	regval &= ~GENMASK(12, 8);
	regval |= GENMASK(12, 8) & (DDR_CTRL_QOS_LPR << 8); /* uMCTL2 QoS: LPR storage size */
	BS_DDRC_WRITE(port, SCHED, regval);
	BS_DDRC_WRITE(port, SCHED1, 0x40); /* timer = 64 tCK */

	/* disable auto DFI updates (for PHY training) */
	/* configure a DFIUPD0 reg. */
	BS_DDRC_WRITE(port, DFIUPD0, BS_DDRC_READ(port, DFIUPD0) | 1U << 31);
	/* enables the support for acknowledging PHY-initiated updates (rewrite default register value) */
	BS_DDRC_WRITE(port, DFIUPD2, 1U << 31);

	/* for DRAM initialization by PHY (MARK) */
	/* configure a INIT0 reg. */
	BS_DDRC_WRITE(port, INIT0, BS_DDRC_READ(port, INIT0) | 0x3U << 30); /* skip DRAM initialization */

	/* configure a PWRCTL reg. */
	BS_DDRC_WRITE(port, PWRCTL, 1 << 5);

	return 0;
}

int ctrl_prepare_phy_init(int port)
{
	int ret = 0;
	uint64_t timeout;

	/* disable ddrc auto-refreshes during trainings (uMCTL2 DataBook p.1567 Table6-8, step 4) */
	BS_DDRC_WRITE(port, RFSHCTL3, (1 << 0));

	/* enable Quazi-Dynamic registers programming (uMCTL2 DataBook p.1567 Table6-8, step 5) */
	BS_DDRC_WRITE(port, SWCTL, 0);

	/* set dfi_init_complete_en = 0 (uMCTL2 DataBook p.1567 Table6-8, step 6) */
	BS_DDRC_WRITE(port, DFIMISC, (1 << 6));

	/* disable Quazi-Dynamic registers programming (uMCTL2 DataBook p.1567 Table6-8, step 7) */
	BS_DDRC_WRITE(port, SWCTL, (1 << 0));

	/* wait done (uMCTL2 DataBook p.1567 Table6-8, step 7) */
	for (timeout = timeout_init_us(DDR_CTRL_DONE_TIMEOUT);;) {
		if (BS_DDRC_READ(port, SWSTAT) & 1) {
			break;
		} else if (timeout_elapsed(timeout)) {
			ERROR("%s: failed to prepare PHY init\n", __func__);
			ret = -1;
			break;
		}
	}
	return ret;
}

void ctrl_complete_phy_init(int port, struct ddr_configuration *data)
{
	uint64_t timeout;

	/* from S.Hudchenko (verif/top/tb/bk_top_seq_pkg/bk_top_mm_ddr_seq_pkg/bk_top_ddr_init_seq.svh) */
	/* enable Quazi-Dynamic registers programming (uMCTL2 DataBook p.1567 Table6-8, step 15) */
	BS_DDRC_WRITE(port, SWCTL, 0x0);

	/* issue dfi_init_start signal (for DRAM init by PHY) (uMCTL2 DataBook p.1567 Table6-8, step 16)
	 * WARNING: this is last step of time critical section for RDIMM
	 * (avoid any delay from PHY training complete to dfi_init_start)
	 */
	BS_DDRC_WRITE(port, DFIMISC, (1 << 6) | (1 << 5));

	/* disable Quazi-Dynamic registers programming (uMCTL2 DataBook p.1567 Table6-8, step 17) */
	BS_DDRC_WRITE(port, SWCTL, 0x1);

	/* wait done (uMCTL2 DataBook p.1567 Table6-8, step 17) */
	for (timeout = timeout_init_us(DDR_CTRL_DONE_TIMEOUT);;) {
		if (BS_DDRC_READ(port, SWSTAT) & 1) {
			break;
		} else if (timeout_elapsed(timeout)) {
			ERROR("%s:%d failed to prepare PHY init\n", __func__, __LINE__);
			break;
		}
	}

	/* wait for dfi_init_complete (uMCTL2 DataBook p.1567 Table6-8, step 18) */
	/* note: delay is about 12 usec really (measured by vcs) */
	for (timeout = timeout_init_us(DDR_CTRL_DFI_INIT_TIMEOUT);;) {
		if (BS_DDRC_READ(port, DFISTAT) & 1) {
			break;
		} else if (timeout_elapsed(timeout)) {
			ERROR("%s:%d failed to prepare PHY init (port %d)\n", __func__, __LINE__, port);
			break;
		}
	}

	/* enable Quazi-Dynamic registers programming (uMCTL2 DataBook p.1567 Table6-8, step 19) */
	BS_DDRC_WRITE(port, SWCTL, 0x0);

	/* deassert dfi_init_start signal (for DRAM init by PHY) (uMCTL2 DataBook p.1567 Table6-8, step 20) */
	BS_DDRC_WRITE(port, DFIMISC, (1 << 6));

	/* (uMCTL2 DataBook p.1567 Table6-8, step 21) */
	unsigned int RWSL; /* Read to Write System Latency (as PHY training result) */

	if (data->dimms == 1 && data->ranks == 1) {
		RWSL = data->trn_res.rd2wr_CDD_sr; /* single rank constrain */
	} else {
		RWSL = data->trn_res.rd2wr_CDD_mr; /* multi-ranks constrain */
	}

	unsigned int rd2wr = data->RL + 4 + 1 + (data->wr_preamble_2CK ? 2 : 1) - data->WL + RWSL; /* see umctl2 Databook for "rd2wr" description */

	unsigned int WWSL; /* Write to Write System Latency (as PHY training result) */

	if (data->dimms == 1 && data->ranks == 1) {
		WWSL = 0; /* single rank constrain (see PHY PUB databook "Rank-to-Rank Spacing") */
	} else {
		WWSL = data->trn_res.wr2wr_CDD_mr; /* multi-ranks constrain */
	}

	/* increase wr2rd delay by PHY training CDD_WW: see PUB databook 10.2.1.2.2 */
	unsigned int wr2rd;

	if (data->crc_on && MR5_DM) {
		wr2rd = data->CWL + data->PL + 4 + data->tWTR_L_CRC_DM + WWSL; /* CRC_&_DM on */
	} else {
		wr2rd = data->CWL + data->PL + 4 + data->tWTR_L + WWSL; /* CRC off */
	}

	uint32_t val = BS_DDRC_READ(port, DRAMTMG2);

	val &= ~GENMASK(13, 8);
	val |= GENMASK(13, 8) & (((rd2wr + 1) / 2) << 8);
	val &= ~GENMASK(5, 0);
	val |= GENMASK(5, 0) & ((wr2rd + 1) / 2);
	BS_DDRC_WRITE(port, DRAMTMG2, val);

	/* tphy_wrcsgap=4 (see phy pub databook) */
	unsigned int wrcsgap = 4 + (data->wr_preamble_2CK ? 1 : 0) + data->crc_on + WWSL;

	unsigned int RRSL; /* Read to Read System Latency (as PHY training result) */

	if (data->dimms == 1 && data->ranks == 1) {
		RRSL = 0; /* single rank constrain (see PHY PUB databook "Rank-to-Rank Spacing") */
	} else {
		RRSL = data->trn_res.rd2rd_CDD_mr; /* multi-ranks constrain */
	}

	unsigned int rdcsgap = 2 + (data->rd_preamble_2CK ? 1 : 0) + RRSL; /* tphy_rdcsgap = 2 (see phy pub databook) */
	unsigned int rdcsgap_odt = 5 + (data->rd_preamble_2CK ? 2 : 1) - 2 + RRSL; /* (rd_odt_hold = (5 + RD_PREAMBLE)) - BL / 2 */

	if (rdcsgap_odt > rdcsgap) {
		rdcsgap = rdcsgap_odt;
	}

	val = BS_DDRC_READ(port, RANKCTL);
	val &= ~GENMASK(11, 8);
	val |= GENMASK(11, 8) & (((wrcsgap + 1) / 2) << 8);
	val &= ~GENMASK(7, 4);
	val |= GENMASK(7, 4) & (((rdcsgap + 1) / 2) << 4);
	BS_DDRC_WRITE(port, RANKCTL, val);

	/* update DFITMG1 (DFI timing) */
	unsigned int txdqs_dly = data->trn_res.TxDqsDlyMax;

	val = BS_DDRC_READ(port, DFITMG1);
	/* see PHY PUB DataBook, Table5-3 (p.91) << 16); (t_ctrl_delay + (6 + 4) + TxDqsDly) / 2 */
	val &= ~GENMASK(20, 16);
	val |= GENMASK(20, 16) & (((DDR_PHY_TWRDATA_DELAY + txdqs_dly + 1) / 2 + 1) << 16);
	BS_DDRC_WRITE(port, DFITMG1, val);

	/*  set dfi_init_complete_en to 1 (uMCTL2 DataBook p.1567 Table6-8, step 22) */
	BS_DDRC_WRITE(port, DFIMISC, (1 << 6) | 0x1);

	/* set selfref_sw = 0 (uMCTL2 DataBook p.1567 Table6-8, step 23) */
	BS_DDRC_WRITE(port, PWRCTL, 0x0);

	/* disable Quazi-Dynamic registers programming (uMCTL2 DataBook p.1567 Table6-8, step 24) */
	BS_DDRC_WRITE(port, SWCTL, 0x1);

	/* wait done (uMCTL2 DataBook p.1567 Table6-8, step 24) */
	for (timeout = timeout_init_us(DDR_CTRL_DONE_TIMEOUT);;) {
		if (BS_DDRC_READ(port, SWSTAT) & 1) {
			break;
		} else if (timeout_elapsed(timeout)) {
			ERROR("%s:%d failed to prepare PHY init\n", __func__, __LINE__);
			break;
		}
	}

	/* enable AXI port */
	BS_DDRC_WRITE(port, PCTRL_0, 0x1);

	/* wait for uMCTL2 normal mode (uMCTL2 DataBook p.1567 Table6-8, step 25) */
	for (timeout = timeout_init_us(DDR_CTRL_DONE_TIMEOUT);;) {
		if ((BS_DDRC_READ(port, STAT) & 0x7) == 0x1) {
			break;
		} else if (timeout_elapsed(timeout)) {
			ERROR("%s:%d failed to prepare PHY init\n", __func__, __LINE__);
			break;
		}
	}

	/* enable ddrc auto-refreshes after trainings (uMCTL2 DataBook p.1567 Table6-8, step 26) */
	BS_DDRC_WRITE(port, RFSHCTL3, 0x0);
}

int ddr_init_ecc_memory(int port)
{
	uint32_t ret;
	uint64_t timeout;
	/* uMCTL2 DataBook, p.426 "Initialization Writes" */
	/* 2. block the AXI ports from taking the transaction */
	BS_DDRC_WRITE(port, PCTRL_0, 0x0);
	/* disable scrubber */
	uint32_t sbrctl = BS_DDRC_READ(port, SBRCTL);
	/* 4. Set SBRCTL.scrub_mode = 1 (ECC scrubber will perform writes) */
	/* 5. Set SBRCTL.scrub_interval = 0 (back-to-back reading) */
	sbrctl |= (1 << 2); /* ecc write mode */
	sbrctl &= ~GENMASK(20, 8);
	BS_DDRC_WRITE(port, SBRCTL, sbrctl);

	/* 7. Enable the SBR by programming SBRCTL.scrub_en = 1. */
	/* enable scrub in a separate step to avoid race conditions */
	sbrctl |= (1 << 0);
	BS_DDRC_WRITE(port, SBRCTL, sbrctl);

	/* uMCTL2 DataBook, p.426 "Initialization Writes" */
	/* 8. wait for all scrub writes commands have been send */
	for (timeout = timeout_init_us(50000000);;) {
		if (BS_DDRC_READ(port, SBRSTAT) & 0x2) {
			break;
		} else if (timeout_elapsed(timeout)) {
			ERROR("%s: failed to init memory (undone)\n", __func__);
			ret = -1;
			goto exit;
		}
	}
	for (timeout = timeout_init_us(10000);;) {
		if (!(BS_DDRC_READ(port, SBRSTAT) & 0x1)) {
			break;
		} else if (timeout_elapsed(timeout)) {
			ERROR("%s: failed to init memory (busy)\n", __func__);
			ret = -1;
			goto exit;
		}
	}

exit:
	/* 10. disable the scrubber */
	sbrctl &= ~(1 << 0);
	BS_DDRC_WRITE(port, SBRCTL, sbrctl);

	/* 14. enable the AXI ports */
	BS_DDRC_WRITE(port, PCTRL_0, 0x1);

	return ret;
}
