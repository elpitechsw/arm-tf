/*
 * Copyright (c) 2021-2022, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BAIKAL_PVT_H
#define BAIKAL_PVT_H

#include <stdint.h>

#define PVT_READ	0
#define PVT_WRITE	1

uint32_t pvt_read_reg(const uintptr_t base, const unsigned int offset);
uint32_t pvt_write_reg(const uintptr_t base, const unsigned int offset, const uint32_t val);

#endif /* BAIKAL_PVT_H */
