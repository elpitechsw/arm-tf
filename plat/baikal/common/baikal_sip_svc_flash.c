/*
 * Copyright (c) 2020-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <cdefs.h>
#include <common/debug.h>

#include <baikal_bootflash.h>
#include <baikal_def.h>
#include <baikal_sip_svc.h>

static int baikal_smc_flash_init(uint64_t *const data);
static int baikal_smc_flash_position(const unsigned int pos);
static int baikal_smc_flash_pull(uint64_t *const data);
static int baikal_smc_flash_push(const uint64_t data0,
				 const uint64_t data1,
				 const uint64_t data2,
				 const uint64_t data3);

static uint8_t flash_buf[1024] __aligned(8);
static unsigned int flash_buf_idx;

int64_t baikal_smc_flash_handler(const uint32_t smc_fid,
			     const uint64_t x1,
			     const uint64_t x2,
			     const uint64_t x3,
			     const uint64_t x4,
			     uint64_t *data)
{
	if ((smc_fid & BIT(30)) == 0) {
		ERROR("%s: SMC32 (smc_fid 0x%x) is not supported\n", __func__, smc_fid);
		return -1;
	}

	switch (smc_fid) {
	case BAIKAL_SMC_FLASH_WRITE:
		return bootflash_write(x1, flash_buf, x2);
	case BAIKAL_SMC_FLASH_READ:
		return bootflash_read(x1, flash_buf, x2);
	case BAIKAL_SMC_FLASH_PUSH:
		return baikal_smc_flash_push(x1, x2, x3, x4);
	case BAIKAL_SMC_FLASH_PULL:
		return baikal_smc_flash_pull(data);
	case BAIKAL_SMC_FLASH_POSITION:
		return baikal_smc_flash_position(x1);
	case BAIKAL_SMC_FLASH_ERASE:
		return bootflash_erase(x1, x2);
	case BAIKAL_SMC_FLASH_INIT:
		return baikal_smc_flash_init(data);
	default:
		ERROR("%s: unknown smc_fid 0x%x\n", __func__, smc_fid);
		return -1;
	}
}

static int baikal_smc_flash_init(uint64_t *const data)
{
	bootflash_init();
	data[0] = BAIKAL_BOOT_SPI_SUBSECTOR;
	data[1] = BAIKAL_BOOT_SPI_SIZE / BAIKAL_BOOT_SPI_SUBSECTOR;
	return 0;
}

static int baikal_smc_flash_position(const unsigned int pos)
{
	if (pos > sizeof(flash_buf) - 4 * sizeof(uint64_t)) {
		return -1;
	}

	flash_buf_idx = pos;
	return 0;
}

static int baikal_smc_flash_pull(uint64_t *const data)
{
	memcpy(data, &flash_buf[flash_buf_idx], 4 * sizeof(uint64_t));
	flash_buf_idx += 4 * sizeof(uint64_t);
	return 0;
}

static int baikal_smc_flash_push(const uint64_t data0,
				 const uint64_t data1,
				 const uint64_t data2,
				 const uint64_t data3)
{
	uint64_t *const buf = (void *)&flash_buf[flash_buf_idx];

	buf[0] = data0;
	buf[1] = data1;
	buf[2] = data2;
	buf[3] = data3;
	flash_buf_idx += 4 * sizeof(uint64_t);
	return 0;
}
