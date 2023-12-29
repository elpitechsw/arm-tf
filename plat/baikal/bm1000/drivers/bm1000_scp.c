/*
 * Copyright (c) 2018-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <baikal_def.h>
#include <baikal_scp.h>
#include <platform_def.h>

#pragma pack(1)
struct scp_service {
	uint32_t	req_id;
	uint32_t	exec_id;
	uint8_t		op;
	uint8_t		st;
	uint16_t	size;
	uint32_t	offset;
	uint32_t	buf[0];
};
#pragma pack()

void *scp_buf(void)
{
	volatile struct scp_service *const scp =
		(volatile struct scp_service *const)SCP_SERVICE_BASE;

	return (void *)scp->buf;
}

static bool scp_busy(void)
{
	volatile struct scp_service *const scp =
		(volatile struct scp_service *const)SCP_SERVICE_BASE;

	return mmio_read_32(MAILBOX_IRB0_SCP2AP_STS) ||
		scp->req_id != scp->exec_id ||
		(scp->st != 'E' && scp->st != 'G');
}

int scp_cmd(uint8_t op, uint32_t arg0, uint32_t arg1)
{
	volatile struct scp_service *const scp =
		(volatile struct scp_service *const)SCP_SERVICE_BASE;

	int err;
	static uint32_t id = 1;
	char s_op[2];
	char s_st[2];
	uint64_t timeout;

	if (scp_busy()) {
		err = -EBUSY;
		goto err;
	}

	switch (op) {
	case 'L':
		scp->op = op;
		scp->buf[0] = arg0;
		break;
	case 'V':
		scp->op = op;
		scp->buf[0] = arg0;
		scp->buf[1] = arg1;
		break;
	case 'R':
	case 'W':
	case 'E':
		scp->op = op;
		scp->offset = arg0 + BAIKAL_SCP_MAX_SIZE;
		scp->size   = (uint16_t)arg1;
		break;
	case 'I':
	case 'M':
	case 'N':
	case 'T':
	case 't':
		scp->op = op;
		break;
	default:
		err = -EINVAL;
		goto err;
	}

	scp->req_id = id++;

	dmbst();

	mmio_write_32(MAILBOX_IRB0_AP2SCP_SET, 1); /* send signal */

	timeout = timeout_init_us(1000000);
	while (scp_busy()) {
		if (timeout_elapsed(timeout)) {
			err = -ETIMEDOUT;
			goto err;
		}
	}

	if (scp->st != 'G') {
		err = -EFAULT;
		goto err;
	}

	dmbsy();
	return 0;

err:
	s_op[0] = op;
	s_op[1] = '\0';
	s_st[0] = scp->st;
	s_st[1] = '\0';

	ERROR("%s %s (req:%u exec:%u op:%s st:%s arg0:0x%x arg1:0x%x)\n",
		__func__,
		err == -EFAULT	  ? "EFAULT"	:
		err == -EBUSY	  ? "EBUSY"	:
		err == -EINVAL	  ? "EINVAL"	:
		err == -ETIMEDOUT ? "ETIMEDOUT"	: "???",
		scp->req_id, scp->exec_id, s_op, s_st, arg0, arg1);

	return err;
}
