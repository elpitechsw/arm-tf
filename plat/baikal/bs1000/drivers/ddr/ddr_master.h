/*
 * Copyright (c) 2022-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DDR_MASTER_H
#define DDR_MASTER_H

#include "ddr_main.h"
#include "ddr_spd.h"

#define MR0_DLL_RESET      0  /* DLL reset no */
#define MR0_RD_BURST_TYPE  0  /* read burst type sequential */
#define MR0_BURST_LENGTH   0  /* BL8 */
#define MR1_Q_OFF          0  /* output buffer enabled */
#define MR1_TDQS           0  /* TDGS disable */
#define MR1_WL             0  /* Write Leveling training disable */
#define MR1_DLL_ENDIS      1  /* DLL enable */
#define MR2_LP_ASR         0  /* Low Power Auto Self Refresh: Manual Mode (normal operating temperature range) */
#define MR3_MPR_RD_FORMAT  0  /* MPR Read Format: serial */
#define MR3_TEMP_RDOUT     0  /* Temperature Sensor Readout: disable */
#define MR3_PDA_MODE       0  /* Per DRAM Addressability: disable */
#define MR3_GEARDOWN_MODE  0  /* GearDown Mode: 1/2 rate */
#define MR3_MPR_OPERATION  0  /* MPR Operation: normal */
#define MR3_MPR_PAGE0      0  /* MPR Page: 0 */
#define MR4_HPPR           0  /* Post Package Repair: disable */
#define MR4_RD_PREAMBLE_TR 0  /* Read  Preamble Training Mode: disable */
#define MR4_SREF_ABORT     0  /* Self Refresh Abort: disable */
#define MR4_CS_CAL_DIS     0  /* CS to CMD/ADDR Latency mode: disable */
#define MR4_SPPR           0  /* Soft Post Package Repair: disable */
#define MR4_INTVREFMON     0  /* Internal Vref Monitor: disable */
#define MR4_TCREF_MODE     0  /* Temperature Controlled Refresh Mode:  disable */
#define MR4_TCREF_RANGE    0  /* Temperature Controlled Refresh Range: normal */
#define MR4_MAX_PD_MODE    0  /* Maximum Power Down Mode:  disable */
#define MR5_RD_DBI         0  /* Read  DBI: disable */
#define MR5_WR_DBI         0  /* Write DBI: disable */
#define MR5_CA_PARITY_PE   0  /* CA Parity Persistent Error: disable */
#define MR5_ODT_BUF_PD     0  /* ODT Input Buffer during Power Down mode: is activated */
#define MR5_CA_PAR_ERR_ST  0  /* CA Parity Error Status: clear */
#define MR5_CRC_ERR_CLEAR  0  /* CRC Error Clear: clear */
#define MR6_VREF_DQ_TR     0  /* Vref DQ Training: disable */

#define SPD_TO_PS(mtb, ftb)	((mtb) * 125 + (ftb))

#define CLOCK_PS(x)	\
	(uint32_t)((((uint64_t)(x) * 1000) / data->tCK + 974) / 1000)
#define CLOCK_NS(x)	CLOCK_PS((uint64_t)(x) * 1000)

uint16_t set_mr0(const uint32_t cl, uint32_t wr);
uint16_t set_mr1(uint32_t odi, uint32_t rtt_nom);
uint16_t set_mr2(uint32_t cwl, uint32_t rtt_wr, uint32_t ddr_crc);
uint16_t set_mr3(uint32_t wcl, uint32_t fg_rfsh);
uint16_t set_mr4(uint32_t wr_pre_2ck, uint32_t rd_pre_2ck);
uint16_t set_mr5(uint32_t pl, uint32_t rtt_park, uint32_t ddr_dm);
uint16_t set_mr6(uint32_t tCCD_L, uint32_t phy_vref);

int ddr_config_by_spd(int port,
			struct ddr4_spd_eeprom *spd,
			struct ddr_configuration *data);

#endif /* DDR_MASTER_H */
