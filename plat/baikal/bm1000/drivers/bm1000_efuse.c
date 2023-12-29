/*
 * Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <baikal_scp.h>
#include <bm1000_efuse.h>

int64_t efuse_get_lot(void)
{
	int err;

	err = scp_cmd('I', 0, 0);
	if (err) {
		return err;
	}

	return *(uint64_t *)scp_buf() & 0xffffffffffff;
}

int32_t efuse_get_mac(void)
{
	int err;

	err = scp_cmd('M', 0, 0);
	if (err) {
		return err;
	}

	return *(uint32_t *)scp_buf();
}

int32_t efuse_get_serial(void)
{
	int err;

	err = scp_cmd('N', 0, 0);
	if (err) {
		return err;
	}

	return *(uint32_t *)scp_buf();
}
