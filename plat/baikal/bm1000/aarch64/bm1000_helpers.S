/*
 * Copyright (c) 2018-2021, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>

	.globl	plat_baikal_calc_core_pos

	/* ---------------------------------------------
	 * unsigned plat_baikal_calc_core_pos(u_register_t mpidr);
	 * ---------------------------------------------
	 */
func plat_baikal_calc_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #7
	ret
endfunc plat_baikal_calc_core_pos
