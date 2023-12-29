/*
 * Copyright (c) 2018-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <bm1000_cmu.h>
#include <bm1000_def.h>
#include <bm1000_private.h>
#include <platform_def.h>

#include "bm1000_mmca57.h"

#define MMCA57_SCLKEN			0x0808
#define MMCA57_SCLKEN_EN		BIT(0)
#define MMCA57_SCLKEN_SET_DIV		BIT(2)
#define MMCA57_SCLKEN_VAL_DIV_MASK	(0xff << 4)
#define MMCA57_SCLKEN_VAL_DIV_SHIFT	4
#define MMCA57_SCLKEN_LOCK_DIV		BIT(31)

#define MMCA57_CFG_RST			0x0814
#define MMCA57_CFG_RST_CPUPORESET0	BIT(0)
#define MMCA57_CFG_RST_CORERESET0	BIT(8)
#define MMCA57_CFG_RST_L2RST		BIT(16)
#define MMCA57_CFG_RST_BRDGRST		BIT(24)
#define MMCA57_CFG_RST_BRDGRST2		BIT(25)
#define MMCA57_CFG_RST_BRDGRST3		BIT(26)
#define MMCA57_CFG_RST_BRDGRST4		BIT(27)
#define MMCA57_CFG_RST_PRESETDBG	BIT(28)
#define MMCA57_CFG_RST_GICRST		BIT(29)

static const uintptr_t mmca57_bases[] = {
	MMCA57_0_BASE,
	MMCA57_1_BASE,
	MMCA57_2_BASE,
	MMCA57_3_BASE
};
CASSERT(ARRAY_SIZE(mmca57_bases) == PLATFORM_CLUSTER_COUNT, assert_mmca57_bases_size);

unsigned int mmca57_get_sclk_div(const uintptr_t base)
{
	uint32_t reg;

	assert(base == mmca57_bases[0] ||
	       base == mmca57_bases[1] ||
	       base == mmca57_bases[2] ||
	       base == mmca57_bases[3]);

	reg = mmio_read_32(base + MMCA57_SCLKEN);
	reg &=	MMCA57_SCLKEN_VAL_DIV_MASK;
	reg >>=	MMCA57_SCLKEN_VAL_DIV_SHIFT;
	return reg;
}

void mmca57_enable_core(const u_register_t mpidr)
{
	const unsigned int cluster = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	const unsigned int core    = mpidr & 0x3;
	uint32_t reg;

	assert(cluster < PLATFORM_CLUSTER_COUNT);
	assert(core < PLATFORM_MAX_CPUS_PER_CLUSTER);

	const uintptr_t base = mmca57_bases[cluster];

	if (!cmu_pll_is_enabled(base)) {
		int ret;

		cmu_pll_set_gains(base, 0, -20, 0, -20);
		ret = cmu_pll_enable(base);
		if (ret) {
			return;
		}
	}

	/* Assert core resets (perform powerup reset) */
	reg = mmio_read_32(base + MMCA57_CFG_RST);
	reg |= (MMCA57_CFG_RST_CORERESET0 |
		MMCA57_CFG_RST_CPUPORESET0) << core;
	mmio_write_32(base + MMCA57_CFG_RST, reg);
	udelay(1);

	/* Deassert cluster resets */
	reg &= ~(MMCA57_CFG_RST_L2RST	  |
		 MMCA57_CFG_RST_BRDGRST   |
		 MMCA57_CFG_RST_BRDGRST2  |
		 MMCA57_CFG_RST_BRDGRST3  |
		 MMCA57_CFG_RST_BRDGRST4  |
		 MMCA57_CFG_RST_PRESETDBG |
		 MMCA57_CFG_RST_GICRST);
	mmio_write_32(base + MMCA57_CFG_RST, reg);

	/*
	 * Deassert core resets.
	 * N.B. Simultaneous deassertion of cluster and core resets is prohibited.
	 */
	reg &= ~((MMCA57_CFG_RST_CORERESET0 |
		  MMCA57_CFG_RST_CPUPORESET0) << core);
	mmio_write_32(base + MMCA57_CFG_RST, reg);
}

void mmca57_reconf_sclken(const uintptr_t base, const unsigned int div)
{
	uint32_t reg;

	assert(base == mmca57_bases[0] ||
	       base == mmca57_bases[1] ||
	       base == mmca57_bases[2] ||
	       base == mmca57_bases[3]);

	assert(div >= 1 && div <= 3);

	dsb();

	reg = mmio_read_32(base + MMCA57_SCLKEN);
	reg &= ~MMCA57_SCLKEN_SET_DIV;

	/*
	 * Reconfiguration routine requires to reset SCLKEN.EN bit.
	 * But disabling of cluster SCLK by a core of the cluster
	 * leads to freezing of the core (it is obvious).
	 */
	if (base != mmca57_bases[(read_mpidr() >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK]) {
		reg &= ~MMCA57_SCLKEN_EN;
	}

	mmio_write_32(base + MMCA57_SCLKEN, reg);
	reg &= ~MMCA57_SCLKEN_VAL_DIV_MASK;
	reg |= div << MMCA57_SCLKEN_VAL_DIV_SHIFT;
	mmio_write_32(base + MMCA57_SCLKEN, reg);
	reg |= MMCA57_SCLKEN_SET_DIV;
	mmio_write_32(base + MMCA57_SCLKEN, reg);

	/* SCLKEN.LOCK_DIV bit is set only when PLL is enabled */
	if (cmu_pll_is_enabled(base)) {
		uint64_t timeout;

		timeout = timeout_init_us(100000);
		while (!(mmio_read_32(base + MMCA57_SCLKEN) & MMCA57_SCLKEN_LOCK_DIV)) {
			if (timeout_elapsed(timeout)) {
				ERROR("%s(0x%lx, %u): SCLKEN.LOCK_DIV timeout\n",
				      __func__, base, div);
				break;
			}
		}
	}

	mmio_setbits_32(base + MMCA57_SCLKEN, MMCA57_SCLKEN_EN);
}
