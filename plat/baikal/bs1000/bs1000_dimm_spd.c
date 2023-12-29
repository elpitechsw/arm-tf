/*
 * Copyright (c) 2022-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <lib/utils_def.h>

#include <platform_def.h>
#include <baikal_def.h>
#include <bs1000_def.h>
#include <bs1000_dimm_spd.h>
#include <crc.h>
#include <dw_i2c.h>
#include <spd.h>

#define SPD_MAXSIZE	512

#define SPD_SPA0	0x36
#define SPD_SPA1	0x37

static uint8_t spd_data[SPD_MAXSIZE];

static void baikal_dimm_spd_read(const unsigned int chip_idx,
				 const unsigned int dimm_idx)
{
	const uintptr_t base = dimm_idx < (BS1000_DIMM_NUM / 2) ?
				PLATFORM_ADDR_OUT_CHIP(chip_idx, I2C2_BASE) :
				PLATFORM_ADDR_OUT_CHIP(chip_idx, I2C3_BASE);
	uint8_t *buf = spd_data;
	int rxsize;
	const unsigned int spd_addr = 0x50 + dimm_idx % 6;
	uint8_t startaddr = 0;

	memset(buf, 0xff, SPD_MAXSIZE);

	i2c_txrx(base, BAIKAL_I2C_ICLK_FREQ,
		 SPD_SPA0,
		 &startaddr, sizeof(startaddr),
		 NULL, 0);
	rxsize = i2c_txrx(base, BAIKAL_I2C_ICLK_FREQ,
			  spd_addr,
			  &startaddr, sizeof(startaddr),
			  buf, 128);
	if (rxsize == 128 &&
	    crc16(buf, 126, 0) == spd_get_baseconf_crc(buf)) {
		const unsigned int bytes_used = buf[0] & 0xf;

		if (bytes_used > 1 && bytes_used < 5) {
			buf += rxsize;
			startaddr += rxsize;
			rxsize = i2c_txrx(base,
					  BAIKAL_I2C_ICLK_FREQ,
					  spd_addr,
					  &startaddr,
					  sizeof(startaddr),
					  buf, 128);
			if (rxsize == 128 && bytes_used > 2) {
				buf += rxsize;
				startaddr = 0;
				i2c_txrx(base,
					 BAIKAL_I2C_ICLK_FREQ,
					 SPD_SPA1,
					 &startaddr,
					 sizeof(startaddr),
					 NULL, 0);
				i2c_txrx(base,
					 BAIKAL_I2C_ICLK_FREQ,
					 spd_addr,
					 &startaddr,
					 sizeof(startaddr),
					 buf,
					 bytes_used == 3 ? 128 :
							   256);
				i2c_txrx(base,
					 BAIKAL_I2C_ICLK_FREQ,
					 SPD_SPA0,
					 &startaddr,
					 sizeof(startaddr),
					 NULL, 0);
			}
		}
	}
}

const void *baikal_dimm_spd_get(const unsigned int chip_idx,
				const unsigned int dimm_idx)
{
	if (chip_idx >= PLATFORM_CHIP_COUNT || dimm_idx >= BS1000_DIMM_NUM) {
		return NULL;
	}

	baikal_dimm_spd_read(chip_idx, dimm_idx);

	return spd_data;
}
