/*
 * Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef XORSHIFT_H
#define XORSHIFT_H

static inline uint64_t xorshift64(uint64_t val) __attribute__((always_inline));

static inline uint64_t xorshift64(uint64_t val)
{
	val ^= val << 13;
	val ^= val >> 7;
	val ^= val << 17;
	return val;
}

#endif /* XORSHIFT_H */
