/*
 * Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MEMTEST_H
#define MEMTEST_H

#include <stdint.h>

int memtest_rand64(const uintptr_t base,
		   const size_t size,
		   const unsigned int incr,
		   const uint64_t seed);

int memtest_rand8(const uintptr_t base,
		  const size_t size,
		  const unsigned int incr,
		  const uint64_t seed);

#endif /* MEMTEST_H */
