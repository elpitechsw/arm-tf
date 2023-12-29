/*
 * Copyright (c) 2018-2022, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <lib/utils_def.h>

#include <baikal_scp.h>
#include <bm1000_scp_flash.h>

#define SCP_FLASH_MAX_CHUNK_SIZE	UL(1024)

int scp_flash_init(void)
{
	return scp_cmd('T', 0, 0); /* disable trace */
}

int scp_flash_erase(uint32_t addr, size_t size)
{
	while (size) {
		size_t chunk = MIN(size, SCP_FLASH_MAX_CHUNK_SIZE);
		int err;

		err = scp_cmd('E', addr, chunk);
		if (err) {
			return err;
		}

		addr += chunk;
		size -= chunk;
	}

	return 0;
}

int scp_flash_read(uint32_t addr, void *buf, size_t size)
{
	uint8_t *pbuf = buf;

	while (size) {
		size_t chunk = MIN(size, SCP_FLASH_MAX_CHUNK_SIZE);
		int err;

		err = scp_cmd('R', addr, chunk);
		if (err) {
			return -1;
		}

		memcpy(pbuf, scp_buf(), chunk);
		addr += chunk;
		pbuf += chunk;
		size -= chunk;
	}

	return 0;
}

int scp_flash_write(uint32_t addr, void *data, size_t size)
{
	uint8_t *pdata = data;

	while (size) {
		size_t chunk = MIN(size, SCP_FLASH_MAX_CHUNK_SIZE);
		int err;

		memcpy(scp_buf(), pdata, chunk);
		err = scp_cmd('W', addr, chunk);
		if (err) {
			return err;
		}

		addr  += chunk;
		pdata += chunk;
		size  -= chunk;
	}

	return 0;
}
