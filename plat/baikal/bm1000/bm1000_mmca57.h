/*
 * Copyright (c) 2018-2022, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BM1000_MMCA57_H
#define BM1000_MMCA57_H

#include <stdint.h>

unsigned int mmca57_get_sclk_div(const uintptr_t base);
void	     mmca57_enable_core(const u_register_t mpidr);
void	     mmca57_reconf_sclken(const uintptr_t base, const unsigned int div);

#endif /* BM1000_MMCA57_H */
