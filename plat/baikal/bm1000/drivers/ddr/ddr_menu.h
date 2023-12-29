/*
 * Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _DDR_MENU_H_
#define _DDR_MENU_H_

#include <baikal_def.h>
#include "ddr_main.h"

#define BAIKAL_STR_PORT_OFFS		BAIKAL_BOOT_SPI_SUBSECTOR
#define BAIKAL_UEFI_DDR_FLAG_OFFSET	(BAIKAL_VAR_OFFSET - \
						(BAIKAL_STR_PORT_OFFS * 2))

#define FLASH_SPEEDBIN_2400		0x3f77
#define FLASH_SPEEDBIN_2133		0x3e33
#define FLASH_SPEEDBIN_1866		0x1622
#define FLASH_SPEEDBIN_1600		0x1600

#define BAIKAL_FLASH_USE_SPD		0xd182
#define BAIKAL_FLASH_USE_STR		0xf1a3

#define BAIKAL_DIC_RZQ_DIV_7		0x0fff
#define BAIKAL_DIC_RZQ_DIV_5		0x0f0f

#define BAIKAL_RTTWR_DYN_OFF		0x7fff
#define BAIKAL_RTTWR_RZQ_DIV_4		0x07ff
#define BAIKAL_RTTWR_RZQ_DIV_2		0x03ff
#define BAIKAL_RTTWR_HI_Z		0x01ff
#define BAIKAL_RTTWR_RZQ_DIV_3		0x00ff

#define BAIKAL_RTTNOM_RZQ_DIS		0x7fff
#define BAIKAL_RTTNOM_RZQ_DIV_4		0x07ff
#define BAIKAL_RTTNOM_RZQ_DIV_2		0x03ff
#define BAIKAL_RTTNOM_RZQ_DIV_6		0x01ff
#define BAIKAL_RTTNOM_RZQ_DIV_1		0x00ff
#define BAIKAL_RTTNOM_RZQ_DIV_5		0x007f
#define BAIKAL_RTTNOM_RZQ_DIV_3		0x003f
#define BAIKAL_RTTNOM_RZQ_DIV_7		0x0037

#define BAIKAL_RTTPARK_RZQ_DIS		0x7fff
#define BAIKAL_RTTPARK_RZQ_DIV_4	0x07ff
#define BAIKAL_RTTPARK_RZQ_DIV_2	0x03ff
#define BAIKAL_RTTPARK_RZQ_DIV_6	0x01ff
#define BAIKAL_RTTPARK_RZQ_DIV_1	0x00ff
#define BAIKAL_RTTPARK_RZQ_DIV_5	0x007f
#define BAIKAL_RTTPARK_RZQ_DIV_3	0x003f
#define BAIKAL_RTTPARK_RZQ_DIV_7	0x0037

struct ddr_flash_config {
	uint16_t speedbin;
	uint16_t ddr_sign;
	uint16_t flsh_dic;
	uint16_t flsh_rtt_wr;
	uint16_t flsh_rtt_nom;
	uint16_t flsh_rtt_park;
	uint16_t flsh_cl;
	uint16_t flsh_trcd;
	uint16_t flsh_trp;
	uint16_t flsh_tras;
	uint16_t flsh_tfaw;
	uint16_t flsh_1t2t;
	uint16_t flsh_host_v;
	uint16_t flsh_dram_v;
	uint16_t flsh_vref_use;
};

int ddr_flash_conf_load(int port);
int ddr_flash_conf_store(int port, struct ddr_configuration *data);

uint32_t ddr_flash_tmg2val(uint16_t tmg_in);
uint16_t ddr_flash_val2tmg(uint32_t val_in);

#endif /* _DDR_MENU_H_ */
