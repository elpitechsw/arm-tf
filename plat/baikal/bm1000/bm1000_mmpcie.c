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

void mmpcie_init(void)
{
	uint32_t reg;
	const cmu_pll_ctl_vals_t mmpcie_cmu0_pll_ctls = {
		0, 0, 0x7800000000, 0, -20, 0, -20
	};

	cmu_pll_on(MMPCIE_CMU0_BASE, &mmpcie_cmu0_pll_ctls);

	cmu_clkch_enable_by_base(MMPCIE_CMU0_CLKCHCTL_PCIE0_MSTR,  4); /* 375 MHz */
	cmu_clkch_enable_by_base(MMPCIE_CMU0_CLKCHCTL_PCIE0_SLV,   4); /* 375 MHz */
	cmu_clkch_enable_by_base(MMPCIE_CMU0_CLKCHCTL_PCIE0_CFG,  30); /*  50 MHz */
	cmu_clkch_enable_by_base(MMPCIE_CMU0_CLKCHCTL_PCIE1_MSTR,  4); /* 375 MHz */
	cmu_clkch_enable_by_base(MMPCIE_CMU0_CLKCHCTL_PCIE1_SLV,   4); /* 375 MHz */
	cmu_clkch_enable_by_base(MMPCIE_CMU0_CLKCHCTL_PCIE1_CFG,  30); /*  50 MHz */
	cmu_clkch_enable_by_base(MMPCIE_CMU0_CLKCHCTL_PCIE2_MSTR,  3); /* 500 MHz */
	cmu_clkch_enable_by_base(MMPCIE_CMU0_CLKCHCTL_PCIE2_SLV,   3); /* 500 MHz */
	cmu_clkch_enable_by_base(MMPCIE_CMU0_CLKCHCTL_PCIE2_CFG,  30); /*  50 MHz */
	cmu_clkch_enable_by_base(MMPCIE_CMU0_CLKCHCTL_TCU_CFG,     4); /* 375 MHz */
	cmu_clkch_enable_by_base(MMPCIE_CMU0_CLKCHCTL_TBU0,        4); /* 375 MHz */
	cmu_clkch_enable_by_base(MMPCIE_CMU0_CLKCHCTL_TBU1,        4); /* 375 MHz */
	cmu_clkch_enable_by_base(MMPCIE_CMU0_CLKCHCTL_TBU2,        3); /* 500 MHz */

	/* Deassert reset signals */
	reg = mmio_read_32(MMPCIE_GPR_MMRST);
	reg &= ~(MMPCIE_GPR_MMRST_NIC_CFG_PCIE0_RST |
		 MMPCIE_GPR_MMRST_NIC_CFG_PCIE1_RST |
		 MMPCIE_GPR_MMRST_NIC_CFG_PCIE2_RST |
		 MMPCIE_GPR_MMRST_NIC_CFG_TCU_RST);
	mmio_write_32(MMPCIE_GPR_MMRST, reg);
	reg &= ~MMPCIE_GPR_MMRST_NIC_CFG_SLV_RST;
	mmio_write_32(MMPCIE_GPR_MMRST, reg);
	reg &= ~(MMPCIE_GPR_MMRST_NIC_SLV_PCIE0_RST |
		 MMPCIE_GPR_MMRST_NIC_SLV_PCIE1_RST |
		 MMPCIE_GPR_MMRST_NIC_SLV_PCIE2_RST);
	mmio_write_32(MMPCIE_GPR_MMRST, reg);
	reg &= ~MMPCIE_GPR_MMRST_NIC_SLV_SLV_RST;
	mmio_write_32(MMPCIE_GPR_MMRST, reg);

	/* Set MSI_AWUSER to 0 */
	mmio_clrbits_32(MMPCIE_GPR_MSI_TRANS0,
			MMPCIE_GPR_MSI_TRANS0_MSI_AWUSER_MASK);

	/* Configure SMMU StreamID interface */
	mmio_write_32(MMPCIE_GPR_PCIE0_SID_PROT,
		      MMPCIE_GPR_PCIE_SID_PROT_AWPROT(2) |
		      MMPCIE_GPR_PCIE_SID_PROT_ARPROT(2) |
		      MMPCIE_GPR_PCIE_SID_PROT_WSID_MODE |
		      MMPCIE_GPR_PCIE_SID_PROT_RSID_MODE);

	mmio_write_32(MMPCIE_GPR_PCIE1_SID_PROT,
		      MMPCIE_GPR_PCIE_SID_PROT_AWPROT(2) |
		      MMPCIE_GPR_PCIE_SID_PROT_ARPROT(2) |
		      MMPCIE_GPR_PCIE_SID_PROT_WSID_MODE |
		      MMPCIE_GPR_PCIE_SID_PROT_RSID_MODE);

	mmio_write_32(MMPCIE_GPR_PCIE2_SID_PROT,
		      MMPCIE_GPR_PCIE_SID_PROT_AWPROT(2) |
		      MMPCIE_GPR_PCIE_SID_PROT_ARPROT(2) |
		      MMPCIE_GPR_PCIE_SID_PROT_WSID_MODE |
		      MMPCIE_GPR_PCIE_SID_PROT_RSID_MODE);

	/* Enable non-secure access */
	mmio_write_32(MMPCIE_NIC_CFG_GPV_REGIONSEC_SMMU,  NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMPCIE_NIC_CFG_GPV_REGIONSEC_PCIE0, NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMPCIE_NIC_CFG_GPV_REGIONSEC_PCIE1, NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMPCIE_NIC_CFG_GPV_REGIONSEC_PCIE2, NIC_GPV_REGIONSEC_NONSECURE);

	mmio_write_32(MMPCIE_NIC_GPV_REGIONSEC_PCIE0, NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMPCIE_NIC_GPV_REGIONSEC_PCIE1, NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMPCIE_NIC_GPV_REGIONSEC_PCIE2, NIC_GPV_REGIONSEC_NONSECURE);

	/* Enable non-secure access to some GPRs */
	mmio_write_32(MMPCIE_GPR_S_BASE + 0x00,
		      BIT(0)  | /* MMPCIE_GPR_PCIE0_RST */
		      BIT(1)  | /* MMPCIE_GPR_PCIE0_STS */
		      BIT(2)  | /* MMPCIE_GPR_PCIE0_GEN */
		      BIT(4)  | /* MMPCIE_GPR_PCIE0_PWR */
		      BIT(8)  | /* MMPCIE_GPR_PCIE1_RST */
		      BIT(9)  | /* MMPCIE_GPR_PCIE1_STS */
		      BIT(10) | /* MMPCIE_GPR_PCIE1_GEN */
		      BIT(12) | /* MMPCIE_GPR_PCIE1_PWR */
		      BIT(16) | /* MMPCIE_GPR_PCIE2_RST */
		      BIT(17) | /* MMPCIE_GPR_PCIE2_STS */
		      BIT(18) | /* MMPCIE_GPR_PCIE2_GEN */
		      BIT(20)); /* MMPCIE_GPR_PCIE2_PWR */

	mmio_write_32(MMPCIE_GPR_S_BASE + 0x04,
		      BIT(30)); /* MMPCIE_GPR_MSI_TRANS2 */
}
