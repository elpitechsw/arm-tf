/*
 * Copyright (c) 2022-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include "../ddr_main.h"
#include "ddr_phy_misc.h"
#include "ddr_phy_tmp_regs.h"

#define NUMBER_ANIB	12
/*
 * Enable Write DQS Extension feature of PHY.
 * See "DesignWare Cores LPDDR4 MultiPHY, WDQS Extension Application Note"
 */
#define WDQSEXT		0
#define SNPS_UMCTLOPT	0
#define EXTCALRES_VAL	0

void phyinit_C_PhyConfig(int port, struct ddr_configuration *data)
{
	/*
	 * Program TxSlewRate:
	 * - TxSlewRate::TxPreDrvMode is dependent on DramType.
	 * - TxSlewRate::TxPreP and TxSlewRate::TxPreN are technology-specific
	 * User should consult the "Output Slew Rate" section of HSpice Model App Note
	 * in specific technology for recommended settings
	 */
	const int TxPreP = 0xf; /* Default to 0xf (max). Optimal setting is technology specific. */
	const int TxPreN = 0xf; /* Default to 0xf (max). Optimal setting is technology specific. */
	const int TxPreDrvMode = 0x2;
	const int TxSlewRate = (TxPreDrvMode << csr_TxPreDrvMode_LSB) |
			(TxPreN << csr_TxPreN_LSB) | (TxPreP << csr_TxPreP_LSB);

	unsigned int byte;

	for (byte = 0; byte < 9; byte++) {
		for (unsigned int lane = 0; lane <= 1; lane++) {
			unsigned int b_addr = lane << 8;
			unsigned int c_addr = byte << 12;

			DDRPHY_WRITE_REG16(port, (tDBYTE | c_addr | b_addr | csr_TxSlewRate_ADDR), TxSlewRate);
		}
	}

	/*
	 * Program ATxSlewRate:
	 * - ATxSlewRate::ATxPreDrvMode is dependent on DramType and whether the ACX4 instance is used for AC or CK
	 * - ATxSlewRate::ATxPreP and ATxSlewRate::TxPreN are technology-specific
	 * User should consult the "Output Slew Rate" section of HSpice Model App Note
	 * in specific technology for recommended settings
	 */
	unsigned int anib;

	for (anib = 0; anib < NUMBER_ANIB; anib++) {
		/* # of ANIBs CK ANIB Instance */
		/* ACX8		ANIB 1 */
		/* ACX10	ANIB 4,5 */
		/* ACX12	ANIB 4,5 */
		/* ACX13	ANIB 4,5 */

		int ATxPreDrvMode;

		if (anib == 0x4 || anib == 0x5) {
			/* CK ANIB instance */
			ATxPreDrvMode = 0x0;
		} else {
			/* non-CK ANIB instance */
			ATxPreDrvMode = 0x3;
		}

		const int ATxPreP = 0xf; /* Default to 0xf (max). Optimal setting is technology specific. */
		const int ATxPreN = 0xf; /* Default to 0xf (max). Optimal setting is technology specific. */
		const int ATxSlewRate = (ATxPreDrvMode << csr_ATxPreDrvMode_LSB) |
				(ATxPreN << csr_ATxPreN_LSB) | (ATxPreP << csr_ATxPreP_LSB);

		unsigned int c_addr = anib << 12;

		DDRPHY_WRITE_REG16(port, (tANIB | c_addr | csr_ATxSlewRate_ADDR), ATxSlewRate);
	}

	if (data->registered_dimm) {
		/* Program EnableCsMulticast */
		DDRPHY_WRITE_REG16(port, (tMASTER | csr_EnableCsMulticast_ADDR), 0x0);
	}

	/* Program PllCtrl2: Calculate PLL controls per p-state from Frequency */
	int PllCtrl2;

	if (data->clock_mhz / 2 < 235) {
		PllCtrl2 = 0x7;
	} else if (data->clock_mhz / 2 < 313) {
		PllCtrl2 = 0x6;
	} else if (data->clock_mhz / 2 < 469) {
		PllCtrl2 = 0xb;
	} else if (data->clock_mhz / 2 < 625) {
		PllCtrl2 = 0xa;
	} else if (data->clock_mhz / 2 < 938) {
		PllCtrl2 = 0x19;
	} else if (data->clock_mhz / 2 < 1067) {
		PllCtrl2 = 0x18;
	} else {
		PllCtrl2 = 0x19;
	}

	DDRPHY_WRITE_REG16(port, (tMASTER | csr_PllCtrl2_ADDR), PllCtrl2);

	/*
	 * Program ARdPtrInitVal:
	 * - The values programmed here assume ideal properties of DfiClk and Pclk including:
	 * - DfiClk skew
	 * - DfiClk jitter
	 * - DfiClk PVT variations
	 * - Pclk skew
	 * - Pclk jitter
	 * ARdPtrInitVal Programmed differently based on PLL Bypass mode and Frequency:
	 * - PLL Bypassed mode:
	 * - For MemClk frequency > 933MHz, the valid range of ARdPtrInitVal_p0[3:0] is: 2-6
	 * - For MemClk frequency < 933MHz, the valid range of ARdPtrInitVal_p0[3:0] is: 1-6
	 * - PLL Enabled mode:
	 * - For MemClk frequency > 933MHz, the valid range of ARdPtrInitVal_p0[3:0] is: 1-6
	 * - For MemClk frequency < 933MHz, the valid range of ARdPtrInitVal_p0[3:0] is: 0-6
	 */
	int ARdPtrInitVal;

	if (data->clock_mhz >= 933) {
		ARdPtrInitVal = 0x2;
	} else {
		ARdPtrInitVal = 0x1;
	}

	DDRPHY_WRITE_REG16(port, (tMASTER | csr_ARdPtrInitVal_ADDR), ARdPtrInitVal);

	/*
	 * Program DbyteDllModeCntrl:
	 * - DllRxPreambleMode
	 * Program DqsPreambleControl:
	 * - Fields:
	 * - TwoTckRxDqsPre
	 * - TwoTckTxDqsPre
	 * - PositionDfeInit
	 * - LP4TglTwoTckTxDqsPre
	 * - LP4PostambleExt
	 * - LP4SttcPreBridgeRxEn
	 */
	int TwoTckRxDqsPre = !!data->rd_preamble_2CK;
	const int LP4SttcPreBridgeRxEn = 0x0;
	const int DllRxPreambleMode = 0x1;
	int TwoTckTxDqsPre = !!data->wr_preamble_2CK;
	const int LP4TglTwoTckTxDqsPre = 0x0;
	const int PositionDfeInit = 0x2;
	const int LP4PostambleExt = 0x0;

	const int WDQSEXTENSION = 0;
	int DqsPreambleControl = (WDQSEXTENSION << csr_WDQSEXTENSION_LSB) |
			(LP4SttcPreBridgeRxEn << csr_LP4SttcPreBridgeRxEn_LSB) |
			(LP4PostambleExt << csr_LP4PostambleExt_LSB) |
			(LP4TglTwoTckTxDqsPre << csr_LP4TglTwoTckTxDqsPre_LSB) |
			(PositionDfeInit << csr_PositionDfeInit_LSB) |
			(TwoTckTxDqsPre << csr_TwoTckTxDqsPre_LSB) |
			(TwoTckRxDqsPre << csr_TwoTckRxDqsPre_LSB);
	DDRPHY_WRITE_REG16(port, (tMASTER | csr_DqsPreambleControl_ADDR), DqsPreambleControl);

	int DbyteDllModeCntrl = DllRxPreambleMode << csr_DllRxPreambleMode_LSB;

	DDRPHY_WRITE_REG16(port, (tMASTER | csr_DbyteDllModeCntrl_ADDR), DbyteDllModeCntrl);

	const int DllGainIV = 0x1;
	const int DllGainTV = 0x6;
	int DllGainCtl = DllGainIV | (DllGainTV<<csr_DllGainTV_LSB);
	const int DisDllSeedSel = 0;
	const int DisDllGainIVSeed = 1;
	const int LcdlSeed0 = 0x21;
	int DllLockParam = DisDllSeedSel | (DisDllGainIVSeed<<csr_DisDllGainIVSeed_LSB)|(LcdlSeed0<<csr_LcdlSeed0_LSB);

	DDRPHY_WRITE_REG16(port, (tMASTER | csr_DllLockParam_ADDR), DllLockParam);
	DDRPHY_WRITE_REG16(port, (tMASTER | csr_DllGainCtl_ADDR), DllGainCtl);

	/*
	 * Program ProcOdtTimeCtl:
	 * - POdtStartDelay[3:2]
	 * - POdtTailWidth[1:0]
	 */
	int ProcOdtTimeCtl;

	if (WDQSEXT) {
		ProcOdtTimeCtl = 0x3; /* POdtStartDelay = 0x0, POdtTailWidth = 0x3 */
		/* Memclk Freq <= 933MHz */
	} else if (data->clock_mhz <= 933) {
		ProcOdtTimeCtl = 0xa; /* POdtStartDelay = 0x2, POdtTailWidth = 0x2 */
		/* 933MHz < Memclk Freq <= 1200MHz */
	} else if (data->clock_mhz <= 1200) {
		if (TwoTckRxDqsPre == 1) {
			ProcOdtTimeCtl = 0x2;/* POdtStartDelay = 0x0, POdtTailWidth = 0x2 */
		} else {
			ProcOdtTimeCtl = 0x6;/* POdtStartDelay = 0x1, POdtTailWidth = 0x2 */
		}
		/* Memclk Freq > 1200MHz */
	} else {
		if (TwoTckRxDqsPre == 1) {
			ProcOdtTimeCtl = 0x3; /* POdtStartDelay = 0x0, POdtTailWidth = 0x3 */
		} else {
			ProcOdtTimeCtl = 0x7; /* POdtStartDelay = 0x1, POdtTailWidth = 0x3 */
		}
	}

	DDRPHY_WRITE_REG16(port, (tMASTER | csr_ProcOdtTimeCtl_ADDR), ProcOdtTimeCtl);

	/*
	 * Program TxOdtDrvStren:
	 * - ODTStrenP_px[5:0]
	 * - ODTStrenN_px[11:6]
	 */
	int ODTStrenP = phyinit_mapDrvStren(data->PHY_ODT, ODTStrenP_T);
	int ODTStrenN = phyinit_mapDrvStren(data->PHY_ODT, ODTStrenN_T);
	int TxOdtDrvStren = (ODTStrenN << csr_ODTStrenN_LSB) | ODTStrenP;

	for (byte = 0; byte < 9; byte++) {
		for (unsigned int lane = 0; lane <= 1; lane++) {
			unsigned int b_addr = lane << 8;
			unsigned int c_addr = byte << 12;

			DDRPHY_WRITE_REG16(port, (tDBYTE | c_addr | b_addr | csr_TxOdtDrvStren_ADDR), TxOdtDrvStren);
		}
	}

	/*
	 * Program TxImpedanceCtrl1:
	 * - DrvStrenFSDqP[5:0]
	 * - DrvStrenFSDqN[11:6]
	 */
	int DrvStrenFSDqP = phyinit_mapDrvStren(data->PHY_ODI, DrvStrenFSDqP_T);
	int DrvStrenFSDqN = phyinit_mapDrvStren(data->PHY_ODI, DrvStrenFSDqN_T);
	int TxImpedanceCtrl1 = (DrvStrenFSDqN << csr_DrvStrenFSDqN_LSB) | (DrvStrenFSDqP << csr_DrvStrenFSDqP_LSB);

	for (byte = 0; byte < 9; byte++) {
		for (unsigned int lane = 0; lane <= 1; lane++) {
			unsigned int b_addr = lane << 8;
			unsigned int c_addr = byte << 12;

			DDRPHY_WRITE_REG16(port,
				(tDBYTE | c_addr | b_addr | csr_TxImpedanceCtrl1_ADDR),
				TxImpedanceCtrl1);
		}
	}

	/*
	 * Program ATxImpedance:
	 *	- ADrvStrenP[4:0]
	 *	- ADrvStrenN[9:5]
	 */
	int ATxImpedance_config = 20;

	for (anib = 0; anib < NUMBER_ANIB; anib++) {
		const int ADrvStrenP = phyinit_mapDrvStren(ATxImpedance_config, ADrvStrenP_T);
		const int ADrvStrenN = phyinit_mapDrvStren(ATxImpedance_config, ADrvStrenN_T);
		const int ATxImpedance = (ADrvStrenN << csr_ADrvStrenN_LSB) | (ADrvStrenP << csr_ADrvStrenP_LSB);
		unsigned int c_addr = anib << 12;

		DDRPHY_WRITE_REG16(port, (tANIB | c_addr | csr_ATxImpedance_ADDR), ATxImpedance);
	}

	/* Program DfiMode */
	int DfiMode = 0x1; /* DFI1 does not physically exists */

	DDRPHY_WRITE_REG16(port, (tMASTER | csr_DfiMode_ADDR), DfiMode);

	/*
	 * Program DfiCAMode:
	 * - DfiLp3CAMode
	 * - DfiD4CAMode
	 * - DfiLp4CAMode
	 * - DfiD4AltCAMode
	 */
	int DfiCAMode = 2; /* DDR4 */

	DDRPHY_WRITE_REG16(port, (tMASTER | csr_DfiCAMode_ADDR), DfiCAMode);

	/*
	 * Program CalDrvStr0:
	 * - CalDrvStrPd50[3:0]
	 * - CalDrvStrPu50[7:4]
	 */
	uint16_t CalDrvStrPu50 = EXTCALRES_VAL;

	uint16_t CalDrvStrPd50 = CalDrvStrPu50;
	uint16_t CalDrvStr0 = (CalDrvStrPu50 << csr_CalDrvStrPu50_LSB) | CalDrvStrPd50;

	DDRPHY_WRITE_REG16(port, (tMASTER | csr_CalDrvStr0_ADDR), CalDrvStr0);

	/* Program CalUclkInfo: Impedance calibration CLK Counter. */
	/* number of DfiClk cycles per 1us - round up */
	uint16_t CalUClkTicksPer1uS = (data->clock_mhz + 1) / 2;

	if (CalUClkTicksPer1uS < 24) {
		CalUClkTicksPer1uS = 24; /* Minimum value of CalUClkTicksPer1uS = 24 */
	}

	DDRPHY_WRITE_REG16(port, (tMASTER | csr_CalUclkInfo_ADDR), CalUClkTicksPer1uS);

	/***************************************************************
	 * Program Calibration CSRs based on user input
	 * CSRs to program:
	 * CalRate:: CalInterval
	 * :: CalOnce
	 * User input dependencies::
	 * CalInterval
	 * CalOnce
	 ***************************************************************/
	int CalInterval = 0x9;
	int CalOnce = 0x0;
	int CalRate = (CalOnce << csr_CalOnce_LSB) | (CalInterval << csr_CalInterval_LSB);

	DDRPHY_WRITE_REG16(port, (tMASTER | csr_CalRate_ADDR), CalRate);

	/* Program VrefInGlobal:
	 * - DqDqsRcvCntrl and csrVrefInGlobal to select Global VREF from Master to be used in each DQ
	 * - Fields:
	 *	 - GlobalVrefInSel: Select Range of GlobalVref DAC. Default: set to 1.
	 *	 - GlobalVrefInDAC: Vref level is set based on mb_DDR4U_1D[pstate].PhyVref value.
	 *		The following formula is used to convert the PhyVref into the register setting.
	 *		\f{eqnarray*}{
	 *			 PhyVrefPrcnt &=& \frac{mb DDR4U 1D[pstate].PhyVref}{128}
	 *		 if GlobalVrefInSel = 1 :
	 *			 GlobalVrefInDAC &=& 1+\frac{PhyVrefPrcnt}{0.005}
	 *		 if GlobalVrefInSel = 0 :
	 *			 GlobalVrefInDAC &=& \frac{(PhyVrefPrcnt-0.345)}{0.005}
	 *			 RxVref &=& (GlobalVrefInDAC == 0) ? Hi-Z : (PhyVrefPrcnt \times VDDQ)
	 *		 \f}
	 * - Program DqDqsRcvCntrl:
	 *	- DqDqsRcvCntrl and csrVrefInGlobal to select Global VREF from Master to be used in each DQ
	 *	- Fields:
	 *	- SelAnalogVref
	 *	- MajorModeDbyte
	 *	- ExtVrefRange
	 *	- DfeCtrl
	 *	- GainCurrAdj
	 */
	int MajorModeDbyte = 3; /* DDR4 */

	const int SelAnalogVref = 1; /* Use Global VREF from Master */
	const int ExtVrefRange_defval = 0;
	const int DfeCtrl_defval = 0;
	const int GainCurrAdj_defval = 0xb;

	int vref_percentVddq = data->HOST_VREF * 1000 * 100 / 128;

	uint8_t GlobalVrefInSel = 0x4;
	/* check range1 first. Only use range0 if customer input maxes out range1 */
	uint8_t GlobalVrefInDAC = (uint8_t)((vref_percentVddq / 500) + 1); /* Min value is 1 */

	if (GlobalVrefInDAC > 127) {
		GlobalVrefInDAC = (uint8_t)((vref_percentVddq - 34500) / 500);
		if (GlobalVrefInDAC < 1) {
			GlobalVrefInDAC = 1; /* Min value is 1 */
		}
		GlobalVrefInSel = 0x0;
	}
	if (GlobalVrefInDAC > 127) {
		GlobalVrefInDAC = 127;
	}

	int VrefInGlobal = (GlobalVrefInDAC << csr_GlobalVrefInDAC_LSB) | GlobalVrefInSel;

	DDRPHY_WRITE_REG16(port, (tMASTER | csr_VrefInGlobal_ADDR), VrefInGlobal);

	int DqDqsRcvCntrl = (GainCurrAdj_defval << csr_GainCurrAdj_LSB) |
			(MajorModeDbyte << csr_MajorModeDbyte_LSB) |
			(DfeCtrl_defval << csr_DfeCtrl_LSB) |
			(ExtVrefRange_defval << csr_ExtVrefRange_LSB) |
			(SelAnalogVref << csr_SelAnalogVref_LSB);

	for (byte = 0; byte < 9; byte++) {
		for (unsigned int lane = 0; lane <= 1; lane++) {
			unsigned int b_addr = lane << 8;
			unsigned int c_addr = byte << 12;

			DDRPHY_WRITE_REG16(port,
				(tDBYTE | c_addr | b_addr | csr_DqDqsRcvCntrl_ADDR),
				DqDqsRcvCntrl);
		}
	}

	/*
	 * Program MemAlertControl and MemAlertControl2:
	 * - Fields:
	 * - MALERTVrefLevel
	 * - MALERTPuStren
	 * - MALERTPuEn
	 * - MALERTRxEn
	 * - MALERTSyncBypass
	 * - MALERTDisableVal
	 */

	/* MemAlert applies to DDR4(all DIMM) or DDR3(RDIMM) only */
	const int MALERTPuEn = 1;
	const int MALERTRxEn = 1;
	int MALERTPuStren = 5;
	const int MALERTDisableVal_defval = 1;
	int MALERTVrefLevel = 0x29;

	int MemAlertControl = (MALERTDisableVal_defval << 14) |
			(MALERTRxEn << 13) | (MALERTPuEn << 12) |
			(MALERTPuStren << 8) | MALERTVrefLevel;
	DDRPHY_WRITE_REG16(port, (tMASTER | csr_MemAlertControl_ADDR), MemAlertControl);

	int MALERTSyncBypass = 0x0;
	int MemAlertControl2 = (MALERTSyncBypass << csr_MALERTSyncBypass_LSB);

	DDRPHY_WRITE_REG16(port, (tMASTER | csr_MemAlertControl2_ADDR), MemAlertControl2);

	/* Program DfiFreqRatio */
	int DfiFreqRatio = 1; /* set 1 for 1:2 DFI clock */

	DDRPHY_WRITE_REG16(port, (tMASTER | csr_DfiFreqRatio_ADDR), DfiFreqRatio);

	/*
	 * Program TristateModeCA based on DramType and 2T Timing
	 * - Fields:
	 * - CkDisVal
	 * - DisDynAdrTri
	 * - DDR2TMode
	 */

	/* CkDisVal depends on DramType */
	int CkDisVal_def = 1;
	int DisDynAdrTri = 0x0;
	int DDR2TMode = data->timing_2t;
	int TristateModeCA = (CkDisVal_def << csr_CkDisVal_LSB) |
				(DDR2TMode << csr_DDR2TMode_LSB) |
				(DisDynAdrTri << csr_DisDynAdrTri_LSB);
	DDRPHY_WRITE_REG16(port, (tMASTER | csr_TristateModeCA_ADDR), TristateModeCA);

	/* Program DfiXlat based on Pll Bypass Input */
	uint16_t pllbypass_dat = 0;

	pllbypass_dat |= 0x0 << (0);

	for (uint16_t loopVector = 0; loopVector < 8; loopVector++) {
		if (loopVector == 0) {
			/* Relock DfiFreq = 00,01,02,03) Use StartVec 5 (pll_enabled) or StartVec 6 (pll_bypassed) */
			uint16_t dfifreqxlat_dat = pllbypass_dat + 0x5555;

			DDRPHY_WRITE_REG16(port, (tMASTER | (csr_DfiFreqXlat0_ADDR + loopVector)), dfifreqxlat_dat);
		} else if (loopVector == 7) {
			/* LP3-entry DfiFreq = 1F */
			DDRPHY_WRITE_REG16(port, (tMASTER | (csr_DfiFreqXlat0_ADDR + loopVector)), 0xf000);
		} else {
			/*
			 * Everything else = skip retrain (could also map to 0000 since retrain code is excluded,
			 * but this is cleaner)
			 */
			DDRPHY_WRITE_REG16(port, (tMASTER | (csr_DfiFreqXlat0_ADDR + loopVector)), 0x5555);
		}
	}

	/*
	 * Program DqDqsRcvCntrl1 (Receiver Powerdown) and DbyteMiscMode
	 *	- see function dwc_ddrphy_phyinit_IsDbyteDisabled() to determine
	 *	 which DBytes are turned off completely based on PHY configuration.
	 * - Fields:
	 *  - DByteDisable
	 *  - PowerDownRcvr
	 *  - PowerDownRcvrDqs
	 *  - RxPadStandbyEn
	 */
	/* Implements Section 1.3 of Pub Databook */
	for (int d = 0; d < 9; d++) /* for each dbyte */ {
		if (d == 8 && (!data->ecc_on)) {
			int c_addr = d * c1;
			uint16_t regData = 0x1 << csr_DByteDisable_LSB;

			DDRPHY_WRITE_REG16(port, (c_addr | tDBYTE | csr_DbyteMiscMode_ADDR), regData);

			unsigned int regData1 = (0x1ff << csr_PowerDownRcvr_LSB |
				0x1 << csr_PowerDownRcvrDqs_LSB |
				0x1 << csr_RxPadStandbyEn_LSB);

			DDRPHY_WRITE_REG16(port, (c_addr | tDBYTE | csr_DqDqsRcvCntrl1_ADDR), regData1);
		} else {
			/* disable RDBI lane if not used. */
			int mr5_rdbi_off = 1;

			if (((data->pocket.MR5 >> 12) & 0x1)) {
				mr5_rdbi_off = 0;
			}

			if ((data->device_width != 4) && mr5_rdbi_off) {
				/* turn off Rx of DBI lane */
				unsigned int regData2 = (0x100 << csr_PowerDownRcvr_LSB | csr_RxPadStandbyEn_MASK);
				int c_addr = d * c1;

				DDRPHY_WRITE_REG16(port, (c_addr | tDBYTE | csr_DqDqsRcvCntrl1_ADDR), regData2);
			}
		}
	}

	/*
	 * Program DqDqsRcvCntrl1 (Receiver Powerdown) and DbyteMiscMode
	 * - Fields:
	 *  - X4TG
	 *  - MasterX4Config
	 * note: PHY does not support mixed dram device data width
	 */
	int X4TG = 0;

	if (data->device_width == 4) {
		X4TG = 0xf;
	}

	int MasterX4Config = X4TG << csr_X4TG_LSB;

	DDRPHY_WRITE_REG16(port, (tMASTER | csr_MasterX4Config_ADDR), MasterX4Config);

	/* Program DMIPinPresent based on DramType and Read-DBI enable
	 * - Fields:
	 * - RdDbiEnabled
	 */
	/* For DDR4, Read DBI is enabled in MR5-A12 */
	int DMIPinPresent = (data->pocket.MR5 >> 12) & 0x1;

	DDRPHY_WRITE_REG16(port, (tMASTER | csr_DMIPinPresent_ADDR), DMIPinPresent);

	/* note: This feature is BETA and untested. Future PhyInit version will fully enable this feature. */
	uint16_t Acx4AnibDis = 0x0;

	DDRPHY_WRITE_REG16(port, (tMASTER | csr_Acx4AnibDis_ADDR), Acx4AnibDis);
}

/** @brief This function must be used to trigger setting DfiClk to the
 * frequency associated with the input PState.
 * The purpose of this function is to change DfiClk to the desired frequency for
 * the input PState before proceeding to the next step. The PhyInit
 * dwc_ddrphy_phyinit_sequence() function calls this function multiple times in
 * order to set DfiClk before triggering training firmware execution for
 * different PStates.
 * the clock should be stable at the new frequency. For more information on
 * clocking requirements, see "Clocks" section in the PUB documentation.
 * @note this routine implies other items such as DfiFreqRatio, DfiCtlClk are
 * also set properly. Because the clocks are controlled in the SOC, external to
 * the software and PHY, this step is intended to be replaced by the user with
 * the necessary SOC operations to achieve the new input frequency to the PHY.
 */
void phyinit_E_setDfiClk(int port)
{
	/* Appendix: Enable here dwc_ddrphy_int interrupts */
	uint16_t reg = DDRPHY_READ_REG16(port, (tAPBONLY | csr_MicroContMuxSel_ADDR));
	int csr_access = (reg & csr_MicroContMuxSel_MASK);

	if (csr_access) {
		/* Enable access to the internal CSRs by setting the MicroContMuxSel CSR to 0. */
		/* This allows the memory controller unrestricted access to the configuration CSRs. */
		DDRPHY_WRITE_REG16(port, (tAPBONLY | csr_MicroContMuxSel_ADDR), 0x0);
	}

	/* Enable PHY interrupts */
	uint16_t int_enable = (1 << csr_PhyTrngCmpltEn_LSB) | (1 << csr_PhyInitCmpltEn_LSB) | (1 << csr_PhyTrngFailEn_LSB);

	DDRPHY_WRITE_REG16(port, (tMASTER | csr_PhyInterruptEnable_ADDR), int_enable);
	reg = DDRPHY_READ_REG16(port, (tMASTER | csr_PhyInterruptEnable_ADDR));

	if (csr_access) {
		/* Back to CSRs access by PHY Init Engine */
		DDRPHY_WRITE_REG16(port, (tAPBONLY | csr_MicroContMuxSel_ADDR), 0x1);
	}
}

/** @brief Execute the Training Firmware
 * The training firmware is executed with the following procedure:
 * -# Reset the firmware microcontroller by writing the MicroReset register to
 * set the StallToMicro and ResetToMicro fields to 1 (all other fields should be
 * zero). Then rewrite the registers so that only the StallToMicro remains set
 * (all other fields should be zero).
 * -# Begin execution of the training firmware by setting the MicroReset
 * register to 4'b0000.
 * -# Wait for the training firmware to complete by following the procedure in
 * "uCtrl Initialization and Mailbox Messaging" implemented in
 * dwc_ddrphy_phyinit_userCustom_G_waitFwDone() function.
 * -# Halt the microcontroller.
 */
int phyinit_G_ExecFW(int port)
{
	/* 1. Reset the firmware microcontroller by writing the MicroReset CSR to set the StallToMicro and */
	/* ResetToMicro fields to 1 (all other fields should be zero). */
	/* Then rewrite the CSR so that only the StallToMicro remains set (all other fields should be zero). */
	DDRPHY_WRITE_REG16(port, (tAPBONLY | csr_MicroContMuxSel_ADDR), 0x1);
	DDRPHY_WRITE_REG16(port, (tAPBONLY | csr_MicroReset_ADDR), csr_ResetToMicro_MASK|csr_StallToMicro_MASK);
	DDRPHY_WRITE_REG16(port, (tAPBONLY | csr_MicroReset_ADDR), csr_StallToMicro_MASK);

	/* 2. Begin execution of the training firmware by setting the MicroReset CSR to 4\'b0000. */
	DDRPHY_WRITE_REG16(port, (tAPBONLY | csr_MicroReset_ADDR), 0x0);

	/*
	 * 3. Wait for the training firmware to complete by following the procedure in
	 * uCtrl Initialization and Mailbox Messaging"
	 */
	int ret = phyinit_G_WaitFWDone(port);

	/* 4. Halt the microcontroller." */
	DDRPHY_WRITE_REG16(port, (tAPBONLY | csr_MicroReset_ADDR), csr_StallToMicro_MASK);

	return ret;
}
