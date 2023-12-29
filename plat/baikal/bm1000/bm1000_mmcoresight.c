/*
 * Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <bm1000_def.h>
#include <bm1000_private.h>

#define MMCORESIGHT_CFG_BASE		(MMCORESIGHT_BASE + 0x1000000)
#define MMCORESIGHT_TSGEN_BASE		(MMCORESIGHT_CFG_BASE + 0x3000)
#define MMCORESIGHT_TSGEN_CNTCR		(MMCORESIGHT_TSGEN_BASE)
#define MMCORESIGHT_TSGEN_CNTCR_EN	BIT(0)

void mmcoresight_init(void)
{
	/* Deassert reset signals */
	mmio_clrbits_32(MMCORESIGHT_GPR_MMRST,
			MMCORESIGHT_GPR_MMRST_CFG_NIC_RST |
			MMCORESIGHT_GPR_MMRST_CSSYS_RST	  |
			MMCORESIGHT_GPR_MMRST_CC0_RST	  |
			MMCORESIGHT_GPR_MMRST_CC1_RST	  |
			MMCORESIGHT_GPR_MMRST_CC2_RST	  |
			MMCORESIGHT_GPR_MMRST_CC3_RST);

	/* Enable non-secure access */
	mmio_write_32(MMCORESIGHT_NIC_CFG_GPV_REGIONSEC_SMMU,  NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMCORESIGHT_NIC_CFG_GPV_REGIONSEC_APBIC, NIC_GPV_REGIONSEC_NONSECURE);

	/* Enable TSGEN */
	mmio_setbits_32(MMCORESIGHT_TSGEN_CNTCR,
			MMCORESIGHT_TSGEN_CNTCR_EN);
}
