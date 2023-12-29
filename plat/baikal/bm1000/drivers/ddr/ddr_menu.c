/*
 * Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <baikal_bootflash.h>
#include "ddr_main.h"
#include "ddr_menu.h"

extern int ddr_flash_init_flag;
extern struct ddr_flash_config ddr_storage;

int ddr_flash_conf_load(int port)
{
	int err = bootflash_read((BAIKAL_UEFI_DDR_FLAG_OFFSET + \
				(BAIKAL_STR_PORT_OFFS * port)),
				&ddr_storage, sizeof(struct ddr_flash_config));
	if (err) {
		ERROR("Failed to read flash for DDR frequency setup\n");
		return -1;
	}
	if ((ddr_storage.speedbin == 0x0) && (ddr_storage.ddr_sign == 0x0)) {
		ddr_flash_init_flag = 0xf;
	}
	return 0;
}

int ddr_flash_conf_store(int port, struct ddr_configuration *data)
{
	int err = bootflash_erase(BAIKAL_UEFI_DDR_FLAG_OFFSET + \
			(BAIKAL_STR_PORT_OFFS * port), BAIKAL_BOOT_SPI_SUBSECTOR);
	if (err) {
		ERROR("Failed to erase flash\n");
		return -2;
	}
	switch (data->clock_mhz) {
	case 1200:
		ddr_storage.speedbin = FLASH_SPEEDBIN_2400;
		break;
	case 1066:
		ddr_storage.speedbin = FLASH_SPEEDBIN_2133;
		break;
	case 933:
		ddr_storage.speedbin = FLASH_SPEEDBIN_1866;
		break;
	case 800:
		ddr_storage.speedbin = FLASH_SPEEDBIN_1600;
		break;
	default:
		/* Improbable situation; */
		break;
	}

	switch (data->DIC) {
	case 0:
		ddr_storage.flsh_dic = BAIKAL_DIC_RZQ_DIV_7;
		break;
	case 1:
		ddr_storage.flsh_dic = BAIKAL_DIC_RZQ_DIV_5;
		break;
	default:
		/* Improbable situation; */
		break;
	}
	switch (data->RTT_WR) {
	case 0:
		ddr_storage.flsh_rtt_wr = BAIKAL_RTTWR_DYN_OFF;
		break;
	case 1:
		ddr_storage.flsh_rtt_wr = BAIKAL_RTTWR_RZQ_DIV_4;
		break;
	case 2:
		ddr_storage.flsh_rtt_wr = BAIKAL_RTTWR_RZQ_DIV_2;
		break;
	case 3:
		ddr_storage.flsh_rtt_wr = BAIKAL_RTTWR_HI_Z;
		break;
	case 4:
		ddr_storage.flsh_rtt_wr = BAIKAL_RTTWR_RZQ_DIV_3;
		break;
	default:
		/* Improbable situation; */
		break;
	}
	switch (data->RTT_NOM) {
	case 0:
		ddr_storage.flsh_rtt_nom = BAIKAL_RTTNOM_RZQ_DIS;
		break;
	case 1:
		ddr_storage.flsh_rtt_nom = BAIKAL_RTTNOM_RZQ_DIV_4;
		break;
	case 2:
		ddr_storage.flsh_rtt_nom = BAIKAL_RTTNOM_RZQ_DIV_2;
		break;
	case 3:
		ddr_storage.flsh_rtt_nom = BAIKAL_RTTNOM_RZQ_DIV_6;
		break;
	case 4:
		ddr_storage.flsh_rtt_nom = BAIKAL_RTTNOM_RZQ_DIV_1;
		break;
	case 5:
		ddr_storage.flsh_rtt_nom = BAIKAL_RTTNOM_RZQ_DIV_5;
		break;
	case 6:
		ddr_storage.flsh_rtt_nom = BAIKAL_RTTNOM_RZQ_DIV_3;
		break;
	case 7:
		ddr_storage.flsh_rtt_nom = BAIKAL_RTTNOM_RZQ_DIV_7;
		break;
	default:
		/* Improbable situation; */
		break;
	}
	switch (data->RTT_PARK) {
	case 0:
		ddr_storage.flsh_rtt_park = BAIKAL_RTTPARK_RZQ_DIS;
		break;
	case 1:
		ddr_storage.flsh_rtt_park = BAIKAL_RTTPARK_RZQ_DIV_4;
		break;
	case 2:
		ddr_storage.flsh_rtt_park = BAIKAL_RTTPARK_RZQ_DIV_2;
		break;
	case 3:
		ddr_storage.flsh_rtt_park = BAIKAL_RTTPARK_RZQ_DIV_6;
		break;
	case 4:
		ddr_storage.flsh_rtt_park = BAIKAL_RTTPARK_RZQ_DIV_1;
		break;
	case 5:
		ddr_storage.flsh_rtt_park = BAIKAL_RTTPARK_RZQ_DIV_5;
		break;
	case 6:
		ddr_storage.flsh_rtt_park = BAIKAL_RTTPARK_RZQ_DIV_3;
		break;
	case 7:
		ddr_storage.flsh_rtt_park = BAIKAL_RTTPARK_RZQ_DIV_7;
		break;
	default:
		/* Improbable situation; */
		break;
	}

	ddr_storage.flsh_cl = data->CL;
	ddr_storage.flsh_trcd = data->tRCD;
	ddr_storage.flsh_trp = data->tRP;
	ddr_storage.flsh_tras = data->tRAS;
	ddr_storage.flsh_tfaw = data->tFAW;

	/* Default/Reset value stored in DXnGCR5 registers is 9 */
	ddr_storage.flsh_host_v = data->PHY_HOST_VREF ? data->PHY_HOST_VREF : 9;
	ddr_storage.flsh_dram_v = data->PHY_DRAM_VREF;
	ddr_storage.flsh_vref_use = false;

	if (data->dimms == 2) {
		ddr_storage.flsh_1t2t = true;
		ddr_storage.flsh_host_v = data->PHY_HOST_VREF;
		ddr_storage.flsh_dram_v = data->PHY_DRAM_VREF;
		ddr_storage.flsh_vref_use = false;
	}

	ddr_storage.ddr_sign = BAIKAL_FLASH_USE_SPD;
	err = bootflash_write(BAIKAL_UEFI_DDR_FLAG_OFFSET + \
			(BAIKAL_STR_PORT_OFFS * port),
			&ddr_storage, sizeof(struct ddr_flash_config));

	ddr_flash_init_flag = 0x0;

	if (err) {
		ERROR("Failed to write default DDR frequency structure\n");
		return -1;
	}
	return 0;
}
