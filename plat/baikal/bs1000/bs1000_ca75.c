/*
 * Copyright (c) 2021-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <lib/mmio.h>

#include <bs1000_def.h>
#include <platform_def.h>

#include "bs1000_ca75.h"

#define CA75_GPR_RST_CTL			0x20000
#define CA75_GPR_RST_CTL_CPUPORESET0		BIT(0)
#define CA75_GPR_RST_CTL_CORERESET0		BIT(4)

#define CA75_GPR_DSU_CORE0_PMU_CTL		0x20060
#define CA75_GPR_DSU_CORE_PMU_CTL_PREQ		BIT(6)
#define CA75_GPR_DSU_CORE_PMU_CTL_PSTATE_OFF	0
#define CA75_GPR_DSU_CORE_PMU_CTL_PSTATE_ON	8

#define CA75_GPR_DSU_CORE0_PMU_STS		0x20064
#define CA75_GPR_DSU_CORE_PMU_STS_PACCEPT	BIT(18)
#define CA75_GPR_DSU_CORE_PMU_STS_PDENY		BIT(19)

static const uintptr_t ca75_bases[] = {
	CA75_0_BASE,
	CA75_1_BASE,
	CA75_2_BASE,
	CA75_3_BASE,
	CA75_4_BASE,
	CA75_5_BASE,
	CA75_6_BASE,
	CA75_7_BASE,
	CA75_8_BASE,
	CA75_9_BASE,
	CA75_10_BASE,
	CA75_11_BASE
};
CASSERT(ARRAY_SIZE(ca75_bases) == PLATFORM_CLUSTER_COUNT, assert_ca75_bases_size);

void ca75_core_enable(const u_register_t mpidr)
{
	const unsigned int chip    = MPIDR_AFFLVL3_VAL(mpidr);
	const unsigned int cluster = MPIDR_AFFLVL2_VAL(mpidr);
	const unsigned int core    = MPIDR_AFFLVL1_VAL(mpidr);

	assert(chip < PLATFORM_CHIP_COUNT);
	assert(cluster < PLATFORM_CLUSTER_COUNT);
	assert(core < PLATFORM_MAX_CPUS_PER_CLUSTER);

	const uintptr_t base = PLATFORM_ADDR_OUT_CHIP(chip, ca75_bases[cluster]);

	/* Deassert resets */
	mmio_clrbits_32(base + CA75_GPR_RST_CTL,
			(CA75_GPR_RST_CTL_CORERESET0 |
			 CA75_GPR_RST_CTL_CPUPORESET0) << core);
}

void ca75_core_warm_reset(const u_register_t mpidr)
{
	const unsigned int chip    = MPIDR_AFFLVL3_VAL(mpidr);
	const unsigned int cluster = MPIDR_AFFLVL2_VAL(mpidr);
	const unsigned int core    = MPIDR_AFFLVL1_VAL(mpidr);

	assert(chip < PLATFORM_CHIP_COUNT);
	assert(cluster < PLATFORM_CLUSTER_COUNT);
	assert(core < PLATFORM_MAX_CPUS_PER_CLUSTER);

	const uintptr_t base = PLATFORM_ADDR_OUT_CHIP(chip, ca75_bases[cluster]);

	/* Switch the core to powerdown state */
	mmio_write_32(base + CA75_GPR_DSU_CORE0_PMU_CTL + 8 * core,
		      CA75_GPR_DSU_CORE_PMU_CTL_PREQ |
		      CA75_GPR_DSU_CORE_PMU_CTL_PSTATE_OFF);

	/* Wait for PACCEPT == 1 && PDENY == 0 */
	while ((mmio_read_32(base + CA75_GPR_DSU_CORE0_PMU_STS + 8 * core) &
				   (CA75_GPR_DSU_CORE_PMU_STS_PACCEPT |
				    CA75_GPR_DSU_CORE_PMU_STS_PDENY)) !=
				    CA75_GPR_DSU_CORE_PMU_STS_PACCEPT)
		;

	/* Deassert PREQ */
	mmio_clrbits_32(base + CA75_GPR_DSU_CORE0_PMU_CTL + 8 * core,
			CA75_GPR_DSU_CORE_PMU_CTL_PREQ);

	/* Wait for PACCEPT == 0 */
	while (mmio_read_32(base + CA75_GPR_DSU_CORE0_PMU_STS + 8 * core) &
				   CA75_GPR_DSU_CORE_PMU_STS_PACCEPT)
		;

	/* Assert core warm reset */
	mmio_setbits_32(base + CA75_GPR_RST_CTL,
			       CA75_GPR_RST_CTL_CORERESET0 << core);

	/* Switch the core to powerup state */
	mmio_write_32(base + CA75_GPR_DSU_CORE0_PMU_CTL + 8 * core,
		      CA75_GPR_DSU_CORE_PMU_CTL_PREQ |
		      CA75_GPR_DSU_CORE_PMU_CTL_PSTATE_ON);

	/* Deassert core warm reset */
	mmio_clrbits_32(base + CA75_GPR_RST_CTL,
			CA75_GPR_RST_CTL_CORERESET0 << core);

	/* Wait for PACCEPT == 1 && PDENY == 0 */
	while ((mmio_read_32(base + CA75_GPR_DSU_CORE0_PMU_STS + 8 * core) &
				   (CA75_GPR_DSU_CORE_PMU_STS_PACCEPT |
				    CA75_GPR_DSU_CORE_PMU_STS_PDENY)) !=
				    CA75_GPR_DSU_CORE_PMU_STS_PACCEPT)
		;

	/* Deassert PREQ */
	mmio_clrbits_32(base + CA75_GPR_DSU_CORE0_PMU_CTL + 8 * core,
			CA75_GPR_DSU_CORE_PMU_CTL_PREQ);

	/* Wait for PACCEPT == 0 */
	while (mmio_read_32(base + CA75_GPR_DSU_CORE0_PMU_STS + 8 * core) &
				   CA75_GPR_DSU_CORE_PMU_STS_PACCEPT)
		;
}
