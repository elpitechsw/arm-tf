/*
 * Copyright (c) 2019-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <baikal_def.h>
#include <baikal_pvt.h>

static uintptr_t pvt_get_reg_addr(const uintptr_t base, const unsigned int offset)
{
	/* Ensure that the offset in PVT region range */
	if (offset > 0x40) {
		return 0;
	}

	return base + offset;
}

uint32_t pvt_read_reg(const uintptr_t base, const unsigned int offset)
{
	const uintptr_t reg_addr = pvt_get_reg_addr(base, offset);

	if (reg_addr == 0) {
		return 1;
	}
#ifndef BAIKAL_QEMU
	return mmio_read_32(reg_addr);
#endif
	return 0;
}

uint32_t pvt_write_reg(const uintptr_t base, const unsigned int offset, const uint32_t val)
{
	const uintptr_t reg_addr = pvt_get_reg_addr(base, offset);

	if (reg_addr == 0) {
		return 1;
	}
#ifndef BAIKAL_QEMU
	mmio_write_32(reg_addr, val);
#endif
	return 0;
}
