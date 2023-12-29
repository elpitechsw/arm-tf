/*
 * Copyright (c) 2022-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DDR_MISC_H
#define DDR_MISC_H

#define DDRSB_1600	800
#define DDRSB_1866	933
#define DDRSB_2133	1066
#define DDRSB_2400	1200
#define DDRSB_2666	1333
#define DDRSB_2933	1466
#define DDRSB_3200	1600

#define RSTMM			0x0
#define ADBFNCCR		0x78
#define SCP_TO_LCP_IRQ_REG	0x1d8
#define LCP_TO_SCP_IRQ_REG	0x1d0
#define DDR_LCPCMD_TIMEOUT	300

enum ddr_speed_bin {
	CMU_DDR4_1600 = 0,
	CMU_DDR4_1866,
	CMU_DDR4_2133,
	CMU_DDR4_2400,
	CMU_DDR4_2666,
	CMU_DDR4_2933,
	CMU_DDR4_3200
};

enum cmd_type_t {
	CMD_CLOCKCHANNEL_SET = 0x01,
	CMD_PLL_CLKCH_MM_SET = 0x02,
	CMD_DDRSPEEDBIN_SET  = 0x04,
	CMD_PVTMODE_SET      = 0x08,
	CMD_PVTTIMER_SET     = 0x09,
	CMD_PVTTHR_SET       = 0x0a,
	CMD_JMP_TO           = 0x0c,
	CMD_EXEC_INSTR       = 0x0d,
	CMD_READ_BYTE        = 0x11,
	CMD_WRITE_BYTE       = 0x21,
	CMD_READ_LONG        = 0x14,
	CMD_WRITE_LONG       = 0x24,
	MSG_EXCEPT_OCCURED   = 0x00,
	MSG_PVTTHR_EXCEEDED  = 0x01,
	MSG_INIT_DONE_STATUS = 0x21,
	MSG_SEND_STATUS      = 0x22,
	MSG_INTERFACE_ERROR  = 0x23,
	CMD_UNKNOWN          = 0xff,
};

void ddrlcru_apb_reset_off(int port);
void ddrlcru_core_reset_off(int port);
int ddr_lcpcmd_set_speedbin(int port, int clock_mhz);

#endif /* DDR_MISC_H */
