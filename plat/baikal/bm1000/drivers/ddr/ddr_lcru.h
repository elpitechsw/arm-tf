/*
 * Copyright (c) 2018-2021, Baikal Electronics, JSC. All rights reserved.
 *
 * Author: Alexey Malahov <Alexey.Malahov@baikalelectronics.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DDR_LCRU_H
#define DDR_LCRU_H

#include <stdint.h>

#include <arch_helpers.h>

/* LCRU DDR CMU registers */
#define LCRU_DDR_CMU_CLKCH0_ACLK	0x0020
#define LCRU_DDR_CMU_CLKCH1_CORE	0x0030
#define LCRU_DDR_CMU_CLKCH2_TZC_PCLK	0x0040
#define LCRU_DDR_CMU_CLKCH3_PCLK	0x0050

/* LCRU DDR GPR registers */
#define LCRU_DDR_RESET_CFG_NIC		0x50000 /* GPR0_RW */
#define LCRU_DDR_ACLK_LPI_REQ		0x50008 /* GPR1_RW */
#define LCRU_DDR_ACLK_LPI_STS		0x5000c /* GPR1_RO */
#define LCRU_DDR_CORECLK_LPI_REQ	0x50010 /* GPR2_RW */
#define LCRU_DDR_CORECLK_LPI_STS	0x50014 /* GPR2_RO */
#define LCRU_DDR_GPR3_REQ		0x50018 /* GPR3_RW */
#define LCRU_DDR_GPR4_REQ		0x50020 /* GPR4_RW */
#define LCRU_DDR_AXI_PORT0_STS		0x5002c /* GPR5_RO */
#define LCRU_DDR_ADDR_CTL		0x50038 /* GPR7_RW */
#define LCRU_DDR_PHY_CTL		0x50048 /* GPR9_RW */

/* CMU_CLKCHn_CTL register bit assignments */
#define LCRU_CMU_CLK_EN			BIT(0)
#define LCRU_CMU_SWRST			BIT(1)
#define LCRU_CMU_SET_CLKDIV		BIT(2)
#define LCRU_CMU_VAL_CLKDIV		BIT(4)
#define LCRU_CMU_CLK_RDY		BIT(30)
#define LCRU_CMU_LOCK_CLKDIV		BIT(31)

#define DDR_NIC_CFG_GPV_BASE_OFFS	0x100000
#define DDR_NIC_CFG_REGIONSEC_CTL_OFFS	0x100008
#define DDR_NIC_CFG_REGIONSEC_PHY_OFFS	0x100000

void ddr_lcru_dual_mode(int port, int mode);
void ddr_lcru_clkch_rst(int port, uint32_t ra, int set);
int ddr_lcru_disable(int port);

#endif /* DDR_LCRU_H */
