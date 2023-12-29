/*
 * Copyright (c) 2021-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <baikal_def.h>
#include "ddr_main.h"
#include "ddr_menu.h"

enum {
	ODT_SET_1RANK = 0,
	ODT_SET_2RANK,
	ODT_SET_RDIMM_1RANK,
	ODT_SET_RDIMM_2RANK,
	ODT_SET_MTA9ASF1G72,
	ODT_SET_MTA18ADF2G72,
	ODT_SET_KVR24N17S8,
	ODT_SET_FL2133D4U15D_8G,
	ODT_SET_DP2400D4U17S_4S01,
	ODT_SET_TSGLH72V6E2,
	ODT_SET_KVR24N17S6,
	ODT_SET_KVR26N19S6
};

static const uint16_t dram_crc_val[] = {
	0x0000,	/* 1-rank */
	0x0000,	/* 2-rank */
	0x0000,	/* RDIMM 1-rank */
	0x0000,	/* RDIMM 2-rank */
	0x4688,	/* MTA9ASF1G72 */
	0xb1d3,	/* MTA18ADF2G72 */
	0xbd97,	/* KVR24N17S8/8 */
	0x399d,	/* FL2133D4U15D_8G */
	0xfc67,	/* DP2400D4U17S_4S01 */
	0x9d5d,	/* TSGLH72V6E2 */
	0x7fca, /* KVR24N17S6/4 */
	0xb246  /* KVR26N19S6/4 */
};

struct odt_settings {
	uint32_t dic;
	uint32_t rtt_wr;
	uint32_t rtt_nom;
	uint32_t rtt_park;
};

#if defined(BAIKAL_DBM10) || defined(BAIKAL_DBM20) || defined(BAIKAL_QEMU)
static const struct odt_settings dram_odt_set_0[] = {
	{ 0, 1, 2, 4 }, /* 1-rank (default) */
	{ 0, 1, 2, 4 }, /* 2-rank (default) */
	{ 0, 1, 2, 4 }, /* RDIMM 1-rank */
	{ 0, 1, 2, 4 }, /* RDIMM 2-rank */
	{ 0, 1, 2, 4 }, /* 1-rank (MTA9ASF1G72) */
	{ 0, 1, 2, 4 }, /* 2-rank (MTA18ADF2G72) */
	{ 0, 0, 6, 0 }, /* KVR24N17S8/8 */
	{ 2, 2, 4, 2 }, /* FL2133D4U15D_8G */
	{ 0, 1, 4, 2 }, /* DP2400D4U17S_4S01 */
	{ 0, 2, 5, 2 }, /* TSGLH72V6E2 */
	{ 0, 1, 2, 4 }, /* KVR24N17S6/4 */
	{ 0, 1, 2, 4 }  /* KVR26N19S6/4 */
};

static const struct odt_settings dram_odt_set_1[] = {
	{ 0, 1, 2, 4 }, /* 1-rank (default) */
	{ 0, 1, 2, 4 }, /* 2-rank (default) */
	{ 0, 1, 2, 4 }, /* RDIMM 1-rank */
	{ 0, 1, 2, 4 }, /* RDIMM 2-rank */
	{ 0, 1, 2, 4 }, /* 1-rank (MTA9ASF1G72) */
	{ 0, 1, 2, 4 }, /* 2-rank (MTA18ADF2G72) */
	{ 0, 0, 6, 0 }, /* KVR24N17S8/8 */
	{ 2, 2, 4, 2 }, /* FL2133D4U15D_8G */
	{ 0, 1, 4, 2 }, /* DP2400D4U17S_4S01 */
	{ 0, 2, 5, 2 }, /* TSGLH72V6E2 */
	{ 0, 1, 2, 4 }, /* KVR24N17S6/4 */
	{ 0, 1, 2, 4 }  /* KVR26N19S6/4 */
};
#elif defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
static const struct odt_settings dram_odt_set_0[] = {
	{ 0, 4, 0, 0 }, /* 1-rank (default) 34,240  34, 80,DIS,DIS */
	{ 0, 2, 0, 2 }, /* 2-rank (default) 30,240  34,120,DIS,120 */
	{ 0, 1, 2, 2 }, /* RDIMM 1-rank     30,240  34,120,120,120 (training by MTA9ADF1G72PZ-3G2) */
	{ 0, 1, 4, 4 }, /* RDIMM 2-rank     30,240  34,120,240,240 (training by MTA18ASF2G72PDZ-2G3) */
	{ 0, 4, 0, 0 }, /* 1-rank (MTA9ASF1G72) */
	{ 0, 1, 0, 2 }, /* 2-rank (MTA18ADF2G72) */
	{ 0, 0, 6, 0 }, /* KVR24N17S8/8 */
	{ 2, 2, 4, 2 }, /* FL2133D4U15D_8G */
	{ 0, 1, 4, 2 }, /* DP2400D4U17S_4S01 */
	{ 0, 2, 0, 2 }, /* TSGLH72V6E2 */
	{ 0, 4, 6, 7 }, /* KVR24N17S6/4 */
	{ 0, 4, 3, 6 }  /* KVR26N19S6/4 */
};

static const struct odt_settings dram_odt_set_1[] = {
	{ 0, 4, 0, 0 }, /* 1-rank (default) 30,120  34, 80,DIS,DIS */
	{ 0, 2, 0, 2 }, /* 2-rank (default) 30, 80  34,240,DIS,120 */
	{ 0, 4, 0, 0 }, /* RDIMM 1-rank     30, 80  34, 80,DIS,DIS (training by MTA9ADF1G72PZ-3G2) */
	{ 0, 1, 2, 4 }, /* RDIMM 2-rank     30,120  34,120,120,240 (training by MTA18ASF2G72PDZ-2G3) */
	{ 0, 4, 0, 0 }, /* 1-rank (MTA9ASF1G72) */
	{ 0, 2, 0, 2 }, /* 2-rank (MTA18ADF2G72) */
	{ 0, 0, 6, 0 }, /* KVR24N17S8/8 */
	{ 2, 2, 4, 2 }, /* FL2133D4U15D_8G */
	{ 0, 1, 4, 2 }, /* DP2400D4U17S_4S01 */
	{ 0, 2, 0, 2 }, /* TSGLH72V6E2 */
	{ 0, 4, 0, 0 }, /* KVR24N17S6/4 */
	{ 0, 4, 0, 0 }  /* KVR26N19S6/4 */
};
#endif

#ifdef BAIKAL_DDR_SETTINGS_IN_FLASH
#include <common/debug.h>

extern struct ddr_flash_config ddr_storage;

static void ddr_odt_flash_values(struct ddr_configuration *const data)
{
	int err_flag = 0;
	if (ddr_storage.ddr_sign == BAIKAL_FLASH_USE_STR) {
		if (ddr_storage.flsh_dic) {
			switch (ddr_storage.flsh_dic) {
			case BAIKAL_DIC_RZQ_DIV_7:
				data->DIC = 0;
				break;
			case BAIKAL_DIC_RZQ_DIV_5:
				data->DIC = 1;
				break;
			default:
				err_flag = 0xf0f0;
				break;
			}
		}
		if (ddr_storage.flsh_rtt_wr) {
			switch (ddr_storage.flsh_rtt_wr) {
			case BAIKAL_RTTWR_DYN_OFF:
				data->RTT_WR = 0;
				break;
			case BAIKAL_RTTWR_RZQ_DIV_4:
				data->RTT_WR = 1;
				break;
			case BAIKAL_RTTWR_RZQ_DIV_2:
				data->RTT_WR = 2;
				break;
			case BAIKAL_RTTWR_HI_Z:
				data->RTT_WR = 3;
				break;
			case BAIKAL_RTTWR_RZQ_DIV_3:
				data->RTT_WR = 4;
				break;
			default:
				err_flag = 0xf0f0;
				break;
			}
		}
		if (ddr_storage.flsh_rtt_nom) {
			switch (ddr_storage.flsh_rtt_nom) {
			case BAIKAL_RTTNOM_RZQ_DIS:
				data->RTT_NOM = 0;
				break;
			case BAIKAL_RTTNOM_RZQ_DIV_4:
				data->RTT_NOM = 1;
				break;
			case BAIKAL_RTTNOM_RZQ_DIV_2:
				data->RTT_NOM = 2;
				break;
			case BAIKAL_RTTNOM_RZQ_DIV_6:
				data->RTT_NOM = 3;
				break;
			case BAIKAL_RTTNOM_RZQ_DIV_1:
				data->RTT_NOM = 4;
				break;
			case BAIKAL_RTTNOM_RZQ_DIV_5:
				data->RTT_NOM = 5;
				break;
			case BAIKAL_RTTNOM_RZQ_DIV_3:
				data->RTT_NOM = 6;
				break;
			case BAIKAL_RTTNOM_RZQ_DIV_7:
				data->RTT_NOM = 7;
				break;
			default:
				err_flag = 0xf0f0;
				break;
			}
		}
		if (ddr_storage.flsh_rtt_park) {
			switch (ddr_storage.flsh_rtt_park) {
			case BAIKAL_RTTPARK_RZQ_DIS:
				data->RTT_PARK = 0;
				break;
			case BAIKAL_RTTPARK_RZQ_DIV_4:
				data->RTT_PARK = 1;
				break;
			case BAIKAL_RTTPARK_RZQ_DIV_2:
				data->RTT_PARK = 2;
				break;
			case BAIKAL_RTTPARK_RZQ_DIV_6:
				data->RTT_PARK = 3;
				break;
			case BAIKAL_RTTPARK_RZQ_DIV_1:
				data->RTT_PARK = 4;
				break;
			case BAIKAL_RTTPARK_RZQ_DIV_5:
				data->RTT_PARK = 5;
				break;
			case BAIKAL_RTTPARK_RZQ_DIV_3:
				data->RTT_PARK = 6;
				break;
			case BAIKAL_RTTPARK_RZQ_DIV_7:
				data->RTT_PARK = 7;
				break;
			default:
				err_flag = 0xf0f0;
				break;
			}
		}
		if (ddr_storage.flsh_vref_use) {
			data->PHY_HOST_VREF = ddr_storage.flsh_host_v;
			data->PHY_DRAM_VREF = ddr_storage.flsh_dram_v;
		}
	}
	if (err_flag) {
		ERROR("Failed to configure some of the ODT values from flash\n");
	}
}
#endif

int ddr_odt_configuration(const unsigned int port,
			   const uint16_t crc_val,
			   struct ddr_configuration *const data)
{
	unsigned int odt_set;
#ifdef BAIKAL_DUAL_CHANNEL_MODE
	if (data->dimms == 2) {
		goto dual_channel;
	}
#endif
	if (crc_val == dram_crc_val[ODT_SET_MTA9ASF1G72]) {
		odt_set = ODT_SET_MTA9ASF1G72;
	} else if (crc_val == dram_crc_val[ODT_SET_KVR26N19S6]) {
		odt_set = ODT_SET_KVR26N19S6;
	} else if (crc_val == dram_crc_val[ODT_SET_KVR24N17S6]) {
		odt_set = ODT_SET_KVR24N17S6;
	} else if (crc_val == dram_crc_val[ODT_SET_MTA18ADF2G72]) {
		odt_set = ODT_SET_MTA18ADF2G72;
	} else if (crc_val == dram_crc_val[ODT_SET_KVR24N17S8]) {
		odt_set = ODT_SET_KVR24N17S8;
	} else if (crc_val == dram_crc_val[ODT_SET_FL2133D4U15D_8G]) {
		odt_set = ODT_SET_FL2133D4U15D_8G;
	} else if (crc_val == dram_crc_val[ODT_SET_DP2400D4U17S_4S01]) {
		odt_set = ODT_SET_DP2400D4U17S_4S01;
	} else if (crc_val == dram_crc_val[ODT_SET_TSGLH72V6E2]) {
		odt_set = ODT_SET_TSGLH72V6E2;
	} else if (data->registered_dimm && data->ranks == 2) {
		odt_set = ODT_SET_RDIMM_2RANK;
	} else if (data->registered_dimm && data->ranks == 1) {
		odt_set = ODT_SET_RDIMM_1RANK;
	} else if (data->ranks == 2) {
		odt_set = ODT_SET_2RANK;
	} else {
		odt_set = ODT_SET_1RANK;
	}

	if (port == 0) {
		data->RTT_PARK	= dram_odt_set_0[odt_set].rtt_park;
		data->RTT_NOM	= dram_odt_set_0[odt_set].rtt_nom;
		data->RTT_WR	= dram_odt_set_0[odt_set].rtt_wr;
		data->DIC	= dram_odt_set_0[odt_set].dic;
	} else {
		data->RTT_PARK	= dram_odt_set_1[odt_set].rtt_park;
		data->RTT_NOM	= dram_odt_set_1[odt_set].rtt_nom;
		data->RTT_WR	= dram_odt_set_1[odt_set].rtt_wr;
		data->DIC	= dram_odt_set_1[odt_set].dic;
	}

#ifdef BAIKAL_DDR_SETTINGS_IN_FLASH
	ddr_odt_flash_values(data);
#endif
	return 0;
#ifdef BAIKAL_DUAL_CHANNEL_MODE
dual_channel:
	/***********************************************************************
	 * Due to high speed of the DDR4 Standard configuring controllers to
	 * operate properly in dual-channel mode can be quite challenging at
	 * times. It sets high expectations on quality of your board electronic
	 * schematic and, usually, requires extra work to figure out certain
	 * settings in order for the controller and DIMM to function well
	 * together like in the case of DBMv2 board.
	 * It appeared that changing default values of Host's Phy and DRAM's
	 * voltages may have positive influence on the system's ability to
	 * operate in dual-channel mode.
	 * Below you may see an example of settings that happened to work well
	 * for DBMv2 board.
	 * Each board and each pair of DIMMs may require its own peculiar
	 * settings. So take your time to figure out what will work best with
	 * your particular hardware.
	 **********************************************************************/
#if defined(BAIKAL_DBM20)
	if (port == 0) {
		switch (crc_val) {
		case 0x1859: /* CT4G4DFS824A */
		case 0x7fca: /* M378A5244CBO */
			data->RTT_PARK = 7; /* RZQ/7 */
			data->RTT_NOM = 4; /* RZQ */
			data->RTT_WR = 3; /* Hi-Z */
			data->DIC = 0; /* RZQ/7 */
			data->PHY_HOST_VREF = 25; /* 61,2% */
			data->PHY_DRAM_VREF = 30; /* 79,5% */
			return 0;
		case 0xd8f1: /* MTA9ASF2G72AZ */
			data->RTT_PARK = 3; /* RZQ/6 */
			data->RTT_NOM = 2; /* RZQ/2 */
			data->RTT_WR = 4; /* RZQ/3 */
			data->DIC = 0; /* RZQ/7 */
			data->PHY_HOST_VREF = 26; /* 62.2% */
			data->PHY_DRAM_VREF = 30; /* 79,5% */
			return 0;
		default:
			break;
		}
	}

	if (port == 1) {
		switch (crc_val) {
		case 0xd8f1: /* MTA9ASF2G72AZ */
			data->RTT_PARK = 3; /* RZQ/6 */
			data->RTT_NOM = 2; /* RZQ/2 */
			data->RTT_WR = 4; /* RZQ/3 */
			data->DIC = 0; /* RZQ/7 */
			data->PHY_HOST_VREF = 26; /* 62.2% */
			data->PHY_DRAM_VREF = 30; /* 79,5% */
			return 0;
		default:
			break;
		}
	}
#endif
	return -1;
#endif /* defined(BAIKAL_DUAL_CHANNEL_MODE) */
}
