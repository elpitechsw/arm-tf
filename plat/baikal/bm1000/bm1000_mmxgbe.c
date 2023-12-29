/*
 * Copyright (c) 2018-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <baikal_def.h>
#include <bm1000_cmu.h>
#include <bm1000_private.h>
#include <dw_gpio.h>

void mmxgbe_init(void)
{
	const cmu_pll_ctl_vals_t mmxgbe_cmu0_pll_ctls = {
		0, 0,    0x6400000000, 0, -20, 0, -20
	};

	mmio_clrbits_32(MMXGBE_GPR_MMRST,
			MMXGBE_GPR_MMRST_NICCFGS |
			MMXGBE_GPR_MMRST_NICCFGM |
			MMXGBE_GPR_MMRST_NICFNCS |
			MMXGBE_GPR_MMRST_NICFNCM);

#ifdef BAIKAL_HDMI_CLKEN_GPIO_PIN
	gpio_out_rst(MMAVLSP_GPIO32_BASE, BAIKAL_HDMI_CLKEN_GPIO_PIN);
	gpio_dir_set(MMAVLSP_GPIO32_BASE, BAIKAL_HDMI_CLKEN_GPIO_PIN);
#endif

	cmu_pll_on(MMXGBE_CMU0_BASE, &mmxgbe_cmu0_pll_ctls);
	cmu_clkch_enable_by_base(MMXGBE_CMU0_CLKCHCTL_CSR0,	       25); /*  50.0 MHz */
	cmu_clkch_enable_by_base(MMXGBE_CMU0_CLKCHCTL_CSR1,	       12); /* 104.0 MHz */
	cmu_clkch_enable_by_base(MMXGBE_CMU0_CLKCHCTL_XGBE0_REF,        8); /* 156.0 MHz */
	cmu_clkch_enable_by_base(MMXGBE_CMU0_CLKCHCTL_XGBE0_AXI,        5); /* 250.0 MHz */
	cmu_clkch_enable_by_base(MMXGBE_CMU0_CLKCHCTL_XGBE0_PTP,        8); /* 156.0 MHz */
	cmu_clkch_enable_by_base(MMXGBE_CMU0_CLKCHCTL_XGBE1_REF,        8); /* 156.0 MHz */
	cmu_clkch_enable_by_base(MMXGBE_CMU0_CLKCHCTL_XGBE1_AXI,        5); /* 250.0 MHz */
	cmu_clkch_enable_by_base(MMXGBE_CMU0_CLKCHCTL_XGBE1_PTP,        8); /* 156.0 MHz */
	cmu_clkch_enable_by_base(MMXGBE_CMU0_CLKCHCTL_GMAC0_AXI,        5); /* 250.0 MHz */
	cmu_clkch_enable_by_base(MMXGBE_CMU0_CLKCHCTL_GMAC0_PTP,       10); /* 125.0 MHz */
	cmu_clkch_enable_by_base(MMXGBE_CMU0_CLKCHCTL_GMAC0_TX2,        5); /* 250.0 MHz */
	cmu_clkch_enable_by_base(MMXGBE_CMU0_CLKCHCTL_GMAC1_AXI,        5); /* 250.0 MHz */
	cmu_clkch_enable_by_base(MMXGBE_CMU0_CLKCHCTL_GMAC1_PTP,       10); /* 125.0 MHz */
	cmu_clkch_enable_by_base(MMXGBE_CMU0_CLKCHCTL_GMAC1_TX2,        5); /* 250.0 MHz */
	cmu_clkch_enable_by_base(MMXGBE_CMU0_CLKCHCTL_SMMU,             3); /* 416.7 MHz */
	cmu_clkch_enable_by_base(MMXGBE_CMU0_CLKCHCTL_HDMI_VDU_ACLK,    2); /* 625.0 MHz */
	cmu_clkch_enable_by_base(MMXGBE_CMU0_CLKCHCTL_HDMI_AUDIO_ACLK, 10); /* 125.0 MHz */
	cmu_clkch_enable_by_base(MMXGBE_CMU0_CLKCHCTL_HDMI_SFR0,       50); /*  25.0 MHz */

	/* Deassert XGMAC resets */
	mmio_clrbits_32(MMXGBE_GPR_MMRST,
			MMXGBE_GPR_MMRST_XGBE0PWRON |
			MMXGBE_GPR_MMRST_XGBE1PWRON |
			MMXGBE_GPR_MMRST_HDMI_PWRON);

	/* GMAC - Domain 2, cached */
	mmio_write_32(MMXGBE_GPR_GMAC0_CACHE_CTL,
		      MMXGBE_GPR_GMAC_CACHE_CTL_ARDOMAIN(2)  |
		      MMXGBE_GPR_GMAC_CACHE_CTL_AWDOMAIN(2)  |
		      MMXGBE_GPR_GMAC_CACHE_CTL_ARCACHE(0xb) |
		      MMXGBE_GPR_GMAC_CACHE_CTL_AWCACHE(0x7));

	mmio_write_32(MMXGBE_GPR_GMAC1_CACHE_CTL,
		      MMXGBE_GPR_GMAC_CACHE_CTL_ARDOMAIN(2)  |
		      MMXGBE_GPR_GMAC_CACHE_CTL_AWDOMAIN(2)  |
		      MMXGBE_GPR_GMAC_CACHE_CTL_ARCACHE(0xb) |
		      MMXGBE_GPR_GMAC_CACHE_CTL_AWCACHE(0x7));

	/* VDU, HDMI */
	mmio_write_32(MMXGBE_GPR_HDMI_VIDEO_CACHE_CTL,
		      MMXGBE_GPR_HDMI_VIDEO_CACHE_CTL_ARDOMAIN(2) |
		      MMXGBE_GPR_HDMI_VIDEO_CACHE_CTL_ARCACHE(0xb));

	mmio_write_32(MMXGBE_GPR_HDMI_VIDEO_QOS_CTL,
		      MMXGBE_GPR_HDMI_VIDEO_QOS_CTL_ARQOS(0xf));

	mmio_write_32(MMXGBE_GPR_HDMI_AUDIO_CACHE_CTL,
		      MMXGBE_GPR_HDMI_AUDIO_CACHE_CTL_ARDOMAIN(2) |
		      MMXGBE_GPR_HDMI_AUDIO_CACHE_CTL_ARCACHE(0xb));
}

void mmxgbe_ns_access(void)
{
	mmio_write_32(MMXGBE_GPR_GMAC0_PROT_CTL,
		      MMXGBE_GPR_GMAC_PROT_CTL_AWPROT(2) |
		      MMXGBE_GPR_GMAC_PROT_CTL_ARPROT(2));

	mmio_write_32(MMXGBE_GPR_GMAC1_PROT_CTL,
		      MMXGBE_GPR_GMAC_PROT_CTL_AWPROT(2) |
		      MMXGBE_GPR_GMAC_PROT_CTL_ARPROT(2));

	mmio_write_32(MMXGBE_GPR_XGBE0_PROT_CTL,
		      MMXGBE_GPR_XGBE_PROT_CTL_AWPROT(2) |
		      MMXGBE_GPR_XGBE_PROT_CTL_ARPROT(2));

	mmio_write_32(MMXGBE_GPR_XGBE1_PROT_CTL,
		      MMXGBE_GPR_XGBE_PROT_CTL_AWPROT(2) |
		      MMXGBE_GPR_XGBE_PROT_CTL_ARPROT(2));

	mmio_write_32(MMXGBE_GPR_HDMI_VIDEO_PROT_CTL,
		      MMXGBE_GPR_HDMI_VIDEO_PROT_CTL_ARPROT(2));

	mmio_write_32(MMXGBE_GPR_HDMI_AUDIO_PROT_CTL,
		      MMXGBE_GPR_HDMI_AUDIO_PROT_CTL_ARPROT(2));

	mmio_write_32(MMXGBE_NIC_CFG_GPV_REGIONSEC_VDU,	       NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMXGBE_NIC_CFG_GPV_REGIONSEC_XGBE0_CTRL, NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMXGBE_NIC_CFG_GPV_REGIONSEC_XGBE0_PHY,  NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMXGBE_NIC_CFG_GPV_REGIONSEC_XGBE1_CTRL, NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMXGBE_NIC_CFG_GPV_REGIONSEC_XGBE1_PHY,  NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMXGBE_NIC_CFG_GPV_REGIONSEC_GMAC0,      NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMXGBE_NIC_CFG_GPV_REGIONSEC_GMAC1,      NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMXGBE_NIC_CFG_GPV_REGIONSEC_SMMU,       NIC_GPV_REGIONSEC_NONSECURE);
	mmio_write_32(MMXGBE_NIC_CFG_GPV_REGIONSEC_HDMI,       NIC_GPV_REGIONSEC_NONSECURE);
}
