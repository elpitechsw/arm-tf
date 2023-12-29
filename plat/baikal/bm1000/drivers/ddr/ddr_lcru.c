/*
 * Copyright (c) 2021-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <bm1000_cmu.h>
#include <bm1000_def.h>
#include <ndelay.h>

#include "ddr_lcru.h"
#include "ddr_main.h"

#define LCRU_DDR(port)	(MMDDR0_BASE + (port) * (MMDDR1_BASE - MMDDR0_BASE))

static const cmu_pll_ctl_vals_t pll_ddr_1600 = {4, 0, 0xa000000000, 0, -20, 0, -20};
static const cmu_pll_ctl_vals_t pll_ddr_1866 = {2, 0, 0x7000000000, 0, -20, 0, -20};
static const cmu_pll_ctl_vals_t pll_ddr_2133 = {2, 0, 0x8000000000, 0, -20, 0, -20};
static const cmu_pll_ctl_vals_t pll_ddr_2400 = {2, 0, 0x9000000000, 0, -20, 0, -20};
static const cmu_pll_ctl_vals_t pll_ddr_2666 = {2, 0, 0xa000000000, 0, -20, 0, -20};

void ddr_lcru_clkch_rst(int port, uint32_t ra, int set)
{
	if (set) {
		mmio_setbits_32(LCRU_DDR(port) + ra, LCRU_CMU_SWRST);
	} else {
		mmio_clrbits_32(LCRU_DDR(port) + ra, LCRU_CMU_SWRST);
	}
}

void ddr_lcru_dual_mode(int port, int mode)
{
	mmio_write_32(LCRU_DDR(port) + LCRU_DDR_ADDR_CTL, mode);
}

static int baikal_ddrlcru_special(const int port, enum ddr_speed_bin speedbin)
{
	const cmu_pll_ctl_vals_t *pllinit;
	int divisor_50mhz;

	switch (speedbin) {
	case DDR4_1600:
		pllinit = &pll_ddr_1600;
		divisor_50mhz = 8;
		break;
	case DDR4_1866:
		pllinit = &pll_ddr_1866;
		divisor_50mhz = 9;
		break;
	case DDR4_2133:
		pllinit = &pll_ddr_2133;
		divisor_50mhz = 11;
		break;
	case DDR4_2400:
		pllinit = &pll_ddr_2400;
		divisor_50mhz = 12;
		break;
	case DDR4_2666:
		pllinit = &pll_ddr_2666;
		divisor_50mhz = 13;
		break;
	default:
		return -1;
	}

	ddr_lcru_clkch_rst(port, LCRU_DDR_CMU_CLKCH0_ACLK, 1);
	ddr_lcru_clkch_rst(port, LCRU_DDR_CMU_CLKCH1_CORE, 1);

	ndelay(50);

	if (cmu_pll_is_enabled(LCRU_DDR(port))) {
		ddr_lcru_disable(port);
		mdelay(1);
	}

	cmu_pll_on(LCRU_DDR(port), pllinit);

	cmu_clkch_enable_by_base(LCRU_DDR(port) + LCRU_DDR_CMU_CLKCH0_ACLK, 1);
	cmu_clkch_enable_by_base(LCRU_DDR(port) + LCRU_DDR_CMU_CLKCH1_CORE, 1);
	/* hold resets active for core */
	ddr_lcru_clkch_rst(port, LCRU_DDR_CMU_CLKCH0_ACLK, 1);
	ddr_lcru_clkch_rst(port, LCRU_DDR_CMU_CLKCH1_CORE, 1);

	cmu_clkch_enable_by_base(LCRU_DDR(port) + LCRU_DDR_CMU_CLKCH2_TZC_PCLK, divisor_50mhz);
	cmu_clkch_enable_by_base(LCRU_DDR(port) + LCRU_DDR_CMU_CLKCH3_PCLK, divisor_50mhz);

	ndelay(50);

	mmio_write_32(LCRU_DDR(port) + LCRU_DDR_RESET_CFG_NIC, 0);

	/* allow nonsecure access to ddr controller region */
	mmio_write_32(LCRU_DDR(port) + DDR_NIC_CFG_REGIONSEC_CTL_OFFS,
			NIC_GPV_REGIONSEC_NONSECURE);

	mdelay(1);

	return 0;
}

int ddr_lcru_disable(int port)
{
	return cmu_pll_disable(LCRU_DDR(port));
}

int ddr_lcru_initport(int port, uint32_t clock_mhz)
{
	enum ddr_speed_bin speed_bin;

	switch (clock_mhz) {
	case 800:
		speed_bin = DDR4_1600;
		break;
	case 933:
		speed_bin = DDR4_1866;
		break;
	case 1066:
		speed_bin = DDR4_2133;
		break;
	case 1200:
		speed_bin = DDR4_2400;
		break;
	case 1333:
		speed_bin = DDR4_2666;
		break;
	default:
		ERROR("DDR4: unsupported speed bin\n");
		return -1;
	}

	return baikal_ddrlcru_special(port, speed_bin);
}
