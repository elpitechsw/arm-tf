/*
 * Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/smmu_v3.h>
#include <common/debug.h>
#include <lib/mmio.h>

#include <bs1000_def.h>

#include "bs1000_smmu.h"

#define SMMUV3_STE_BYPASS(ste, ma, mt, alloc, sh, ns, priv, inst)				\
	do {											\
		uint32_t *pe = (uint32_t *)(ste);						\
												\
		pe[0] =  SMMU_STE_0_V;								\
		pe[0] |= SMMU_STE_CONFIG_BYPASS_BYPASS << SMMU_STE_0_CONFIG_SHIFT;		\
		pe[3] =  ((ma) << SMMU_STE_3_MEMATTR_SHIFT) & SMMU_STE_3_MEMATTR_MASK;		\
		if ((mt) != 0)									\
			pe[3] |= SMMU_STE_3_MTCFG;						\
		pe[3] |= ((alloc) << SMMU_STE_3_ALLOCCFG_SHIFT) & SMMU_STE_3_ALLOCCFG_MASK;	\
		pe[3] |= ((sh) << SMMU_STE_3_SHCFG_SHIFT) & SMMU_STE_3_SHCFG_MASK;		\
		pe[3] |= ((ns) << SMMU_STE_3_NSCFG_SHIFT) & SMMU_STE_3_NSCFG_MASK;		\
		pe[3] |= ((priv) << SMMU_STE_3_PRIVCFG_SHIFT) & SMMU_STE_3_PRIVCFG_MASK;	\
		pe[3] |= ((inst) << SMMU_STE_3_INSTCFG_SHIFT) & SMMU_STE_3_INSTCFG_MASK;	\
	} while (false)

void bs1000_smmu_s_init(void)
{
	uint64_t base_val;
	uintptr_t ste, smmu_base;
	unsigned int chip_idx;

	memset((void *)BAIKAL_SCMM_SMMU_STRTAB_BASE, 0, BAIKAL_SCMM_SMMU_STRTAB_SIZE);
	for (chip_idx = 0; chip_idx < PLATFORM_CHIP_COUNT; ++chip_idx) {
		smmu_base = PLATFORM_ADDR_OUT_CHIP(chip_idx, SMMU_BASE);
		/* Allocate Command queue */
		base_val = BAIKAL_SCMM_SMMU_CMDQ_BASE | BAIKAL_SCMM_SMMU_CMDQ_LOG2SIZE;
		mmio_write_64(smmu_base + SMMU_S_CMDQ_BASE, base_val);
		mmio_write_32(smmu_base + SMMU_S_CMDQ_PROD, 0);
		mmio_write_32(smmu_base + SMMU_S_CMDQ_CONS, 0);
		/* Allocate Event queue */
		base_val = BAIKAL_SCMM_SMMU_EVENTQ_BASE | BAIKAL_SCMM_SMMU_EVENTQ_LOG2SIZE;
		mmio_write_64(smmu_base + SMMU_S_EVENTQ_BASE, base_val);
		mmio_write_32(smmu_base + SMMU_S_EVENTQ_PROD, 0);
		mmio_write_32(smmu_base + SMMU_S_EVENTQ_CONS, 0);
		/* Configure Stream table */
		mmio_write_32(smmu_base + SMMU_S_STRTAB_BASE_CFG, BAIKAL_SCMM_SMMU_STRTAB_LOG2SIZE);
		mmio_write_64(smmu_base + SMMU_S_STRTAB_BASE, BAIKAL_SCMM_SMMU_STRTAB_BASE);
		dsb();
		/* Initialize Command and Event queues */
		if (smmuv3_enable_queues(smmu_base, true) != 0) {
			ERROR("Chip%u: SCMM SMMU initialization failed.\n", chip_idx);
			return;
		}
		/* Invalidate TLB's and configuration caches */
		if (smmuv3_invalidate_all(smmu_base) != 0) {
			ERROR("Chip%u: SCMM SMMU invalidation failed.\n", chip_idx);
			return;
		}
	}

	/* Create Stream Table Entries */

	/*
	 * Due to hardware bug in BE-S1000, GMAC's have wrong AxCACHE and AxPROT
	 * AXI bus transactions attributes. This results in GMAC's DMA operations
	 * are not cache-coherent. We use SMMU to set these attributes to the
	 * proper values.
	 */
	ste = BAIKAL_SCMM_SMMU_STRTAB_BASE;
	SMMUV3_STE_BYPASS(ste, SMMU_STE_MEMATTR_IWB_OWB, SMMU_STE_MTCFG_REPLACE_MEMATTR,
			  SMMU_STE_ALLOCCFG_RW, SMMU_STE_SHCFG_OUTER, SMMU_STE_NSCFG_NSEC,
			  SMMU_STE_PRIVCFG_USE_INCOMING, SMMU_STE_INSTCFG_USE_INCOMING);
	memcpy((void *)BAIKAL_SCMM_SMMU_STRTAB_BASE + 0xa * SMMU_STE_SIZE, (void *)ste, SMMU_STE_SIZE);

	ste = BAIKAL_SCMM_SMMU_STRTAB_BASE + 0x1 * SMMU_STE_SIZE;
	SMMUV3_STE_BYPASS(ste, 0, SMMU_STE_MTCFG_USE_INCOMING, SMMU_STE_ALLOCCFG_USE_INCOMING,
			  SMMU_STE_SHCFG_USE_INCOMING, SMMU_STE_NSCFG_USE_INCOMING,
			  SMMU_STE_PRIVCFG_USE_INCOMING, SMMU_STE_INSTCFG_USE_INCOMING);
	memcpy((void *)BAIKAL_SCMM_SMMU_STRTAB_BASE + 0x2 * SMMU_STE_SIZE, (void *)ste, SMMU_STE_SIZE);
	memcpy((void *)BAIKAL_SCMM_SMMU_STRTAB_BASE + 0x3 * SMMU_STE_SIZE, (void *)ste, SMMU_STE_SIZE);
	memcpy((void *)BAIKAL_SCMM_SMMU_STRTAB_BASE + 0x4 * SMMU_STE_SIZE, (void *)ste, SMMU_STE_SIZE);
	memcpy((void *)BAIKAL_SCMM_SMMU_STRTAB_BASE + 0x8 * SMMU_STE_SIZE, (void *)ste, SMMU_STE_SIZE);
	memcpy((void *)BAIKAL_SCMM_SMMU_STRTAB_BASE + 0xc * SMMU_STE_SIZE, (void *)ste, SMMU_STE_SIZE);
	dsb();
	/* Enable SMMU */
	for (chip_idx = 0; chip_idx < PLATFORM_CHIP_COUNT; ++chip_idx) {
		smmu_base = PLATFORM_ADDR_OUT_CHIP(chip_idx, SMMU_BASE);
		if (smmuv3_enable(smmu_base, true) != 0) {
			ERROR("Chip%u: SCMM SMMU enable failed.\n", chip_idx);
		}
	}
}
