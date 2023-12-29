/*
 * Copyright (c) 2022-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include "../ddr_main.h"
#include "ddr_phy_tmp_regs.h"
#include "ddr_phy_train_struct.h"

#define DDRPHY_PMU_ICCM_ADDR	0x00050000
#define DDRPHY_PMU_ICCM_SIZE	(32 * 1024)
#define DDRPHY_PMU_DCCM_ADDR	0x00054000
#define DDRPHY_PMU_DCCM_SIZE	(16 * 1024)

/* Enable Fast Frequency Change (FFC) Optimizations specific to Synopsys UMCTL2: */
#define SNPS_UMCTLOPT		0
/* F0RX5x RCD Control Word when using Fast Frequency Change(FFC) optimizations specific to Synopsys UMCTL2: */
#define SNPS_UMCTL_F0RC5x	0x0

void phyinit_LoadPieProdCode_udimm(int port);
void phyinit_LoadPieProdCode_rdimm(int port);

extern uint8_t fw_container[];

void phyinit_D_LoadIMEM(int port, struct ddr_configuration *data, int training_2d)
{
	/* Set MemResetL to avoid glitch on BP_MemReset_L during training */
	if (!training_2d) {
		DDRPHY_WRITE_REG16(port, (tMASTER | csr_MemResetL_ADDR), csr_ProtectMemReset_MASK);
	}

	/* select PMU iccm image */
	unsigned int image_offs = 0;
	unsigned int image_size = 0;

	if (data->registered_dimm) {
		if (training_2d) {
			image_offs = RDIMM_PMUTRAIN_2D_IMEM_OFFS;
			image_size = RDIMM_PMUTRAIN_2D_IMEM_SIZE;
		} else {
			image_offs = RDIMM_PMUTRAIN_1D_IMEM_OFFS;
			image_size = RDIMM_PMUTRAIN_1D_IMEM_SIZE;
		}
	} else {
		if (training_2d) {
			image_offs = UDIMM_PMUTRAIN_2D_IMEM_OFFS;
			image_size = UDIMM_PMUTRAIN_2D_IMEM_SIZE;
		} else {
			image_offs = UDIMM_PMUTRAIN_1D_IMEM_OFFS;
			image_size = UDIMM_PMUTRAIN_1D_IMEM_SIZE;
		}
	}

	/*
	 * 1. Enable access to the internal CSRs by setting the MicroContMuxSel CSR to 0.
	 * This allows the memory controller unrestricted access to the configuration CSRs.
	 */
	DDRPHY_WRITE_REG16(port, (tAPBONLY | csr_MicroContMuxSel_ADDR), 0x0);

	const uint16_t *msg = (uint16_t *)(fw_container + image_offs);
	unsigned int index;

	for (index = 0; index < image_size / sizeof(uint16_t); index++) {
		DDRPHY_WRITE_REG16(port, (DDRPHY_PMU_ICCM_ADDR + index), msg[index]);
	}

	for (; index < DDRPHY_PMU_ICCM_SIZE / sizeof(uint16_t); index++) {
		DDRPHY_WRITE_REG16(port, (DDRPHY_PMU_ICCM_ADDR + index), 0x0);
	}

	/*
	 * 2. Isolate the APB access from the internal CSRs by setting the MicroContMuxSel CSR to 1.
	 * This allows the firmware unrestricted access to the configuration CSRs.
	 */
	DDRPHY_WRITE_REG16(port, (tAPBONLY | csr_MicroContMuxSel_ADDR), 0x1);
}

/** @brief This function loads the training firmware DMEM image and write the
 * Message Block parameters for the training firmware into the SRAM.
 * This function performs the following tasks:
 * -# Load the firmware DMEM segment to initialize the data structures from the
 * DMEM incv file provided in the training firmware package.
 * -# Write the Firmware Message Block with the required contents detailing the training parameters.
 */
void phyinit_F_LoadDMEM(int port, struct ddr_configuration *data, const void *mb, int training_2d)
{
	/* 1. Enable access to the internal CSRs by setting the MicroContMuxSel CSR to 0. */
	/*	This allows the memory controller unrestricted access to the configuration CSRs. */
	DDRPHY_WRITE_REG16(port, (tAPBONLY | csr_MicroContMuxSel_ADDR), 0x0);

	int dccm_index = 0;

	/* upload PMU message block */
	unsigned int msg_block_size = sizeof(struct pmu_smb_ddr4_t);
	uint16_t *msg = (uint16_t *)mb;

	for (; dccm_index < msg_block_size / sizeof(uint16_t); dccm_index++) {
		DDRPHY_WRITE_REG16(port, (DDRPHY_PMU_DCCM_ADDR + dccm_index), msg[dccm_index]);
	}

	/* upload PMU dccm image (tail without message block) */
	unsigned int dccm_image_offs = 0;
	unsigned int dccm_image_size = 0;

	if (data->registered_dimm) {
		if (training_2d) {
			dccm_image_offs = RDIMM_PMUTRAIN_2D_DMEM_OFFS;
			dccm_image_size = RDIMM_PMUTRAIN_2D_DMEM_SIZE;
		} else {
			dccm_image_offs = RDIMM_PMUTRAIN_1D_DMEM_OFFS;
			dccm_image_size = RDIMM_PMUTRAIN_1D_DMEM_SIZE;
		}
	} else {
		if (training_2d) {
			dccm_image_offs = UDIMM_PMUTRAIN_2D_DMEM_OFFS;
			dccm_image_size = UDIMM_PMUTRAIN_2D_DMEM_SIZE;
		} else {
			dccm_image_offs = UDIMM_PMUTRAIN_1D_DMEM_OFFS;
			dccm_image_size = UDIMM_PMUTRAIN_1D_DMEM_SIZE;
		}
	}

	msg = (uint16_t *)(fw_container + dccm_image_offs);
	for (; dccm_index < dccm_image_size / sizeof(uint16_t); dccm_index++) {
		DDRPHY_WRITE_REG16(port, (DDRPHY_PMU_DCCM_ADDR + dccm_index), msg[dccm_index]);
	}

	if (dccm_index % 2) {
		/* Always write an even number of words so no 32bit quantity is uninitialized */
		DDRPHY_WRITE_REG16(port, (DDRPHY_PMU_DCCM_ADDR + dccm_index), 0x0);
		dccm_index++;
	}

	/*
	 * 2. Isolate the APB access from the internal CSRs by setting the MicroContMuxSel CSR to 1.
	 *	This allows the firmware unrestricted access to the configuration CSRs.
	 */
	DDRPHY_WRITE_REG16(port, (tAPBONLY | csr_MicroContMuxSel_ADDR), 0x1);
}

/**@brief Loads registers after training
 * This function programs the PHY Initialization Engine (PIE) instructions and
 * the associated registers.
 */
void phyinit_I_LoadPIE(int port, struct ddr_configuration *data)
{
	/* Enable access to the internal CSRs by setting the MicroContMuxSel CSR to 0. */
	/* This allows the memory controller unrestricted access to the configuration CSRs. */
	DDRPHY_WRITE_REG16(port, (tAPBONLY | csr_MicroContMuxSel_ADDR), 0x0);

	/* Programming PIE Production Code */
	if (data->registered_dimm) {
		phyinit_LoadPieProdCode_rdimm(port);
	} else {
		phyinit_LoadPieProdCode_udimm(port);
	}

	/*
	 * Registers: Seq0BDLY0, Seq0BDLY1, Seq0BDLY2, Seq0BDLY3
	 *	- Program PIE instruction delays
	 */

	/* Need delays for 0.5us, 1us, 10us, and 25us. */
	uint16_t psCount[4];
	const int delayScale = 1;
	int DfiFrq = data->clock_mhz / 2;

	psCount[0] = DfiFrq * delayScale / 2 / 4;

	int LowFreqOpt = 0;

	if (data->registered_dimm) {
		if (data->clock_mhz < 400) {
			LowFreqOpt = 7;
		} else if (data->clock_mhz < 533) {
			LowFreqOpt = 14;
		}
	} else {
		if (data->clock_mhz < 400) {
			LowFreqOpt = 3;
		} else if (data->clock_mhz < 533) {
			LowFreqOpt = 11;
		}
	}

	psCount[1] = DfiFrq * delayScale * 1 / 4 - LowFreqOpt;
	psCount[2] = DfiFrq * delayScale * 10 / 4;

	int dllLock;

	if (DfiFrq > 267) {
		dllLock = 176;
	} else if (DfiFrq <= 267 && DfiFrq > 200) {
		dllLock = 132;
	} else {
		dllLock = 64;
	}

	psCount[3] = dllLock / 4;

	DDRPHY_WRITE_REG16(port, (tMASTER | csr_Seq0BDLY0_ADDR), psCount[0]);
	DDRPHY_WRITE_REG16(port, (tMASTER | csr_Seq0BDLY1_ADDR), psCount[1]);
	DDRPHY_WRITE_REG16(port, (tMASTER | csr_Seq0BDLY2_ADDR), psCount[2]);
	DDRPHY_WRITE_REG16(port, (tMASTER | csr_Seq0BDLY3_ADDR), psCount[3]);

	/*
	 * Registers: Seq0BDisableFlag0 Seq0BDisableFlag1 Seq0BDisableFlag2
	 * Seq0BDisableFlag3 Seq0BDisableFlag4 Seq0BDisableFlag5
	 *	- Program PIE Instruction Disable Flags
	 */
	DDRPHY_WRITE_REG16(port, (tINITENG | csr_Seq0BDisableFlag0_ADDR), 0x0000);
	DDRPHY_WRITE_REG16(port, (tINITENG | csr_Seq0BDisableFlag1_ADDR), 0x0173);
	DDRPHY_WRITE_REG16(port, (tINITENG | csr_Seq0BDisableFlag2_ADDR), 0x0060);
	DDRPHY_WRITE_REG16(port, (tINITENG | csr_Seq0BDisableFlag3_ADDR), 0x6110);
	DDRPHY_WRITE_REG16(port, (tINITENG | csr_Seq0BDisableFlag4_ADDR), 0x2152);
	DDRPHY_WRITE_REG16(port, (tINITENG | csr_Seq0BDisableFlag5_ADDR), 0xdfbd);
	uint16_t Seq0BDisableFlag6 = 0xffff;
#if (SNPS_UMCTLOPT > 0)
	Seq0BDisableFlag6 = 0x6000; /* Enable Synopsys Uctl Controller DDR4 RDIMM PIE Optimizations */
#endif
	DDRPHY_WRITE_REG16(port, (tINITENG | csr_Seq0BDisableFlag6_ADDR), Seq0BDisableFlag6);
	DDRPHY_WRITE_REG16(port, (tINITENG | csr_Seq0BDisableFlag7_ADDR), 0x6152);

	if (data->registered_dimm) {
		/* Registers AcsmPlayback*x* */

		/* Program Address/Command Sequence Engine (ACSM) registers with required instructions for retraining algorithm. */
		int acsmplayback[2][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
		int NumVec = 0;

		/* DIMM Operating Speed */
		uint32_t F0RC0A = 0x0a0 | (0xf & data->pocket.F0RC0A_D0); /* use the RCD register value for 1D training. */

		acsmplayback[0][NumVec] = 0x3ff & F0RC0A;
		acsmplayback[1][NumVec] = (0x1c00 & F0RC0A) >> 10;
		NumVec += 1;

		/* Fine Granularity RDIMM Operating Speed */
		uint32_t F0RC3x = 0x300 | (0xff & data->pocket.F0RC3x_D0);

		acsmplayback[0][NumVec] = 0x3ff & F0RC3x;
		acsmplayback[1][NumVec] = (0x1c00 & F0RC3x) >> 10;
		NumVec += 1;

		/* F0RC5x: CW Destination Selection & Write/Read Additional QxODT[1:0] Signal High */
		uint32_t F0RC5x = 0x500 | (0xff & SNPS_UMCTL_F0RC5x);

		acsmplayback[0][NumVec] = 0x3ff & F0RC5x;
		acsmplayback[1][NumVec] = (0x1c00 & F0RC5x) >> 10;
		NumVec += 1;

		for (int vec = 0; vec < NumVec; vec++) {
			DDRPHY_WRITE_REG16(port, (tACSM | (csr_AcsmPlayback0x0_ADDR + vec * 2)), acsmplayback[0][vec]);
			DDRPHY_WRITE_REG16(port, (tACSM | (csr_AcsmPlayback1x0_ADDR + vec * 2)), acsmplayback[1][vec]);
		}

		/* Program Training Hardware Registers for mission mode retraining and DRAM drift compensation algorithm. */
		/* Register: AcsmCtrl13	Fields: AcsmCkeEnb */
		uint16_t regData = (0xf << csr_AcsmCkeEnb_LSB);

		DDRPHY_WRITE_REG16(port, (tACSM | csr_AcsmCtrl13_ADDR), regData);
		/* Register: AcsmCtrl0	Fields: AcsmParMode, Acsm2TMode */
		DDRPHY_WRITE_REG16(port, (tACSM | csr_AcsmCtrl0_ADDR), csr_AcsmParMode_MASK | csr_Acsm2TMode_MASK);
	}

	/* - Register: CalZap
	 * - Prepare the calibration controller for mission mode.
	 *  Turn on calibration and hold idle until dfi_init_start is asserted sequence is triggered.
	 */
	DDRPHY_WRITE_REG16(port, (tMASTER | csr_CalZap_ADDR), 0x1);

	/* - Register: CalRate
	 * - Fields:
	 *  - CalRun
	 *  - CalOnce
	 *  - CalInterval
	 */
	int CalInterval = 0x9;
	int CalOnce = 0x0;
	int CalRate = (0x1 << csr_CalRun_LSB) | (CalOnce << csr_CalOnce_LSB) | (CalInterval << csr_CalInterval_LSB);

	DDRPHY_WRITE_REG16(port, (tMASTER | csr_CalRate_ADDR), CalRate);

	/*
	 * At the end of this function, PHY Clk gating register UcclkHclkEnables is
	 * set for mission mode. Additionally APB access is Isolated by setting MicroContMuxSel.
	 */

	/* Disabling Ucclk (PMU) and Hclk (training hardware) */
	DDRPHY_WRITE_REG16(port, (tDRTUB | csr_UcclkHclkEnables_ADDR), 0x0);
	/* Isolate the APB access from the internal CSRs by setting the MicroContMuxSel CSR to 1. */
	DDRPHY_WRITE_REG16(port, (tAPBONLY | csr_MicroContMuxSel_ADDR), 0x1);
}
