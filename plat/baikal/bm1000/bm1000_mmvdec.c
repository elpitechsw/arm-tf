/*
 * Copyright (c) 2018-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <bm1000_cmu.h>
#include <bm1000_def.h>
#include <bm1000_private.h>
#include <bm1000_smmu.h>

void mmvdec_init(void)
{
	/*
	 * Set PLL clock to 1.2 GHz. It will be further divided by 3
	 * by a clock channel to minimize jitter.
	 */
	const cmu_pll_ctl_vals_t mmvdec_cmu0_pll_ctls = {
		0, 0, 0x6000000000, 0, -20, 0, -20
	};

	cmu_pll_on(MMVDEC_CMU0_BASE, &mmvdec_cmu0_pll_ctls);

	/* 1.2 GHz / 3 = 400 MHz */
	cmu_clkch_enable_by_base(MMVDEC_CMU0_CLKCHCTL_SYS, 3);

	/* Deassert reset signals */
	mmio_clrbits_32(MMVDEC_GPR_MMRST,
			MMVDEC_GPR_MMRST_SLAVE |
			MMVDEC_GPR_MMRST_VDEC  |
			MMVDEC_GPR_MMRST_SMMU  |
			MMVDEC_GPR_MMRST_SMMU_SLV);

	mmvdec_smmu_set_normalize(0);
	mmvdec_smmu_set_domain_cache(0x2, 0x2, 0x7);
	mmvdec_smmu_set_qos(0xf, 0xf);

	/* Enable non-secure access */
	mmio_write_32(MMVDEC_NIC_CFG_GPV_REGIONSEC_VDEC, NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMVDEC_NIC_CFG_GPV_REGIONSEC_SMMU, NIC_GPV_REGIONSEC_NONSECURE);

	mmio_clrsetbits_32(MMVDEC_GPR_CFG1,
			   MMVDEC_GPR_CFG1_AWPROT_MASK |
			   MMVDEC_GPR_CFG1_ARPROT_MASK,
			   MMVDEC_GPR_CFG1_AWPROT(0x2) |
			   MMVDEC_GPR_CFG1_ARPROT(0x2));
}
