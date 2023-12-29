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

void mmmali_init(void)
{
	const cmu_pll_ctl_vals_t mmmali_cmu0_pll_ctls = {
		0, 0, 0x6800000000, 0, -20, 0, -20
	};

	cmu_pll_on(MMMALI_CMU0_BASE, &mmmali_cmu0_pll_ctls);
	cmu_clkch_enable_by_base(MMMALI_CMU0_CLKCHCTL_AXI, 2);
	mmio_clrbits_32(MMMALI_GPR_MMRST,
			MMMALI_GPR_MMRST_NIC |
			MMMALI_GPR_MMRST_PVT);

	/* Enable non-secure access */
	mmio_write_32(MMMALI_GPR_AXPROT0,
		      MMMALI_GPR_AXPROT_AWPROT(2) |
		      MMMALI_GPR_AXPROT_ARPROT(2));

	mmio_write_32(MMMALI_GPR_AXPROT1,
		      MMMALI_GPR_AXPROT_AWPROT(2) |
		      MMMALI_GPR_AXPROT_ARPROT(2));

	mmio_write_32(MMMALI_NIC_CFG_GPV_REGIONSEC_MALI, NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMMALI_NIC_CFG_GPV_REGIONSEC_SMMU, NIC_GPV_REGIONSEC_NONSECURE);
}
