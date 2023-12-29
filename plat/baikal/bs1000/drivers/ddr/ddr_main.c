/*
 * Copyright (c) 2022-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <bs1000_dimm_spd.h>
#include <spd.h>

#include "ddr_bs1000.h"
#include "ddr_spd.h"
#include "ddr_ctrl.h"
#include "ddr_main.h"
#include "ddr_misc.h"
#include "ddr_master.h"
#include "phy/ddr_phy_main.h"

#include <baikal_bootflash.h>
#include <platform_def.h>

#define BAIKAL_TFW_RDIMM_SIZE	55036
#define BAIKAL_TFW_UDIMM_SIZE	53468
#define BAIKAL_TFW_RDIMM_OFFS	BAIKAL_TFW_UDIMM_SIZE

int ddr_init_ecc_memory(int port);
int ddr_odt_configuration(const unsigned int port,
			  struct ddr_configuration *const data);

uint8_t fw_container[55036];

static uint64_t ddr_get_addr_strip_bit(unsigned int channels, int capacity_gb)
{
	char bytes[3];
	uint64_t mask;

	if (channels == 1) {
		/* set address bit strip mask for CMN to 1 SN-F */
		bytes[0] = 0;
		bytes[1] = 0;
		bytes[2] = 0;
	} else if (channels == 2) {
		/* set address bit strip mask for CMN to 2 SN-F */
		bytes[0] = 9;
		bytes[1] = 0;
		bytes[2] = 0;
	} else if (channels == 4) {
		/* set address bit strip mask for CMN to 4 SN-F */
		bytes[0] = 8;
		bytes[1] = 9;
		bytes[2] = 0;
	} else if (channels == 6) {
		switch (capacity_gb) {
		case 1:
		/*   1 GiB DRAM size per SN-F addressing */
			bytes[0] = 28;
			bytes[1] = 31;
			bytes[2] = 35;
			break;
		case 2:
		/*   2 GiB DRAM size per SN-F addressing */
			bytes[0] = 28;
			bytes[1] = 32;
			bytes[2] = 33;
			break;
		case 4:
		/*   4 GiB DRAM size per SN-F addressing */
			bytes[0] = 28;
			bytes[1] = 33;
			bytes[2] = 34;
			break;
		case 8:
		/*   8 GiB DRAM size per SN-F addressing */
			bytes[0] = 33;
			bytes[1] = 34;
			bytes[2] = 39;
			break;
		case 16:
		/*  16 GiB DRAM size per SN-F addressing */
			bytes[0] = 28;
			bytes[1] = 36;
			bytes[2] = 39;
			break;
		case 32:
		/*  32 GiB DRAM size per SN-F addressing */
			bytes[0] = 28;
			bytes[1] = 36;
			bytes[2] = 37;
			break;
		case 64:
		/*  64 GiB DRAM size per SN-F addressing */
			bytes[0] = 28;
			bytes[1] = 37;
			bytes[2] = 38;
			break;
		case 128:
		/* 128 GiB DRAM size per SN-F addressing */
			bytes[0] = 37;
			bytes[1] = 38;
			bytes[2] = 43;
			break;
		default:
			ERROR("Can't configure CMN - incorrect DRAM size = %d GiB\n", capacity_gb);
			return -1;
		}
	} else {
		ERROR("CMN can't operate with a such DRAM configuration\n");
		return -1;
	}

	mask = (1ULL << bytes[2]) | (1ULL << bytes[1]) | (1ULL << bytes[0]);
	mask &= ~1; /* don't strip bit 0 (bit 0 = none) */
	return mask;
	return 0;
}

static int ddr_port_init(unsigned int port, struct ddr4_spd_eeprom *spd,
			 unsigned int channels, bool dual_channel_mode)
{
	int err = 0;
	static int fw_read_flag; /* {2,1} if fw for {r,u}dimm is in place */
	struct ddr_configuration data = {0};
	int capacity_gb = spd_get_baseconf_dimm_capacity(spd) / 1024 / 1024 / 1024;

	if (ddr_config_by_spd(port, spd, &data)) {
		goto error;
	}

	/* disable CA PARITY */
	data.par_on = 0;
	/* disable DRAM CRC */
	data.crc_on = 0;
	/* disable DRAM PHY Equalization */
	data.phy_eql = 0;

	if (dual_channel_mode) {
		data.dimms = 2;
	}

	data.addr_strip_bit = ddr_get_addr_strip_bit(channels,
				capacity_gb * (dual_channel_mode ? 2 : 1));
	if ((int64_t)data.addr_strip_bit < 0) {
		goto error;
	}

	if (ddr_odt_configuration(port, &data)) {
		goto error;
	}

	/* enable PHY 2D training */
	if (data.clock_mhz >= 1200) {
		data.phy_training_2d = 1;
	}

	if (data.clock_mhz != 800) {
		if (ddr_lcpcmd_set_speedbin(port, data.clock_mhz)) {
			goto error;
		}
	}

	ddrlcru_apb_reset_off(port);

	if (ctrl_init(port, &data)) {
		goto error;
	}

	ddrlcru_core_reset_off(port);

	ctrl_prepare_phy_init(port);

	if (data.registered_dimm) {
		if (fw_read_flag != 2) {
			err = bootflash_read(BAIKAL_TFW_OFFSET + BAIKAL_TFW_RDIMM_OFFS,
					     fw_container, BAIKAL_TFW_RDIMM_SIZE);
			fw_read_flag = 2;
		}
	} else {
		if (fw_read_flag != 1) {
			err = bootflash_read(BAIKAL_TFW_OFFSET,
					     fw_container, BAIKAL_TFW_UDIMM_SIZE);
			fw_read_flag = 1;
		}
	}

	if (err) {
		ERROR("Failed to read training FW from bootflash\n");
		goto error;
	}

	phy_main(port, &data);

	ctrl_complete_phy_init(port, &data);

	if (data.registered_dimm) {
		/* this is experimental workaround code
		 * for some RDIMMs (like MTA18ASF2G72PZ-3G2, MTA36ASF4G72PZ-3G2)
		 */
		umctl2_enter_SR(port); /* enter DRAM self-refresh mode */
		udelay(10);
		umctl2_exit_SR(port); /* exit DRAM self-refresh mode */
	}

	if (data.ecc_on) {
		ddr_init_ecc_memory(port);
	}

	INFO("DIMM%u: module rate %u MHz, AA-RCD-RP-RAS %u-%u-%u-%u\n", port,
	     data.clock_mhz * 2, data.CL, data.tRCD, data.tRP, data.tRAS);

	return 0;

error:
	ERROR("Failed to init DDR port #%u\n", port);
	return -1;
}

int dram_init(void)
{
	int ret = 0;
	uint64_t conf = 0;
	unsigned int channels[PLATFORM_CHIP_COUNT];
	struct ddr4_spd_eeprom *spd_content;
	unsigned int chip_idx, port_idx, slot_idx;

	for (chip_idx = 0, slot_idx = 0; chip_idx < PLATFORM_CHIP_COUNT; ++chip_idx) {
		channels[chip_idx] = 0;
		for (port_idx = 0; port_idx < DDR_PORT_NUM; ++port_idx) {
			spd_content = (struct ddr4_spd_eeprom *)baikal_dimm_spd_get(chip_idx, port_idx * 2);
			if (spd_content->mem_type == SPD_MEMTYPE_DDR4) {
				INFO("DIMM%u: DDR4 SDRAM is detected\n", chip_idx * DDR_PORT_NUM + port_idx);
				channels[chip_idx]++;
				conf |= 1 << slot_idx;
			}
			slot_idx++;
			spd_content = (struct ddr4_spd_eeprom *)baikal_dimm_spd_get(chip_idx, port_idx * 2 + 1);
			if (spd_content->mem_type == SPD_MEMTYPE_DDR4) {
				conf |= 1 << slot_idx;
			}
			slot_idx++;
		}
	}

	bootflash_init();

	for (chip_idx = 0, slot_idx = 0; chip_idx < PLATFORM_CHIP_COUNT; ++chip_idx) {
		for (port_idx = 0; port_idx < DDR_PORT_NUM; ++port_idx, slot_idx += 2) {
			if (conf & (1 << slot_idx)) {
				bool dual_channel_mode = (conf & (1 << (slot_idx + 1))) ? true : false;

				spd_content = (struct ddr4_spd_eeprom *)baikal_dimm_spd_get(chip_idx, port_idx * 2);
				ret = ddr_port_init(chip_idx * DDR_PORT_NUM + port_idx,
						    spd_content,
						    channels[chip_idx],
						    dual_channel_mode);
				if (ret) {
					goto error;
				}
			}
		}
	}

	return 0;
error:
	ERROR("DDR init failed\n");
	return -1;
}
