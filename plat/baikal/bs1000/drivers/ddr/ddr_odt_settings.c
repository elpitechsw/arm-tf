/*
 * Copyright (c) 2022, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include "ddr_main.h"

enum {
	ODT_SET_1RANK_SDIMM = 0,
	ODT_SET_2RANK_SDIMM,
	ODT_SET_1RANK_DDIMM,
	ODT_SET_2RANK_DDIMM,
};

struct odt_settings {
	uint32_t dic;
	uint32_t rtt_wr;
	uint32_t rtt_nom;
	uint32_t rtt_park;
	uint32_t host_vref;
	uint32_t dram_vref;
	uint32_t phy_odt;
	uint32_t phy_odi;
	uint32_t odt_map;
};

static const struct odt_settings dram_odt_set[] = {
#ifndef ELPITECH // BE settings
	{ 0, 4, 0, 0, 82,  14, 80, 34, 0x0201 },
	{ 0, 0, 5, 0, 84,  26, 80, 34, 0x0102 },
	{ 0, 1, 3, 7, 100, 28, 80, 34, 0x00010004 },
	{ 0, 2, 7, 4, 102, 30, 80, 34, 0x22228888 }
#else // ELP
	{ 0, 1, 0, 0, 82,  14, 80, 34, 0x0 },
	{ 0, 1, 0, 0, 84,  12, 80, 34, 0x0 },
	{ 0, 1, 3, 0, 96,  28, 80, 34, 0x00110044 },
	{ 0, 1, 3, 0, 97,  26, 80, 34, 0x22228888 }
#endif
};

int ddr_odt_configuration(const unsigned int port,
			  struct ddr_configuration *const data)
{
	unsigned int odt_set;

	if (data->dimms == 1) {
		if (data->ranks == 1) {
			odt_set = ODT_SET_1RANK_SDIMM;
		} else {
			odt_set = ODT_SET_2RANK_SDIMM;
		}
	} else {
		if (data->ranks == 1) {
			odt_set = ODT_SET_1RANK_DDIMM;
		} else {
			odt_set = ODT_SET_2RANK_DDIMM;
		}
	}

	data->RTT_PARK	= dram_odt_set[odt_set].rtt_park;
	data->RTT_NOM	= dram_odt_set[odt_set].rtt_nom;
	data->RTT_WR	= dram_odt_set[odt_set].rtt_wr;
	data->DIC	= dram_odt_set[odt_set].dic;
	data->HOST_VREF	= dram_odt_set[odt_set].host_vref;
	data->DRAM_VREF	= dram_odt_set[odt_set].dram_vref;
	data->PHY_ODT	= dram_odt_set[odt_set].phy_odt;
	data->PHY_ODI	= dram_odt_set[odt_set].phy_odi;
	data->odt_map	= dram_odt_set[odt_set].odt_map;

	return 0;
}
