/*
 * Copyright (c) 2020-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <drivers/arm/gicv3.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>

#include <baikal_def.h>
#include <baikal_gicv3.h>
#include <platform_def.h>

/* The GICv3 driver only needs to be initialized in EL3 */
static uintptr_t baikal_rdistif_base_addrs[PLATFORM_CORE_COUNT];

/* Default GICR base address to be used for GICR probe. */
static const uintptr_t gicr_base_addrs[2] = {
	BAIKAL_GICR_BASE,	/* GICR Base address of the primary CPU */
	0U			/* Zero Termination */
};

/* List of zero terminated GICR frame addresses which CPUs will probe */
static const uintptr_t *gicr_frames = gicr_base_addrs;

static const interrupt_prop_t baikal_interrupt_props[] = {
	INTR_PROP_DESC(BAIKAL_IRQ_SEC_SGI_0, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP1S, GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(BAIKAL_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP1S, GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(BAIKAL_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP1S, GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(BAIKAL_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP1S, GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(BAIKAL_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP1S, GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(BAIKAL_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP1S, GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(BAIKAL_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP1S, GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(BAIKAL_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP1S, GIC_INTR_CFG_EDGE),
#ifdef MULTICHIP_SYSTEM_COUNTERS_RESYNC
	INTR_PROP_DESC(BAIKAL_SYNC_TIMER_IRQ, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP0, GIC_INTR_CFG_LEVEL),
#endif
};

static unsigned int baikal_mpidr_to_core_pos(u_register_t mpidr)
{
	return plat_core_pos_by_mpidr(mpidr);
}

static const struct gicv3_driver_data baikal_gic_data = {
	.gicd_base = BAIKAL_GICD_BASE,
	.gicr_base = 0U,
	.interrupt_props = baikal_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(baikal_interrupt_props),
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = baikal_rdistif_base_addrs,
	.mpidr_to_core_pos = baikal_mpidr_to_core_pos
};

/*
 * By default, gicr_frames will be pointing to gicr_base_addrs. If
 * the platform supports a non-contiguous GICR frames (GICR frames located
 * at uneven offset), plat_arm_override_gicr_frames function can be used by
 * such platform to override the gicr_frames.
 */
void baikal_override_gicr_frames(const uintptr_t *plat_gicr_frames)
{
	assert(plat_gicr_frames != NULL);
	gicr_frames = plat_gicr_frames;
}

void baikal_gic_driver_init(void)
{
	gicv3_driver_init(&baikal_gic_data);

	if (gicv3_rdistif_probe(gicr_base_addrs[0]) == -1) {
		ERROR("No GICR base frame found for Primary CPU\n");
		panic();
	}
}

void baikal_gic_init(void)
{
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

void baikal_gic_cpuif_enable(void)
{
	gicv3_cpuif_enable(plat_my_core_pos());
}

void baikal_gic_cpuif_disable(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
}

void baikal_gic_pcpu_init(void)
{
	int result;
	const uintptr_t *plat_gicr_frames = gicr_frames;

	do {
		result = gicv3_rdistif_probe(*plat_gicr_frames);

		/* If the probe is successful, no need to proceed further */
		if (result == 0)
			break;

		plat_gicr_frames++;
	} while (*plat_gicr_frames != 0U);

	if (result == -1) {
		ERROR("No GICR base frame found for CPU 0x%lx\n", read_mpidr());
		panic();
	}
	gicv3_rdistif_init(plat_my_core_pos());
}
