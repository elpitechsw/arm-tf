/*
 * Copyright (c) 2021-2022, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <common/debug.h>

#include <baikal_def.h>

#include "ddr_baikal.h"
#include "ddr_master.h"
#include "ddr_spd.h"

#ifdef BAIKAL_DDR_SETTINGS_IN_FLASH
#include "ddr_menu.h"

extern struct ddr_flash_config ddr_storage;
#endif

int ddr_config_by_spd(int port, struct ddr_configuration *data)
{
	uint32_t tmp;
	extern struct spd_container spd_content;
	struct ddr4_spd_eeprom *const spd = &spd_content.content[port];

	data->dimms = 1;
#ifdef BAIKAL_DUAL_CHANNEL_MODE
	if (spd_content.dual_channel[port] == 'y') {
		data->dimms = 2;
	}
#endif
	/*
	 * The SPD spec has not the burst length byte, but DDR4 spec has
	 * nature BL8 and BC4, BL8 -bit3, BC4 -bit2.
	 */
	data->burst_lengths_bitmask = 0xc;

	/* DIMM organization parameters */
	data->ranks = ((spd->organization >> 3) & 0x7) + 1;
	if (data->ranks > 2) {
		ERROR("%d-rank memory is not supported\n", data->ranks);
		return -1;
	}

	data->device_width = 4 << (spd->organization & 0x7);
	if (data->device_width > 16 || data->device_width < 8) {
		ERROR("current ddr column width (%d bits) isn't supported\n", data->device_width);
		return -1;
	}

	/* SDRAM device parameters */
	data->bank_groups = ((spd->density_banks >> 6) & 0x3) * 2;
	if (data->bank_groups > 4 || data->bank_groups < 2) {
		ERROR("ddrc supports DIMMs with 2 or 4 bank groups only\n");
		return -1;
	}

	data->row_address = ((spd->addressing >> 3) & 0x7) + 12;
	if (data->row_address > 18 || data->row_address < 14) {
		ERROR("unsupported row address width (%d)\n", data->row_address);
		return -1;
	}

	switch (spd->module_type & DDR4_SPD_MODULETYPE_MASK) {
	case DDR4_SPD_MODULETYPE_RDIMM:
	case DDR4_SPD_MODULETYPE_72B_SO_RDIMM:
		data->registered_dimm	   = 1;
		data->mirrored_dimm	   = spd->mod_section.registered.reg_map & 0x1;
		data->registered_ca_stren  = spd->mod_section.registered.ca_stren;
		data->registered_clk_stren = spd->mod_section.registered.clk_stren;
		break;

	case DDR4_SPD_MODULETYPE_UDIMM:
	case DDR4_SPD_MODULETYPE_SO_DIMM:
	case DDR4_SPD_MODULETYPE_72B_SO_UDIMM:
		data->mirrored_dimm	   = spd->mod_section.unbuffered.addr_mapping & 0x1;
		data->registered_dimm	   = 0;
		data->registered_ca_stren  = 0;
		data->registered_clk_stren = 0;
		break;

	default:
		ERROR("%s: SDRAM memory module type 0x%02x is not supported\n",
		      __func__, spd->module_type);
		return -1;
	}

	/*
	 * MTB - medium timebase. The MTB in the SPD spec is 125 ps.
	 * FTB - fine timebase. The FTB in the SPD spec is 1 ps.
	 */
	if (spd->timebases) {
		ERROR("%s: unknown timebases 0x%02x\n", __func__, spd->timebases);
		return -1;
	}

	/* Get Connector to SDRAM Bit Mapping */
	memset(data->DQ_map, 0, sizeof(data->DQ_map));
	memcpy(((uint8_t *)data->DQ_map + 0), spd->mapping + 0, 8);
	memcpy(((uint8_t *)data->DQ_map + 8), spd->mapping + 10, 8);
	memcpy(((uint8_t *)data->DQ_map + 16), spd->mapping + 8, 2);
	data->DQ_swap_rank = ((spd->mapping[0] >> 6) & 0x3) == 0 ? 1 : 0;

	/* The DIMM has ECC capability when the extension bus exist */
	data->ecc_on = (spd->bus_width >> 3) & 0x1;
	if ((spd->bus_width & 0x7) == 0x2) {
		data->dbus_half = 1;
	}

	/* Get Cycle Time (tCK) */
	data->tCK = SPD_TO_PS(spd->tck_min, spd->fine_tck_min);
	data->clock_mhz = 1000000 / data->tCK;
	if ((data->clock_mhz <= 800) || ((data->clock_mhz > 800) && (data->clock_mhz < 900))) {
		data->clock_mhz = 800;
	} else if ((data->clock_mhz <= 933) || ((data->clock_mhz > 933) && (data->clock_mhz < 1050))) {
		data->clock_mhz = 933;
	} else if ((data->clock_mhz <= 1066) || ((data->clock_mhz > 1066) && (data->clock_mhz < 1150))) {
		data->clock_mhz = 1066;
	} else if ((data->clock_mhz <= 1200) || ((data->clock_mhz > 1200) && (data->clock_mhz < 1300))) {
		data->clock_mhz = 1200;
	} else {
		data->clock_mhz = 1333;
	}
#ifdef BAIKAL_DDR_CUSTOM_CLOCK_FREQ
	if (data->clock_mhz > BAIKAL_DDR_CUSTOM_CLOCK_FREQ) {
		data->clock_mhz = BAIKAL_DDR_CUSTOM_CLOCK_FREQ;
		data->tCK = 1000000 / data->clock_mhz;
	}
#endif
#ifdef BAIKAL_DDR_SETTINGS_IN_FLASH
	if (ddr_storage.ddr_sign == BAIKAL_FLASH_USE_STR) {
		switch (ddr_storage.speedbin) {
		case FLASH_SPEEDBIN_2400:
			data->clock_mhz = 1200;
			break;
		case FLASH_SPEEDBIN_2133:
			data->clock_mhz = 1066;
			break;
		case FLASH_SPEEDBIN_1866:
			data->clock_mhz = 933;
			break;
		case FLASH_SPEEDBIN_1600:
			data->clock_mhz = 800;
			break;
		default:
			ERROR("Wrong DDR frequncy setting from flash\n");
			break;
		}
	}
#endif

#ifdef BAIKAL_DUAL_CHANNEL_MODE
#ifdef BAIKAL_DBM20
	if (spd_content.dual_channel[port] == 'y') {
		data->clock_mhz = 800; /* 1600 MHz in double rate */
		data->tCK = 1000000 / data->clock_mhz;
	}
#endif
#endif /* BAIKAL_DUAL_CHANNEL_MODE */

	/* Compute CAS Latency (CL) */
	uint64_t lat_mask = ((uint64_t)spd->caslat_b1 << 7)  |
			    ((uint64_t)spd->caslat_b2 << 15) |
			    ((uint64_t)spd->caslat_b3 << 23) |
			    (((uint64_t)spd->caslat_b4 & 0x3f) << 31);

	if (spd->caslat_b4 & 0x80) {
		lat_mask <<= 16;
	}

	tmp = SPD_TO_PS(spd->taa_min, spd->fine_taa_min);
	tmp = CLOCK_PS(tmp);
	while (tmp < 64 && (lat_mask & (1ULL << tmp)) == 0) {
		++tmp;
	}

	if (tmp * data->tCK > 18000) {
		ERROR("%s: the choosen CAS Latency %u is too large\n", __func__, tmp);
	}

	data->CL = tmp;

#ifdef BAIKAL_DDR_SETTINGS_IN_FLASH
	if ((ddr_storage.flsh_cl) && \
		(ddr_storage.ddr_sign == BAIKAL_FLASH_USE_STR)) {
		data->CL = ddr_storage.flsh_cl;
	}
#endif

	/*
	 * Compute CAS Write Latency (CWL). JEDEC 79-4, page 16, table 6
	 * (we set 1-tCK preamble mode in configuration code)
	 */
	switch (data->clock_mhz * 2) {
	case 1600:
		lat_mask = BIT(9) | BIT(11);
		break;
	case 1866:
		lat_mask = BIT(10) | BIT(12);
		break;
	case 2132:
		lat_mask = BIT(11) | BIT(14);
		break;
	case 2400:
		lat_mask = BIT(12) | BIT(16);
		break;
	case 2666:
		lat_mask = BIT(14) | BIT(18);
		break;
	case 2932:
	case 3200:
		lat_mask = BIT(16) | BIT(20);
		break;
	default:
		lat_mask = BIT(18);
		break;
	}

	tmp = data->CL;
	while (tmp > 0 && (lat_mask & (1ULL << tmp)) == 0) {
		--tmp;
	}

	data->CWL = tmp;

	/*
	 * Compute Write Command Latency when CRC and DM are both enabled (WCL)
	 * JEDEC 79-4 page 17 table 8
	 */
	data->WCL = 4;
	if (data->clock_mhz * 2 > 1600) {
		++data->WCL;
	}
	if (data->clock_mhz * 2 > 2666) {
		++data->WCL;
	}
	if (data->clock_mhz * 2 > 3200) {
		++data->WCL;
	}

	/* Compute C/A Parity Latency (PL): JEDEC 79-4, page 21, table 12 */
	if (data->clock_mhz * 2 < 1600) {
		data->PL = 0;
	} else if (data->clock_mhz * 2 <= 2132) {
		data->PL = 4;
	} else if (data->clock_mhz * 2 <= 2666) {
		data->PL = 5;
	} else if (data->clock_mhz * 2 <= 3200) {
		data->PL = 6;
	} else {
		data->PL = 8;
	}

	/* Compute Additive Latency (AL). Note: we set AL to (CL-1) like S.Hudchenko example */
	data->AL = data->CL - 1;

	/* Compute Read Latency (RL) */
	data->RL = data->AL + data->CL + data->PL;

	/* Compute Write Latency (WL) */
	data->WL = data->AL + data->CWL + data->PL;

	/* Get Write Recovery Time (tWR) */
	tmp = SPD_TO_PS(((spd->twr_min_ext & 0xf) << 8) | spd->twr_min_lsb, 0);
	data->tWR = CLOCK_PS(MAX(tmp, 15000U)); /* if WR left empty in spd */

	/* Get Active to Precharge Delay Time (tRAS) */
	tmp = SPD_TO_PS(((spd->tras_trc_ext & 0xf) << 8) | spd->tras_min_lsb, 0);
	data->tRAS = CLOCK_PS(tmp);

#ifdef BAIKAL_DDR_SETTINGS_IN_FLASH
	if ((ddr_storage.flsh_tras) && \
		(ddr_storage.ddr_sign == BAIKAL_FLASH_USE_STR)) {
		data->tRAS = ddr_storage.flsh_tras;
	}
#endif

	/* Get Row Precharge Delay Time (tRP) */
	tmp = SPD_TO_PS(spd->trp_min, spd->fine_trp_min);
	data->tRP = CLOCK_PS(tmp);

#ifdef BAIKAL_DDR_SETTINGS_IN_FLASH
	if ((ddr_storage.flsh_trp) && \
		(ddr_storage.ddr_sign == BAIKAL_FLASH_USE_STR)) {
		data->tRP = ddr_storage.flsh_trp;
	}
#endif

	/* Get Active to Active/Refresh Delay Time (tRC) */
	tmp = SPD_TO_PS(((spd->tras_trc_ext & 0xf0) << 4) | spd->trc_min_lsb, spd->fine_trc_min);
	data->tRC = CLOCK_PS(tmp);

	/* Get Write to Read Time (tWTR_L), same bank group */
	tmp = SPD_TO_PS(((spd->twtr_min_ext & 0xf0) << 4) | spd->twtrl_min_lsb, 0);
	data->tWTR_L = CLOCK_PS(MAX(tmp, 7500U));

	/* Get Write to Read Time (tWTR_S), different bank group */
	tmp = SPD_TO_PS(((spd->twtr_min_ext & 0xf) << 8) | spd->twtrs_min_lsb, 0);
	data->tWTR_S = CLOCK_PS(MAX(tmp, 2500U));

	/* Get RAS to CAS Delay Time (tRCD) */
	tmp = SPD_TO_PS(spd->trcd_min, spd->fine_trcd_min);
	data->tRCD = CLOCK_PS(tmp);

#ifdef BAIKAL_DDR_SETTINGS_IN_FLASH
	if ((ddr_storage.flsh_trcd) && \
		(ddr_storage.ddr_sign == BAIKAL_FLASH_USE_STR)) {
		data->tRCD = ddr_storage.flsh_trcd;
	}
#endif

	/* Get Activate to Activate Delay Time (tRRD_L), same bank group */
	tmp = SPD_TO_PS(spd->trrdl_min, spd->fine_trrdl_min);
	data->tRRD_L = MAX(4U, CLOCK_PS(tmp));

	/* Get Activate to Activate Delay Time (tRRD_S), different bank group */
	tmp = SPD_TO_PS(spd->trrds_min, spd->fine_trrds_min);
	data->tRRD_S = MAX(4U, CLOCK_PS(tmp));

	/* Get Refresh Recovery Delay Time, 1x mode (tRFC1) */
	tmp = SPD_TO_PS((spd->trfc1_min_msb << 8) | spd->trfc1_min_lsb, 0);
	data->tRFC1 = CLOCK_PS(tmp);

	/* Get Refresh Recovery Delay Time, 4x mode (tRFC4) */
	tmp = SPD_TO_PS((spd->trfc4_min_msb << 8) | spd->trfc4_min_lsb, 0);
	data->tRFC4 = CLOCK_PS(tmp);

	/* Get Four Activate Window Delay Time (tFAW) */
	tmp = SPD_TO_PS(((spd->tfaw_msb & 0xf) << 8) | spd->tfaw_min, 0);
	switch (data->device_width) {
	case 16:
		data->tFAW = MAX(28U, CLOCK_PS(tmp));
		break;
	case 8:
		data->tFAW = MAX(20U, CLOCK_PS(tmp));
		break;
	default:
		data->tFAW = MAX(16U, CLOCK_PS(tmp));
		break;
	}

#ifdef BAIKAL_DDR_SETTINGS_IN_FLASH
	if ((ddr_storage.flsh_tfaw) && \
		(ddr_storage.ddr_sign == BAIKAL_FLASH_USE_STR)) {
		data->tFAW = ddr_storage.flsh_tfaw;
	}
#endif

	/* Get Average Periodic Refresh Interval (tREFI) */
	data->tREFI = CLOCK_NS(7800);

	/* Compute DLL Locking time (tDLLK): DLL lock. JEDEC 79-4, page 22, table 13 */
	if (data->clock_mhz * 2 <= 1866) {
		data->tDLLK = 597;
	} else if (data->clock_mhz * 2 <= 2400) {
		data->tDLLK = 768;
	} else if (data->clock_mhz * 2 <= 2666) {
		data->tDLLK = 854;
	} else if (data->clock_mhz * 2 <= 2932) {
		data->tDLLK = 940;
	} else {
		data->tDLLK = 1024;
	}

	/* Compute Exit Power Down with DLL on (tXP) */
	data->tXP = MAX(4U, CLOCK_NS(6));

	/* Compute Read to Precharge for autoprecharge (tRTP) */
	data->tRTP = MAX(4U, CLOCK_PS(7500));

	/*
	 * Compute Write Recovery time when CRC and DM are enabled (tWR_CRC_DM)
	 *
	 * Compute Write to Read time for different bank group with both CRC and
	 * DM enabled (tWTR_S_CRC_DM)
	 *
	 * Compute Write to Read time for same bank group with both CRC and DM
	 * enabled (tWTR_L_CRC_DM)
	 */
	if (data->clock_mhz * 2 <= 1600) {
		tmp = MAX(4U, CLOCK_PS(3750));
	} else {
		tmp = MAX(5U, CLOCK_PS(3750));
	}
	data->tWR_CRC_DM = data->tWR + tmp;
	data->tWTR_S_CRC_DM = data->tWTR_S + tmp;
	data->tWTR_L_CRC_DM = data->tWTR_L + tmp;

	/* Compute Mode Register Set time (tMOD) */
	data->tMOD = MAX(24U, CLOCK_NS(15));

	/*
	 * Compute Valid Clock Requirement before Self Refresh Exit or
	 * Power-Down Exit or Reset Exit (tCKSRX)
	 */
	data->tCKSRX = MAX(5U, CLOCK_NS(10));

	/* Compute CKE minimum pulse width (tCKE) */
	data->tCKE = MAX(3U, CLOCK_NS(5));

	/* Compute Mode Register Set command cycle time in PDA mode (tMRD_PDA) */
	data->tMRD_PDA = MAX(16U, CLOCK_NS(10));

	/* Get CAS to CAS Delay Time (tCCD_L), same bank group */
	tmp = SPD_TO_PS(spd->tccdl_min, spd->fine_tccdl_min);
	data->tCCD_L = MAX(5U, CLOCK_PS(tmp));

	/* CAS_n to CAS_n command Delay (different BG). Constant for all speedbins */
	data->tCCD_S = 4;
	/* CS setup time to CKE. Get this parameter from SDRAM chip datasheet */
	data->tMPX_S = 1;
	/* CS_n Low hold time to CKE rising edge. JEDEC 79-4, page 134 */
	data->tMPX_LH = CLOCK_NS(12);
	/* Command pass disable delay. JEDEC 79-4, page 190, table 101,102 */
	data->tCPDED = 4;

	/* Compute Exit Self Refresh to commands not requiring a locked DLL (tXS) */
	data->tXS = data->tRFC1 + CLOCK_NS(10);
	/* Compute Exit Self Refresh to ZQCL,ZQCS and MRS (tXS_FAST) */
	data->tXS_FAST = data->tRFC4 + CLOCK_NS(10);

	/* Compute SRX to commands not requiring a locked DLL in Self Refresh ABORT (tXS_ABORT) */
	data->tXS_ABORT = data->tRFC4 + CLOCK_NS(10);

	/* Compute Exit Self Refresh to tXSDLL commands requiring a locked DLL (tXSDLL) */
	data->tXSDLL = data->tDLLK;
	/* Compute Exit MPSM to commands not requiring a locked DLL (tXMP) */
	data->tXMP = data->tXS;
	/* Compute Exit MPSM to commands requiring a locked DLL (tXMPDLL) */
	data->tXMPDLL = data->tXMP + data->tXSDLL;
	/* Compute Valid clock requirement after MPSM entry (tCKMPE) */
	data->tCKMPE = data->tMOD + data->tCPDED;
	/* Compute Maximum ACT to PRE command period (tRASmax) */
	data->tRASmax = data->tREFI * 9;

	return 0;
}
