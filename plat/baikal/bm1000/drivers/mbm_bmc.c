/*
 * Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>

#include <baikal_def.h>
#include <dw_i2c.h>
#include <mbm_bmc.h>

#define MBM_BMC_I2C_BUS			MMAVLSP_I2C1_BASE
#define MBM_BMC_I2C_ADDR		0x08
#define MBM_BMC_REG_PWROFF_RQ		0x05
#define MBM_BMC_REG_PWROFF_RQ_OFF	0x01
#define MBM_BMC_REG_PWROFF_RQ_RESET	0x02

void mbm_bmc_pwr_off(void)
{
	const uint8_t offreq[] = {
		MBM_BMC_REG_PWROFF_RQ,
		MBM_BMC_REG_PWROFF_RQ_OFF
	};

	INFO("BMC: power off\n");
	i2c_txrx(MBM_BMC_I2C_BUS, BAIKAL_I2C_ICLK_FREQ,
		 MBM_BMC_I2C_ADDR, &offreq, sizeof(offreq), NULL, 0);

	mdelay(4000);
}

void mbm_bmc_pwr_rst(void)
{
	const uint8_t rstreq[] = {
		MBM_BMC_REG_PWROFF_RQ,
		MBM_BMC_REG_PWROFF_RQ_RESET
	};

	INFO("BMC: power reset\n");
	i2c_txrx(MBM_BMC_I2C_BUS, BAIKAL_I2C_ICLK_FREQ,
		 MBM_BMC_I2C_ADDR, &rstreq, sizeof(rstreq), NULL, 0);

	mdelay(4000);
}
