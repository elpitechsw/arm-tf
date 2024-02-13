/*
 * Copyright (c) 2022-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <string.h>

#include "../ddr_main.h"
#include "../ddr_master.h"
#include "ddr_phy_misc.h"
#include "ddr_phy_train_struct.h"
#include "ddr_phy_tmp_regs.h"

#define PSTATE_MAX	4

#define DDRPHY_TRAIN_1D_MASK	0x31f
#define DDRPHY_TRAIN_2D_MASK	0x61

#define TRAINOPT_DAC	0x0
#define RD2D_V_RANGE	0x0
#define WR2D_V_RANGE	0x0

/* initialize Message Block sram data structure for PMU firmware */
void phyinit_calcMb(void *mb, struct ddr_configuration *data, int training_2d)
{
	struct pmu_smb_ddr4_t *pmu_smb_p = (struct pmu_smb_ddr4_t *)mb;

	memset(pmu_smb_p, 0, sizeof(struct pmu_smb_ddr4_t));

	pmu_smb_p->Pstate = 0x0;
	pmu_smb_p->PhyConfigOverride = 0x0;
	pmu_smb_p->HdtCtrl = 0xff;
	pmu_smb_p->MsgMisc = 0x0;
	pmu_smb_p->Reserved00 = 0; /* Reserved00[5] = Train vrefDAC0 During Read Deskew */
	pmu_smb_p->DFIMRLMargin = 0x1; /* 1 is typically good in DDR3 */
	/* Use Analytical VREF and Compensate for T28 Attenuator, see PHY databook */
	pmu_smb_p->PhyVref = data->HOST_VREF;

	pmu_smb_p->CsPresentD0 = (data->ranks == 1) ? 0x1 : 0x3;
	if (data->dimms == 2) {
		pmu_smb_p->CsPresentD1 = pmu_smb_p->CsPresentD0 << 2;
	} else {
		pmu_smb_p->CsPresentD1 = 0x0;
	}
	pmu_smb_p->CsPresent = pmu_smb_p->CsPresentD0 | pmu_smb_p->CsPresentD1;
	/* Set AddrMirror if CS[pstate] is mirrored. (typically odd CS are mirroed in DIMMs) */
	pmu_smb_p->AddrMirror = data->mirrored_dimm ? 0xa : 0;

	pmu_smb_p->AcsmOdtCtrl0 = data->odt_map & 0xff;
	pmu_smb_p->AcsmOdtCtrl1 = (data->odt_map >> 8) & 0xff;
	pmu_smb_p->AcsmOdtCtrl2 = (data->odt_map >> 16) & 0xff;
	pmu_smb_p->AcsmOdtCtrl3 = (data->odt_map >> 24) & 0xff;

	pmu_smb_p->EnabledDQs = (data->ecc_on ? 9 : 8) * 8;
	pmu_smb_p->PhyCfg = data->timing_2t;
	pmu_smb_p->X16Present = 0x0;
	pmu_smb_p->D4Misc = 0x1; /* Protect memory reset: */
	pmu_smb_p->CsSetupGDDec = 0x1; /* If Geardown is chosen, dynamically modify CS timing */

	/* Per Rank MR seeting for RTT_NOM, RTT_WR, RTT_PARK per rank. Options unlikely need to be used. */
	/* See MB details on how to program if required. */
	pmu_smb_p->RTT_NOM_WR_PARK0 = 0x0;
	pmu_smb_p->RTT_NOM_WR_PARK1 = 0x0;
	pmu_smb_p->RTT_NOM_WR_PARK2 = 0x0;
	pmu_smb_p->RTT_NOM_WR_PARK3 = 0x0;
	pmu_smb_p->RTT_NOM_WR_PARK4 = 0x0;
	pmu_smb_p->RTT_NOM_WR_PARK5 = 0x0;
	pmu_smb_p->RTT_NOM_WR_PARK6 = 0x0;
	pmu_smb_p->RTT_NOM_WR_PARK7 = 0x0;
	pmu_smb_p->MR0 = set_mr0(data->CL, data->tWR);
	pmu_smb_p->MR1 = set_mr1(data->DIC, data->RTT_NOM);
	pmu_smb_p->MR2 = set_mr2(data->CWL, data->RTT_WR, data->crc_on);
	pmu_smb_p->MR3 = set_mr3(data->WCL, 0);
	pmu_smb_p->MR4 = set_mr4(data->wr_preamble_2CK, data->rd_preamble_2CK);
	pmu_smb_p->MR5 = set_mr5(data->PL, data->RTT_PARK, 0);
	pmu_smb_p->MR6 = set_mr6(data->tCCD_L, data->DRAM_VREF);

	data->pocket.MR5 = pmu_smb_p->MR5; /* saving for later use */

	pmu_smb_p->ALT_CAS_L = 0x0; /* Need to set if using RDDBI */
	if (data->wr_preamble_2CK) {
		pmu_smb_p->ALT_WCAS_L = (pmu_smb_p->MR2 & (0x7 << 3)) | 0x1;
	} else {
		pmu_smb_p->ALT_WCAS_L = 0;
	}

	/* If Share2DVrefResult[x] = 1, pstate x will use the per-lane VrefDAC0/1 CSRs which can be trained by 2d training. */
	/* If 2D has not run yet, VrefDAC0/1 will default to pstate 0's 1D phyVref messageBlock setting. */
	/*
	 * If Share2DVrefResult[x] = 0, pstate x will use the per-phy VrefInGlobal CSR,
	 * which are set to pstate x's 1D phyVref messageBlock setting.
	 */
	pmu_smb_p->Share2DVrefResult = 0x1; /* Bitmap that controls which vref generator the phy will use per pstate */

	if (training_2d) {
		/* DFE on/off, Voltage Step Size=1 DAC setting, LCDL Delay Step Size=1 LCDL delay between checked values. */
		pmu_smb_p->RX2D_TrainOpt = ((TRAINOPT_DAC << 1) & 0x3) | (data->phy_eql & PHY_EQL_DFE) ? 1 : 0;
		/* FFE on/off, Voltage Step Size=1 DAC setting, LCDL Delay Step Size=1 LCDL delay between checked values. */
		pmu_smb_p->TX2D_TrainOpt = ((TRAINOPT_DAC << 1) & 0x3) | (data->phy_eql & PHY_EQL_FFE) ? 1 : 0;
		pmu_smb_p->Delay_Weight2D = 0x20; /* Evenly weigh Delay vs Voltage */
		pmu_smb_p->Voltage_Weight2D = 0x80;
		/* Input for constraining the range */
		/* of vref(DQ) values training will collect data for, usually reducing training time. */
		pmu_smb_p->Reserved1E = ((WR2D_V_RANGE & 0xf) << 4) | ((RD2D_V_RANGE & 0xf) << 0);

		pmu_smb_p->SequenceCtrl = DDRPHY_TRAIN_2D_MASK;
	} else {
		pmu_smb_p->SequenceCtrl = DDRPHY_TRAIN_1D_MASK;
	}
	/* Memory controller may set CAL mode after PHY has entered mission mode. */
	pmu_smb_p->DramType = data->registered_dimm ? 0x4 : 0x2; /* DDR4 RDIMM */
	pmu_smb_p->DRAMFreq = data->clock_mhz * 2;
	pmu_smb_p->PllBypassEn = 0x0;
	pmu_smb_p->DfiFreqRatio = 1 << 1;

	/* note: we program ODT	impedance manually in phyinit_C_PhyConfig() before run training */
	pmu_smb_p->PhyOdtImpedance = 0;
	/* note: we program driver impedance manually in phyinit_C_PhyConfig() before run training */
	pmu_smb_p->PhyDrvImpedance = 0;
	pmu_smb_p->BPZNResVal = 0;
	pmu_smb_p->DisabledDbyte = 0;

	if (data->registered_dimm) {
		/* Only content of the Control Word is required to be programmed. */
		uint16_t tmp = 0;
		/* F0RC0D */
		if (data->mirrored_dimm) {
			pmu_smb_p->F0RC0D_D0 = 0x4 | (pmu_smb_p->CsPresentD0) ? 0x8 : 0x0;
			pmu_smb_p->F0RC0D_D1 = 0x4 | (pmu_smb_p->CsPresentD1) ? 0x8 : 0x0;
		} else {
			pmu_smb_p->F0RC0D_D0 = 0x4;
			pmu_smb_p->F0RC0D_D1 = 0x4;
		}
		/* F0RC0A DIMM Operating Speed. Implement Table 42 of RCD spec */
		switch (data->clock_mhz) {
		case 800:
			tmp = 0;
			break;
		case 933:
			tmp = 1;
			break;
		case 1066:
			tmp = 2;
			break;
		case 1200:
			tmp = 3;
			break;
		case 1333:
			tmp = 4;
			break;
		case 1466:
			tmp = 5;
			break;
		case 1600:
			tmp = 6;
			break;
		default:
			break;
		}
		pmu_smb_p->F0RC0A_D0 = tmp;
		pmu_smb_p->F0RC0A_D1 = tmp;
		data->pocket.F0RC0A_D0 = tmp; /* saving for later use */
		/* F0RC3x Fine Granularity RDIMM Operating Speed */
		tmp = 0xff & ((data->clock_mhz - 621) / 10);
		pmu_smb_p->F0RC3x_D0 = tmp;
		pmu_smb_p->F0RC3x_D1 = tmp;
		data->pocket.F0RC3x_D0 = tmp; /* saving for later use */
		/* F0RC03 CA/CS driver strength */
		tmp = (data->registered_ca_stren >> 4) & 0x3;
		tmp |= ((data->registered_ca_stren >> 6) & 0x3) << 2;
		pmu_smb_p->F0RC03_D0 = tmp;
		pmu_smb_p->F0RC03_D1 = tmp;
		/* F0RC04 ODT/CKE driver strength */
		tmp = (data->registered_ca_stren) & 0x3;
		tmp |= ((data->registered_ca_stren >> 2) & 0x3) << 2;
		pmu_smb_p->F0RC04_D0 = tmp;
		pmu_smb_p->F0RC04_D1 = tmp;
		/* F0RC05 clocks driver strength */
		tmp = (data->registered_clk_stren) & 0x3;
		tmp |= ((data->registered_clk_stren >> 2) & 0x3) << 2;
		pmu_smb_p->F0RC05_D0 = tmp;
		pmu_smb_p->F0RC05_D1 = tmp;
	}
}
