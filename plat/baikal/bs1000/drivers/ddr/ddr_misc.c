/*
 * Copyright (c) 2022, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <drivers/delay_timer.h>

#include "ddr_bs1000.h"
#include "ddr_io.h"
#include "ddr_misc.h"

#define DDRLCRU_GPR_OFFS	0x20000

void ddrlcru_apb_reset_off(int port)
{
	uint32_t val = ddr_io_lcru_read(port, DDRLCRU_GPR_OFFS + RSTMM);

	/* PWROKIN 1 -> 0 (PHY cold reset) */
	val &= ~(1 << 1);
	ddr_io_lcru_write(port, DDRLCRU_GPR_OFFS + RSTMM, val);

	/* DDR SS APB reset 1 -> 0 (DDR configuration APB slave port reset) */
	val &= ~(1 << 3);
	ddr_io_lcru_write(port, DDRLCRU_GPR_OFFS + RSTMM, val);
}

void ddrlcru_core_reset_off(int port)
{
	/* Enable ADB400 */
	uint32_t val = 0x17 << 11;

	ddr_io_lcru_write(port, DDRLCRU_GPR_OFFS + ADBFNCCR, val);

	/* deassert all resets (Deasserting DDR SS Core and AXI Resets) */
	ddr_io_lcru_write(port, DDRLCRU_GPR_OFFS + RSTMM, 0x0);
}

/**@brief get incoming command from LCP (and clear LCRU register)
 * @return: 1 - a valid command, 0 - an empty one
 */
static int ddr_lcpcmd_get_cmd(int port, int *type, int *param)
{
	uint32_t val = ddr_io_gpr_read(port, LCP_TO_SCP_IRQ_REG);

	if (!(val & 0x1)) {
		/* no incoming irq - return none */
		return 0;
	} else {
		/* incoming irq detected */
		/* get command */
		(*type) = val >> 26; /* CMD_OPCODE field */
		(*param) = (val & GENMASK(25, 1)) >> 1; /* CMD_PARAM field */
		/* clear register */
		ddr_io_gpr_write(port, LCP_TO_SCP_IRQ_REG, 0x0);
		return 1;
	}
}

/** @brief send outgoing command to LCP
 */
static void ddr_lcpcmd_send_cmd(int port, int type, int param)
{
	/* force clear LCP back response register */
	ddr_io_gpr_write(port, LCP_TO_SCP_IRQ_REG, 0x0);

	uint32_t val;
	/* set command */
	val  = type << 26;
	val |= (param & GENMASK(24, 0)) << 1;
	val |= 1 << 0;
	/* set register */
	ddr_io_gpr_write(port, SCP_TO_LCP_IRQ_REG, val);
}

/** @brief wait for LCP command response
 */
static int ddr_lcpcmd_wait_response(int port)
{
	int param;
	int type;
	/* wait for initialization done status from LCP */
	for (uint64_t timeout = timeout_init_us(DDR_LCPCMD_TIMEOUT);;) {
		if (ddr_lcpcmd_get_cmd(port, &type, &param)) {
			break;
		} else if (timeout_elapsed(timeout)) {
			/* command timeout */
			return 1;
		}
	}
	if (type != MSG_SEND_STATUS) {
		/* invalid command type response */
		return 1;
	}
	if (param) {
		/* invalid command return code */
		return 1;
	}
	return 0;
}

/** @brief send DDR_SPEEDBIN command to LCP and wait for response
 */
int ddr_lcpcmd_set_speedbin(int port, int clock_mhz)
{
	/* send command to LCP */
	uint32_t mode;
	enum cmd_type_t type = CMD_DDRSPEEDBIN_SET;

	switch (clock_mhz) {
	case DDRSB_1600:
		mode = CMU_DDR4_1600;
		break;
	case DDRSB_1866:
		mode = CMU_DDR4_1866;
		break;
	case DDRSB_2133:
		mode = CMU_DDR4_2133;
		break;
	case DDRSB_2400:
		mode = CMU_DDR4_2400;
		break;
	case DDRSB_2666:
		mode = CMU_DDR4_2666;
		break;
	case DDRSB_2933:
		mode = CMU_DDR4_2933;
		break;
	case DDRSB_3200:
		mode = CMU_DDR4_3200;
		break;
	default:
		return 1;
	}

	ddr_lcpcmd_send_cmd(port, type, mode & 0x7);

	/* let a command sink in */
	udelay(400);

	/* wait for response from LCP */
	if (ddr_lcpcmd_wait_response(port)) {
		/* invalid command response */
		return 1;
	}

	return 0;
}
