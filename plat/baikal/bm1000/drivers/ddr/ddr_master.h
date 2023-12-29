/*
 * Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DDR_MASTER_H
#define DDR_MASTER_H

#include <stdint.h>

#include "ddr_main.h"
#include "ddr_spd.h"

#define DDR_CRC_ENABLE 0

#define CLOCK_PS(x)	\
	(uint32_t)((((uint64_t)(x) * 1000) / data->tCK + 974) / 1000)
#define CLOCK_NS(x)	CLOCK_PS((uint64_t)(x) * 1000)

#define SPD_TO_PS(mtb, ftb)	((mtb) * 125 + (ftb))

int ddr_config_by_spd(int port, struct ddr_configuration *data);

/* BL=8; RD sequential; DLL reset off; set CAS Latency; set Write Recovery; set Read To Precharge */
static inline uint16_t set_mr0(const uint32_t cl, uint32_t wr, const uint32_t rtp)
{
	const uint8_t cl_table[] = {0xd, 0x8, 0xe, 0x9, 0xf, 0xa, 0xc, 0xb};
	uint16_t mr0_reg = 0;
	uint16_t tmp = 0;

	/* The WR value in the mode register must be programmed to be equal or larger than WRmin */
	wr = (wr + 1) & ~0x1U;

	if (rtp <= 10) {
		tmp = rtp - 5;
	} else if (rtp == 11) {
		tmp = 0x7;
	} else if (rtp == 12) {
		tmp = 0x6;
	} else {
		tmp = 0x8;
	}

	mr0_reg |= ((tmp & 0x7) << 9) |
		   ((tmp & 0x8) << 10);

	if (cl <= 16) {
		tmp = cl - 9;
	} else if (cl <= 24) {
		tmp = cl_table[cl - 17];
	} else if (cl <= 32) {
		tmp = cl - 9;
	} else {
		tmp = 0x17;
	}

	mr0_reg |= ((tmp & 0x1) << 2) |
		   ((tmp & 0xe) << 3) |
		   ((tmp & 0x10) << 8);

	return mr0_reg;
}

/*
 * CRC on; ODTwr off; LP ASR manual, normal temperature; set CAS Write Latency
 * Attention: Write Leveling Training require to ODTwr = off
 */
static inline uint16_t set_mr2(const uint32_t cwl, const uint32_t rtt_wr)
{
	uint16_t tmp = (DDR_CRC_ENABLE & 0x1) << 12;

	if (cwl <= 12) {
		tmp |= ((0xff & cwl) - 9) << 3;
	} else {
		tmp |= ((0xff & cwl) / 2 - 3) << 3;
	}

	return tmp | ((rtt_wr & 0x7) << 9);
}

/*
 * Read DBI off; Write DBI off; DM on; CA Parity Persistent Error off; RTT Park off;
 * ODT input buffer PD on; CA Parity Error clear; CRC Error clear; set CA Parity Latency Mode
 */
static inline uint16_t set_mr5(const uint32_t pl, const uint32_t rtt_park)
{
	uint16_t tmp = 1 << 10; /* DM on */

	if (pl) {
		tmp |= pl - 3;
	}

	return tmp | (rtt_park << 6);
}

#endif /* DDR_MASTER_H */
