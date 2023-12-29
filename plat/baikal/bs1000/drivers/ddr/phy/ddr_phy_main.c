/*
 * Copyright (c) 2022, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include "../ddr_main.h"
#include "ddr_phy_main.h"
#include "ddr_phy_train_struct.h"

#define PHYINIT_SEQUENCENUM	0

int phy_main(int port, struct ddr_configuration *data)
{
	int ret = 0;

	/* (C) Initialize PHY Configuration */
	phyinit_C_PhyConfig(port, data);

	/*
	 * Switches between supported phyinit training sequences refer to
	 * "Alternative PHY Training sequence" document for further details.
	 */

	/* 0x0 | Minimizes number of Imem/Dmem loads (default) */
	/* 0x1 | High frequency P1/P2/P3 support (DDR4/LP4 only) */
#if (PHYINIT_SEQUENCENUM == 0)
	/* Run all 1D power states, then 2D P0, to reduce total Imem/Dmem loads. */
	/* (D) Load the IMEM Memory for 1D training */
	phyinit_D_LoadIMEM(port, data, false);

	/* (E) Set the PHY input clocks to the desired frequency */
	phyinit_E_setDfiClk(port);
	/* Note: this routine implies other items such as DfiFreqRatio, DfiCtlClk are also set properly. */

	/* (F) Write the Message Block parameters for the training firmware */
	struct pmu_smb_ddr4_t msg_block = {0};

	phyinit_calcMb((void *)&msg_block, data, false);

	phyinit_F_LoadDMEM(port, data, &msg_block, false);

	/* (G) Execute the Training Firmware */
	ret = phyinit_G_ExecFW(port);
	if (ret) {
		goto exit;
	}

	/* WARNING: this is time critical section for RDIMM (avoid any delay from PHY training complete to dfi_init_start) */
	/* (H) Read the Message Block results */
	ret = phyinit_H_readMsgBlock(port, data);
	if (ret) {
		goto exit;
	}

	/* Now optionally perform 2D training for protocols that allow it */
	if (data->phy_training_2d) {
		/* Step names here mimic the 1D lettering (E,F,G,H). */
		/* They can be found in the Training Firmware Application Note */

		/* 2D-E Set the PHY input clocks to the highest frequency */
		phyinit_E_setDfiClk(port); /* pstate==0; DfiClk fixed 2:1 ratio with MemClk */

		/* 2D-F */
		phyinit_D_LoadIMEM(port, data, true); /* 2D image */

		/* 2D-F, cont.  Write the Message Block parameters for the training firmware */
		phyinit_calcMb((void *)&msg_block, data, true); /* pstate=0 */

		phyinit_F_LoadDMEM(port, data, &msg_block, true);

		/* 2D-G Execute the Training Firmware */
		ret = phyinit_G_ExecFW(port);
		if (ret) {
			goto exit;
		}
		/* WARNING: this is time critical section for RDIMM (avoid any delay from PHY training complete to dfi_init_start) */
	}
#else
	/* Run 2D P0  after 1D P0 so 1D P1/P2/P3 can be run at the vrefDAC0 settings trained by 2D. See Share2DVrefResult messageblock field. */
	/* (D) Load the IMEM Memory for P0 1D training */
	phyinit_D_LoadIMEM(port, config, false);

	/* (E) Set the PHY input clocks to the desired frequency */
	phyinit_E_setDfiClk(port);
	/* Note: this routine implies other items such as DfiFreqRatio, DfiCtlClk are also set properly. */

	/* (F) Write the Message Block parameters for the training firmware */
	struct pmu_smb_ddr4_t msg_block = {0};

	phyinit_calcMb((void *)&msg_block, data, false);

	phyinit_F_LoadDMEM(port, data, &msg_block, false);

	/* (G) Execute the Training Firmware */
	ret = phyinit_G_ExecFW(port);
	if (ret) {
		goto exit;
	}

	/* (H) Read the Message Block results */
	ret = phyinit_H_readMsgBlock(port, data);
	if (ret) {
		goto exit;
	}

	/* Now optionally perform P0 2D training for protocols that allow it */
	if (data->phy_training_2d) {
		/* Step names here mimic the 1D lettering (E,F,G,H). */
		/* They can be found in the Training Firmware Application Note */

		/* 2D-E Set the PHY input clocks to the highest frequency */
		phyinit_E_setDfiClk(port); /* pstate==0; DfiClk fixed 2:1 ratio with MemClk */

		/* 2D-F */
		phyinit_D_LoadIMEM(port, data, true); /* 2D image */

		/* 2D-F, cont.  Write the Message Block parameters for the training firmware */
		msg_block = {0};
		phyinit_calcMb((void *)&msg_block, data, true); /* pstate=0 */

		phyinit_F_LoadDMEM(port, data, &msg_block, true);

		/* 2D-G Execute the Training Firmware */
		ret = phyinit_G_ExecFW(port);
		if (ret) {
			goto exit;
		}

		/* (D) Reload the IMEM Memory for P1,P2,and P3 1D */
		phyinit_D_LoadIMEM(port, data, false);
	}
#endif
	/* WARNING: this is time critical section for RDIMM (avoid any delay from PHY training complete to dfi_init_start) */

	/* (I) Load PHY Init Engine Image */
	phyinit_I_LoadPIE(port, data);

	/* WARNING: this is time critical section for RDIMM (avoid any delay from PHY training complete to dfi_init_start) */

	/* Note: perform dfi initialization by umctl2_CompletePhyInit()
	 *
	 * Note: we don't touch DFIPHYUPD register now and assume DFI PHY Update request mode
	 * to be switch on by default (timeout 64K clocks by default)
	 */
exit:
	/* WARNING: this is time critical section for RDIMM (avoid any delay from PHY training complete to dfi_init_start) */
	return ret;
}
