/*
 * Copyright (c) 2018-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <bm1000_cmu.h>
#include <bm1000_def.h>
#include <bm1000_private.h>

#define MMUSB_CLK_50MHZ		 16
#define MMUSB_CLK_100MHZ	  8
#define MMUSB_CLK_133MHZ	  6
#define MMUSB_CLK_160MHZ	  5
#define MMUSB_CLK_200MHZ	  4
#define MMUSB_CLK_400MHZ	  2

void mmusb_init(void)
{
	uint32_t reg;
	const cmu_pll_ctl_vals_t mmusb_cmu0_pll_ctls = {
		1, 0, 0x8000000000, 0, -20, 0, -20 /* 800 MHz */
	};

	cmu_pll_on(MMUSB_CMU0_BASE, &mmusb_cmu0_pll_ctls);

	cmu_clkch_enable_by_base(MMUSB_CMU0_CLKCHCTL_SATA_REF,       MMUSB_CLK_100MHZ); /* 100 MHz axi clk controller #1 (50 - 100 MHz) */
	cmu_clkch_enable_by_base(MMUSB_CMU0_CLKCHCTL_SATA0_AXI,      MMUSB_CLK_100MHZ); /* 100 MHz differential reference clock */
	cmu_clkch_enable_by_base(MMUSB_CMU0_CLKCHCTL_SATA1_AXI,      MMUSB_CLK_100MHZ); /* 100 MHz axi clk controller #0 (50 - 100 MHz) */
	cmu_clkch_enable_by_base(MMUSB_CMU0_CLKCHCTL_USB2_PHY0_REF,  MMUSB_CLK_50MHZ);  /*  50 MHz max reference PHY #0 clock */
	cmu_clkch_enable_by_base(MMUSB_CMU0_CLKCHCTL_USB2_PHY1_REF,  MMUSB_CLK_50MHZ);  /*  50 MHz max reference PHY #1 clock */
	cmu_clkch_enable_by_base(MMUSB_CMU0_CLKCHCTL_USB2_AXI,       MMUSB_CLK_50MHZ);  /*  50 MHz axi clock */
	cmu_clkch_enable_by_base(MMUSB_CMU0_CLKCHCTL_USB2_SOFITP,    MMUSB_CLK_100MHZ); /* 100 MHz reference clock for SOF and ITP counter (16.129 - 125 MHz) */
	cmu_clkch_enable_by_base(MMUSB_CMU0_CLKCHCTL_USB3_PHY0_REF,  MMUSB_CLK_100MHZ); /* 100 MHz differential reference clock PHY #0 */
	cmu_clkch_enable_by_base(MMUSB_CMU0_CLKCHCTL_USB3_PHY1_REF,  MMUSB_CLK_100MHZ); /* 100 MHz differential reference clock PHY #1 */
	cmu_clkch_enable_by_base(MMUSB_CMU0_CLKCHCTL_USB3_PHY2_REF,  MMUSB_CLK_50MHZ);  /*  50 MHz max reference PHY #2 clock */
	cmu_clkch_enable_by_base(MMUSB_CMU0_CLKCHCTL_USB3_PHY3_REF,  MMUSB_CLK_50MHZ);  /*  50 MHz max reference PHY #3 clock */
	cmu_clkch_enable_by_base(MMUSB_CMU0_CLKCHCTL_USB3_AXI,       MMUSB_CLK_133MHZ); /* 133 MHz axi clock */
	cmu_clkch_enable_by_base(MMUSB_CMU0_CLKCHCTL_USB3_SOFITP,    MMUSB_CLK_100MHZ); /* 100 MHz reference clock for SOF and ITP counter (16.129 - 125 MHz) */
	cmu_clkch_enable_by_base(MMUSB_CMU0_CLKCHCTL_USB3_SUSPEND,   MMUSB_CLK_100MHZ); /* 100 MHz suspend clock for low power state (P3) (32 kHz - 125 MHz) */
	cmu_clkch_enable_by_base(MMUSB_CMU0_CLKCHCTL_SMMU,           MMUSB_CLK_400MHZ); /* 400 MHz axi clk (200 - 400 MHz) */
	cmu_clkch_enable_by_base(MMUSB_CMU0_CLKCHCTL_DMACM2M,        MMUSB_CLK_200MHZ); /* 200 MHz - temp value (250 MHz axi clk (100 - 250 MHz)) */
	cmu_clkch_enable_by_base(MMUSB_CMU0_CLKCHCTL_GIC,            MMUSB_CLK_160MHZ); /* 160 MHz */

	/* SATA clocks */
	reg = mmio_read_32(MMUSB_GPR_SATAPHY_CLK);
	reg &= ~(MMUSB_GPR_SATAPHY_CLK_REFSSPEN	|
		 MMUSB_GPR_SATAPHY_CLK_CGMEN	|
		 MMUSB_GPR_SATAPHY_CLK_CGPEN);
	reg |=	 MMUSB_GPR_SATAPHY_CLK_REFPAD;
	mmio_write_32(MMUSB_GPR_SATAPHY_CLK, reg);
	udelay(1);
	reg |=	 MMUSB_GPR_SATAPHY_CLK_REFSSPEN;
	mmio_write_32(MMUSB_GPR_SATAPHY_CLK, reg);

	/* USB3 PHY clocks */
	reg = mmio_read_32(MMUSB_GPR_USB3PHY_SS0_CTL0);
	reg &= ~(MMUSB_GPR_USB3PHY_SS_CTL0_CGMEN |
		 MMUSB_GPR_USB3PHY_SS_CTL0_CGPEN);
	reg |=	 MMUSB_GPR_USB3PHY_SS_CTL0_REFPAD;
	mmio_write_32(MMUSB_GPR_USB3PHY_SS0_CTL0, reg);
	udelay(1);
	reg |=	 MMUSB_GPR_USB3PHY_SS_CTL0_REFSSPEN;
	mmio_write_32(MMUSB_GPR_USB3PHY_SS0_CTL0, reg);

	reg = mmio_read_32(MMUSB_GPR_USB3PHY_SS1_CTL0);
	reg &= ~(MMUSB_GPR_USB3PHY_SS_CTL0_CGMEN |
		 MMUSB_GPR_USB3PHY_SS_CTL0_CGPEN);
	reg |=	 MMUSB_GPR_USB3PHY_SS_CTL0_REFPAD;
	mmio_write_32(MMUSB_GPR_USB3PHY_SS1_CTL0, reg);
	udelay(1);
	reg |=	 MMUSB_GPR_USB3PHY_SS_CTL0_REFSSPEN;
	mmio_write_32(MMUSB_GPR_USB3PHY_SS1_CTL0, reg);

	/* USB2 PHY clocks */
	mmio_clrsetbits_32(MMUSB_GPR_USB2PHY_HS_CTL,
			   MMUSB_GPR_USB2PHY_HS_CTL_PHS0RCLK_MASK |
			   MMUSB_GPR_USB2PHY_HS_CTL_PHS0FSEL_MASK |
			   MMUSB_GPR_USB2PHY_HS_CTL_PHS1RCLK_MASK |
			   MMUSB_GPR_USB2PHY_HS_CTL_PHS1FSEL_MASK,
			   MMUSB_GPR_USB2PHY_HS_CTL_PHS0RCLK(USB2PHY_PHSRCLK_CLKCORE) |
			   MMUSB_GPR_USB2PHY_HS_CTL_PHS0FSEL(USB2PHY_PHSFSEL_50MHZ)   |
			   MMUSB_GPR_USB2PHY_HS_CTL_PHS1RCLK(USB2PHY_PHSRCLK_CLKCORE) |
			   MMUSB_GPR_USB2PHY_HS_CTL_PHS1FSEL(USB2PHY_PHSFSEL_50MHZ));

	/* Enable DMAC-M2M non-secure manager thread */
	mmio_setbits_32(MMUSB_GPR_DMACM2M_BOOT_MNGR_NS,
			MMUSB_GPR_DMACM2M_BOOT_MNGR_NS_MNGNS);

	/* Enable DMAC-M2M non-secure interrupts */
	mmio_setbits_32(MMUSB_GPR_DMACM2M_IRQ_NS,
			MMUSB_GPR_DMACM2M_IRQ_NS_IRQNS_MASK);

	/* Deassert reset signals */
	reg = mmio_read_32(MMUSB_GPR_MMRST);
	reg &= ~(MMUSB_GPR_MMRST_NICCFGS |
		 MMUSB_GPR_MMRST_NICCFGM |
		 MMUSB_GPR_MMRST_NICFNCS |
		 MMUSB_GPR_MMRST_NICFNCM |
		 MMUSB_GPR_MMRST_USB2	 |
		 MMUSB_GPR_MMRST_USB3	 |
		 MMUSB_GPR_MMRST_SATAPHY |
		 MMUSB_GPR_MMRST_SATA0	 |
		 MMUSB_GPR_MMRST_SATA1	 |
		 MMUSB_GPR_MMRST_DMACM2M |
		 MMUSB_GPR_MMRST_GIC	 |
		 MMUSB_GPR_MMRST_SMMU);
	mmio_write_32(MMUSB_GPR_MMRST, reg);
	udelay(50);
	reg &= ~(MMUSB_GPR_MMRST_USB2PHY0 |
		 MMUSB_GPR_MMRST_USB2PHY1 |
		 MMUSB_GPR_MMRST_USB3PHY0 |
		 MMUSB_GPR_MMRST_USB3PHY1 |
		 MMUSB_GPR_MMRST_USB3PHY2 |
		 MMUSB_GPR_MMRST_USB3PHY3);
	mmio_write_32(MMUSB_GPR_MMRST, reg);

	/* Enable non-secure access */
	mmio_write_32(MMUSB_NIC_CFG_GPV_REGIONSEC_USB2,	    NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMUSB_NIC_CFG_GPV_REGIONSEC_USB3,	    NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMUSB_NIC_CFG_GPV_REGIONSEC_SATA0,    NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMUSB_NIC_CFG_GPV_REGIONSEC_SATA1,    NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMUSB_NIC_CFG_GPV_REGIONSEC_SMMU,	    NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMUSB_NIC_CFG_GPV_REGIONSEC_GIC,	    NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMUSB_NIC_CFG_GPV_REGIONSEC_DMACM2MN, NIC_GPV_REGIONSEC_NONSECURE);

	/* CHC */
	mmio_write_32(MMUSB_GPR_SATA0_AXI_SB, 0xa0000);  /* ARDOMAIN 2 / AWDOMAIN 2 (0xa0000 = Coherent ReadOnce + WriteUnique) */
	mmio_write_32(MMUSB_GPR_SATA0_AXI,    0xb70012); /* ARCACHE 7, AWCACHE 7, AxPROT = 2 */

	mmio_write_32(MMUSB_GPR_SATA1_AXI_SB, 0xa0000);  /* ARDOMAIN 2 / AWDOMAIN 2 (0xa0000 = Coherent ReadOnce + WriteUnique) */
	mmio_write_32(MMUSB_GPR_SATA1_AXI,    0xb70012); /* ARCACHE 7, AWCACHE 7, AxPROT = 2 */

	mmio_write_32(MMUSB_GPR_USB2_AXI_SB,  0xa0000);  /* ARDOMAIN 2 / AWDOMAIN 2 (0xa0000 = Coherent ReadOnce + WriteUnique) */
	mmio_write_32(MMUSB_GPR_USB2_AXI,     0x000012); /* AxPROT = 2 */

	mmio_write_32(MMUSB_GPR_USB3_AXI_SB,  0xa0000);  /* ARDOMAIN 2 / AWDOMAIN 2 (0xa0000 = Coherent ReadOnce + WriteUnique) */
	mmio_write_32(MMUSB_GPR_USB3_AXI,     0x000012); /* AxPROT = 2 */
}
