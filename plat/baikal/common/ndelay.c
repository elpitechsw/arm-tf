/*
 * Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <plat/common/platform.h>

#include <ndelay.h>

void ndelay(const uint64_t nsec)
{
	const unsigned int div = 1000000000 / plat_get_syscnt_freq2();
	const uint64_t expcnt = read_cntpct_el0() + (nsec + div - 1) / div;

	while (read_cntpct_el0() <= expcnt)
		;
}
