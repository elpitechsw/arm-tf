/*
 * Copyright (c) 2022-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <platform_def.h>
#include <baikal_scp.h>
#include <bs1000_cmu.h>
#include <bs1000_def.h>
#include <bs1000_scp_lcru.h>

#include "bs1000_usb.h"

void bs1000_usb_init(void)
{
	uint32_t value;
	unsigned int chip_idx;

	for (chip_idx = 0; chip_idx < PLATFORM_CHIP_COUNT; ++chip_idx) {
		/* Set USB microframe */

		/* 6.1 */
		scp_lcru_setbits(PLATFORM_ADDR_OUT_CHIP(chip_idx, SCP_GPR_MM_RST_CTL1),
				 SCP_GPR_MM_RST_CTL1_USB_HRST);
		scp_lcru_setbits(PLATFORM_ADDR_OUT_CHIP(chip_idx, SCP_GPR_MM_RST_CTL1),
				 SCP_GPR_MM_RST_CTL1_USB_AUX_WELL_RST);
		scp_lcru_setbits(PLATFORM_ADDR_OUT_CHIP(chip_idx, SCP_GPR_MM_RST_CTL1),
				 SCP_GPR_MM_RST_CTL1_USB_UTMI_PHY_RST);
		scp_lcru_setbits(PLATFORM_ADDR_OUT_CHIP(chip_idx, SCP_GPR_MM_RST_CTL1),
				 SCP_GPR_MM_RST_CTL1_OHCI_0_CLKCKT_RST);
		scp_lcru_setbits(PLATFORM_ADDR_OUT_CHIP(chip_idx, SCP_GPR_MM_RST_CTL1),
				 SCP_GPR_MM_RST_CTL1_OHCI_0_CLKDIV_RST);

		/* 6.2 */
		scp_lcru_clrbits(PLATFORM_ADDR_OUT_CHIP(chip_idx, SCP_GPR_MM_RST_CTL1),
				 SCP_GPR_MM_RST_CTL1_OHCI_0_CLKDIV_RST);

		/* 6.3 */
		cmu_clkch_set_rate(PLATFORM_ADDR_OUT_CHIP(chip_idx, 0x4100f0),
				   100 * 1000 * 1000);
		cmu_clkch_set_rate(PLATFORM_ADDR_OUT_CHIP(chip_idx, 0x410100),
				   48 * 1000 * 1000);
		cmu_clkch_enable(PLATFORM_ADDR_OUT_CHIP(chip_idx, 0x4100f0));
		cmu_clkch_enable(PLATFORM_ADDR_OUT_CHIP(chip_idx, 0x410100));

		/* 6.4, set USB FLADJ */
		scp_lcru_read(PLATFORM_ADDR_OUT_CHIP(chip_idx, SCP_GPR_USB_CTL),
			      &value);
		value &= ~SCP_GPR_USB_CTL_FLADJ_VALUE_MASK;
		value &= ~SCP_GPR_USB_CTL_FLADJ_ENABLE_MASK;
		value |= 32 << SCP_GPR_USB_CTL_FLADJ_ENABLE_SHIFT;
		value |= 32 << SCP_GPR_USB_CTL_FLADJ_VALUE_SHIFT;
		scp_lcru_write(PLATFORM_ADDR_OUT_CHIP(chip_idx, SCP_GPR_USB_CTL),
			       value);

		/* 6.5 */
		scp_lcru_clrbits(PLATFORM_ADDR_OUT_CHIP(chip_idx, SCP_GPR_MM_RST_CTL1),
				 SCP_GPR_MM_RST_CTL1_OHCI_0_CLKCKT_RST);
		scp_lcru_clrbits(PLATFORM_ADDR_OUT_CHIP(chip_idx, SCP_GPR_MM_RST_CTL1),
				 SCP_GPR_MM_RST_CTL1_USB_UTMI_PHY_RST);
		scp_lcru_clrbits(PLATFORM_ADDR_OUT_CHIP(chip_idx, SCP_GPR_MM_RST_CTL1),
				 SCP_GPR_MM_RST_CTL1_USB_AUX_WELL_RST);
		mdelay(1);
		scp_lcru_clrbits(PLATFORM_ADDR_OUT_CHIP(chip_idx, SCP_GPR_MM_RST_CTL1),
				 SCP_GPR_MM_RST_CTL1_USB_HRST);
	}
}
