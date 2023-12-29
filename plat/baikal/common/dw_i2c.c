/*
 * Copyright (c) 2020-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <drivers/delay_timer.h>

#include <dw_i2c.h>

struct i2c_regs {
	uint32_t ic_con;
	uint32_t ic_tar;
	uint32_t ic_sar;
	uint32_t ic_hs_maddr;
	uint32_t ic_data_cmd;
	uint32_t ic_ss_scl_hcnt;
	uint32_t ic_ss_scl_lcnt;
	uint32_t ic_fs_scl_hcnt;
	uint32_t ic_fs_scl_lcnt;
	uint32_t ic_hs_scl_hcnt;
	uint32_t ic_hs_scl_lcnt;
	uint32_t ic_intr_stat;
	uint32_t ic_intr_mask;
	uint32_t ic_raw_intr_stat;
	uint32_t ic_rx_tl;
	uint32_t ic_tx_tl;
	uint32_t ic_clr_intr;
	uint32_t ic_clr_rx_under;
	uint32_t ic_clr_rx_over;
	uint32_t ic_clr_tx_over;
	uint32_t ic_clr_rd_req;
	uint32_t ic_clr_tx_abrt;
	uint32_t ic_clr_rx_done;
	uint32_t ic_clr_activity;
	uint32_t ic_clr_stop_det;
	uint32_t ic_clr_start_det;
	uint32_t ic_clr_gen_call;
	uint32_t ic_enable;
	uint32_t ic_status;
	uint32_t ic_txflr;
	uint32_t ic_rxflr;
	uint32_t ic_sda_hold;
	uint32_t ic_tx_abrt_source;
	uint32_t reserved0;
	uint32_t ic_dma_cr;
	uint32_t ic_dma_tdlr;
	uint32_t ic_dma_rdlr;
	uint32_t ic_sda_setup;
	uint32_t ic_ack_general_call;
	uint32_t ic_enable_status;
	uint32_t ic_fs_spklen;
	uint32_t ic_hs_spklen;
};

#define IC_SPEED_MODE_FAST		2
#define IC_CON_MASTER_MODE		BIT(0)
#define IC_CON_SPEED			(IC_SPEED_MODE_FAST << 1)
#define IC_CON_IC_SLAVE_DISABLE		BIT(6)

#define IC_DATA_CMD_CMD			BIT(8)
#define IC_DATA_CMD_STOP		BIT(9)

#define IC_RAW_INTR_STAT_TX_ABRT	BIT(6)

#define IC_ENABLE_ENABLE		BIT(0)

#define IC_STATUS_TFNF			BIT(1)
#define IC_STATUS_TFE			BIT(2)
#define IC_STATUS_RFNE			BIT(3)
#define IC_STATUS_MST_ACTIVITY		BIT(5)

#define IC_ENABLE_STATUS_IC_EN		BIT(0)

#define MIN_FS_SCL_HIGHTIME		600
#define MIN_FS_SCL_LOWTIME		1300

int i2c_txrx(const uintptr_t base,
	     const unsigned int iclk,
	     const unsigned int targetaddr,
	     const void *const txbuf,
	     const unsigned int txbufsize,
	     void *const rxbuf,
	     const unsigned int rxbufsize)
{
	uint64_t activity_timeout;
	int err;
	volatile struct i2c_regs *const i2cregs = (volatile struct i2c_regs *const)base;
	unsigned int rxedsize = 0;
	uint8_t *const rxptr = (uint8_t *)rxbuf;
	unsigned int txedsize = 0;
	const uint8_t *const txptr = (uint8_t *)txbuf;

	assert(i2cregs != NULL);
	assert(targetaddr <= 0x7f);
	assert(txbuf != NULL || !txbufsize);
	assert(rxbuf != NULL || !rxbufsize);

	i2cregs->ic_enable	= 0;
	i2cregs->ic_con		= IC_CON_IC_SLAVE_DISABLE | IC_CON_SPEED | IC_CON_MASTER_MODE;
	i2cregs->ic_tar		= targetaddr;
	i2cregs->ic_rx_tl	= 0;
	i2cregs->ic_tx_tl	= 0;
	i2cregs->ic_intr_mask	= 0;
	i2cregs->ic_fs_scl_hcnt	= ((uint64_t)iclk * MIN_FS_SCL_HIGHTIME + 1000000000 - 1) / 1000000000;
	assert(i2cregs->ic_fs_scl_hcnt + 5 > i2cregs->ic_fs_spklen);
	i2cregs->ic_fs_scl_lcnt	= ((uint64_t)iclk * MIN_FS_SCL_LOWTIME  + 1000000000 - 1) / 1000000000;
	assert(i2cregs->ic_fs_scl_lcnt + 7 > i2cregs->ic_fs_spklen);
	i2cregs->ic_enable	= IC_ENABLE_ENABLE;
	activity_timeout	= timeout_init_us(100000);

	for (;;) {
		const unsigned int ic_status = i2cregs->ic_status;

		if (rxedsize < rxbufsize && (ic_status & IC_STATUS_RFNE)) {
			rxptr[rxedsize++] = i2cregs->ic_data_cmd;
			activity_timeout = timeout_init_us(100000);
			continue;
		}

		if (i2cregs->ic_raw_intr_stat & IC_RAW_INTR_STAT_TX_ABRT) {
			err = -1;
			break;
		} else if (txedsize < txbufsize + rxbufsize) {
			if (ic_status & IC_STATUS_TFNF) {
				/*
				 * Driver must set STOP bit if IC_EMPTYFIFO_HOLD_MASTER_EN
				 * is set. However, IC_EMPTYFIFO_HOLD_MASTER_EN cannot be
				 * detected from the registers. So the STOP bit is always set
				 * when writing/reading the last byte.
				 */
				const uint32_t stop = txedsize < txbufsize + rxbufsize - 1 ?
						      0 : IC_DATA_CMD_STOP;

				if (txedsize < txbufsize) {
					i2cregs->ic_data_cmd = stop | txptr[txedsize];
				} else {
					i2cregs->ic_data_cmd = stop | IC_DATA_CMD_CMD;
				}

				activity_timeout = timeout_init_us(100000);
				++txedsize;
			} else if (!(ic_status & IC_STATUS_MST_ACTIVITY) &&
					timeout_elapsed(activity_timeout)) {
				err = -1;
				break;
			}
		} else if ((ic_status & IC_STATUS_TFE) &&
			  !(ic_status & IC_STATUS_MST_ACTIVITY)) {
			err = 0;
			break;
		}
	}

	i2cregs->ic_enable = 0;

	while (i2cregs->ic_enable_status & IC_ENABLE_STATUS_IC_EN)
		;

	if (err) {
		return -1;
	}

	return rxedsize;
}
