/*
 * Copyright (c) 2020-2022, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <spd.h>

unsigned int spd_get_baseconf_crc(const void *const baseconf)
{
	const uint8_t *const spdbuf = baseconf;

	return (spdbuf[127] << 8) | spdbuf[126];
}

unsigned long long spd_get_baseconf_dimm_capacity(const void *const baseconf)
{
	const uint8_t *const spdbuf = baseconf;
	unsigned long long sdram_capacity_per_die;

	/* SDRAM capacity in MiB */
	switch (spdbuf[4] & 0xf) {
	case 0:
		sdram_capacity_per_die = 256;
		break;
	case 1:
		sdram_capacity_per_die = 512;
		break;
	case 2:
		sdram_capacity_per_die = 1024;
		break;
	case 3:
		sdram_capacity_per_die = 2 * 1024;
		break;
	case 4:
		sdram_capacity_per_die = 4 * 1024;
		break;
	case 5:
		sdram_capacity_per_die = 8 * 1024;
		break;
	case 6:
		sdram_capacity_per_die = 16 * 1024;
		break;
	case 7:
		sdram_capacity_per_die = 32 * 1024;
		break;
	case 8:
		sdram_capacity_per_die = 12 * 1024;
		break;
	case 9:
		sdram_capacity_per_die = 24 * 1024;
		break;
	default:
		return 0;
	}

	/* SDRAM capacity in bytes */
	sdram_capacity_per_die *= 1024 * 1024;

	const unsigned short primary_bus_width = 8 << (spdbuf[13] & 0x7);
	const unsigned char sdram_device_width = 4 << (spdbuf[12] & 0x3);
	const unsigned char logical_ranks_per_dimm = ((spdbuf[12] >> 3) & 0x7) + 1;
	const unsigned char primary_sdram_package_type = (spdbuf[6] >> 7) & 0x1;
	const unsigned char die_count = ((spdbuf[6] >> 4) & 0x7) + 1;
	const unsigned char signal_loading = spdbuf[6] & 0x3;

	if (sdram_device_width > 32 || primary_bus_width > 64) {
		return 0;
	}

	unsigned long long total;

	total  = sdram_capacity_per_die;
	total /= 8;
	total *= primary_bus_width;
	total /= sdram_device_width;

	if (primary_sdram_package_type == 0) {
		if (signal_loading == 0) {
			total *= logical_ranks_per_dimm;
		} else {
			return 0;
		}
	} else {
		if (signal_loading == 1) {
			total *= logical_ranks_per_dimm;
		} else if (signal_loading == 2) {
			total *= logical_ranks_per_dimm * die_count;
		} else {
			return 0;
		}
	}

	return total;
}
