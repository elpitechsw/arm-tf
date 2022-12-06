/*
 * Copyright (c) 2021-2022, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <memtest.h>
#include <xorshift.h>

int memtest_rand64(const uintptr_t base,
		   const size_t size,
		   const unsigned int incr,
		   const uint64_t seed)
{
	uint64_t *ptr;
	int ret = 0;
	int err_cnt = 0;
	uint64_t all_set = 0, all_cleared = 0;
	uint64_t val;

	assert(!(base % sizeof(uint64_t)));
	assert(size > 0);
	assert(!(size % sizeof(uint64_t)));
	assert(incr > 0);
	assert(!(incr % sizeof(uint64_t)));

	INFO("%s: 0x%lx-0x%lx / 0x%x\n", __func__, base, base + size - 1, incr);

	val = seed;
	ptr = (uint64_t *)base;
	do {
		*ptr = val = xorshift64(val);
		ptr += incr / sizeof(uint64_t);
	} while (ptr < (uint64_t *)(base + size));

	val = seed;
	ptr = (uint64_t *)base;
	do {
		val = xorshift64(val);
		if (*ptr != val) {
			uint64_t set, cleared;
			uint64_t d = *ptr ^ val;
                        set = d & *ptr;
                        cleared = d & val;
                        all_set |= set;
                        all_cleared |= cleared;

			ret = -1;
			if (err_cnt++ < 10) {
			ERROR("%s @ %p: expected:0x%016lx actual:0x%016lx\n",
			      __func__, ptr, val, *ptr);
			ERROR("\t(set: %016lx, cleared: %016lx)\n",
                                set, cleared);
			}
		}

		ptr += incr / sizeof(uint64_t);
	} while (ptr < (uint64_t *)(base + size));

	if (err_cnt) {
                ERROR("%s: total %d errors (set bits: %016lx, cleared bits: %016lx)\n",
                        __func__, err_cnt, all_set, all_cleared);
        }

	return ret;
}

int memtest_rand8(const uintptr_t base,
		  const size_t size,
		  const unsigned int incr,
		  const uint64_t seed)
{
	uint8_t *ptr;
	int ret = 0;
	uint64_t val;

	assert(size > 0);
	assert(incr > 0);

	INFO("%s:  0x%lx-0x%lx / 0x%x\n", __func__, base, base + size - 1, incr);

	val = seed;
	ptr = (uint8_t *)base;
	do {
		*ptr = val = xorshift64(val);
		ptr += incr;
	} while (ptr < (uint8_t *)(base + size));

	val = seed;
	ptr = (uint8_t *)base;
	do {
		val = xorshift64(val);
		if (*ptr != (uint8_t)val) {
			ret = -1;
			ERROR("%s @ %p: expected:0x%02x actual:0x%02x\n",
			      __func__, ptr, (uint8_t)val, *ptr);
		}

		ptr += incr;
	} while (ptr < (uint8_t *)(base + size));

	return ret;
}
