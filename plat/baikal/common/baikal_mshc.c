/*
 * Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <baikal_mshc.h>
#include <baikal_def.h>

/* TODO: rework */
#if defined(BAIKAL_DBM10) || defined(BAIKAL_DBM20) || \
    defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
#include <bm1000_cmu.h>
#endif

static int reg_size(int Reg)
{
	int size = 0;

	switch (Reg) {
	case SDHCI_DMA_ADDRESS:
	case SDHCI_ARGUMENT:
	case SDHCI_BUFFER:
	case SDHCI_PRESENT_STATE:
	case SDHCI_MSHC_VER:
	case SDHCI_CAPABILITIES:
	case SDHCI_CAPABILITIES_1:
	case SDHCI_MAX_CURRENT:
	case SDHCI_ADMA_ADDRESS:
	case SDHCI_ADMA_ADDRESS_HI:
	case SDHCI_RESPONSE_0:
	case SDHCI_RESPONSE_1:
	case SDHCI_RESPONSE_2:
	case SDHCI_RESPONSE_3:
		size = 4;
		break;

	case SDHCI_TRANSFER_MODE:
	case SDHCI_BLOCK_SIZE:
	case SDHCI_16BIT_BLK_CNT:
	case SDHCI_COMMAND:
	case SDHCI_CLOCK_CONTROL:
	case SDHCI_INT_STATUS:
	case SDHCI_ERR_STATUS:
	case SDHCI_INT_ENABLE:
	case SDHCI_ERR_ENABLE:
	case SDHCI_SIGNAL_ENABLE:
	case SDHCI_ERR_SIGNAL_ENABLE:
	case SDHCI_AUTO_CMD_STATUS:
	case SDHCI_HOST_CONTROL2:
	case SDHCI_SET_INT_ERROR:
	case SDHCI_SET_ACMD12_ERROR:
	case SDHCI_PRESET_INIT:
	case SDHCI_PRESET_DS:
	case SDHCI_PRESET_HS:
	case SDHCI_PRESET_FOR_SDR12:
	case SDHCI_PRESET_FOR_SDR25:
	case SDHCI_PRESET_FOR_SDR50:
	case SDHCI_PRESET_FOR_SDR104:
	case SDHCI_PRESET_FOR_DDR50:
	case SDHCI_PRESET_FOR_HS400:
	case SDHCI_SLOT_INT_STATUS:
	case SDHCI_HOST_VERSION:
	case SDHCI_EMMC_CONTROL:
		size = 2;
		break;

	case SDHCI_TIMEOUT_CONTROL:
	case SDHCI_SOFTWARE_RESET:
	case SDHCI_ADMA_ERROR:
	case SDHCI_MSHC_CTRL:
	case SDHCI_HOST_CONTROL:
	case SDHCI_POWER_CONTROL:
	case SDHCI_BLOCK_GAP_CONTROL:
	case SDHCI_WAKE_UP_CONTROL:
		size = 1;
		break;
	}

	return size;
}

static int reg_write(uintptr_t base, int Val, int Reg)
{
	switch (reg_size(Reg)) {
	case 4:
		*(uint32_t *) (base + Reg) = Val;
		break;
	case 2:
		*(uint16_t *) (base + Reg) = Val;
		break;
	case 1:
		*(uint8_t *)  (base + Reg) = Val;
		break;
	}

	return -1;
}

static int reg_read(uintptr_t base, int Reg)
{
	switch (reg_size(Reg)) {
	case 4:
		return *(uint32_t *) (base + Reg);
	case 2:
		return *(uint16_t *) (base + Reg);
	case 1:
		return *(uint8_t *)  (base + Reg);
	}

	return -1;
}

static void led_on(uintptr_t base, uint32_t on)
{
	int reg = reg_read(base, SDHCI_HOST_CONTROL);

	if (on) {
		reg &= ~SDHCI_CTRL_LED;
	} else {
		reg |=  SDHCI_CTRL_LED;
	}

	reg_write(base, reg, SDHCI_HOST_CONTROL);
}

static int speed_mode(uintptr_t base, uint32_t mode)
{
	int ctrl1 = reg_read(base, SDHCI_HOST_CONTROL);
	int ctrl2 = reg_read(base, SDHCI_HOST_CONTROL2);

	switch (mode) {
	case SD_DEFAULT:
		ctrl1 &= ~SDHCI_CTRL_HISPD;
		ctrl2 &= ~SDHCI_CTRL_VDD_180;
		break;
	case SD_HIGH:
		ctrl1 |=  SDHCI_CTRL_HISPD;
		ctrl2 &= ~SDHCI_CTRL_VDD_180;
		break;
	case SD_12:
		ctrl2 |=  SDHCI_CTRL_UHS_SDR12;
		ctrl2 |=  SDHCI_CTRL_VDD_180;
		break;
	case SD_25:
		ctrl2 |=  SDHCI_CTRL_UHS_SDR25;
		ctrl2 |=  SDHCI_CTRL_VDD_180;
		break;
	case SD_50:
		ctrl2 |=  SDHCI_CTRL_UHS_SDR50;
		ctrl2 |=  SDHCI_CTRL_VDD_180;
		break;
	case SD_104:
		ctrl2 |=  SDHCI_CTRL_UHS_SDR104;
		ctrl2 |=  SDHCI_CTRL_VDD_180;
		break;
	case DD_50:
		ctrl2 |=  SDHCI_CTRL_UHS_DDR50;
		ctrl2 |=  SDHCI_CTRL_VDD_180;
		break;
	default:
		return -1;
	}

	reg_write(base, ctrl1, SDHCI_HOST_CONTROL);
	reg_write(base, ctrl2, SDHCI_HOST_CONTROL2);
	return 0;
}

static int clock_supply(uintptr_t base, uint32_t Freq)
{
	int ret;
	int Reg;

	/* Disable */
	Reg  = reg_read(base, SDHCI_CLOCK_CONTROL);
	Reg &= ~SDHCI_CLOCK_PLL_EN;
	Reg &= ~SDHCI_CLOCK_CARD_EN;
	reg_write(base, Reg, SDHCI_CLOCK_CONTROL);

	/* TODO: rework */
	/* Config */
#if defined(BM1000_CMU_H)
	int Div = MMAVLSP_PLL_FREQ / (2 * Freq);

	cmu_clkch_enable_by_base(MMAVLSP_CMU0_CLKCHCTL_MSHC_TX_X2, Div);
#elif defined(BS1000_CMU_H)
	/* cmu_clkch_set_rate(MMAVLSP_CMU0_CLKCHCTL_MSHC_TX_X2, MMAVLSP_PLL_FREQ); ?? */
	/* cmu_clkch_enable(MMAVLSP_CMU0_CLKCHCTL_MSHC_TX_X2); ?? */
	return -1;
#elif defined(BL1000_CMU_H)
	return -1;
#else
	return -1;
#endif /* CMU_H */

	/* Wait */
	ret = WAIT(!(reg_read(base, SDHCI_CLOCK_CONTROL) & SDHCI_CLOCK_STABLE));
	if (ret) {
		return -1;
	}

	/* Enable */
	Reg  = reg_read(base, SDHCI_CLOCK_CONTROL);
	Reg |= SDHCI_CLOCK_PLL_EN;
	Reg |= SDHCI_CLOCK_CARD_EN;
	reg_write(base, Reg, SDHCI_CLOCK_CONTROL);

	/* Reset */
	reg_write(base, SDHCI_RESET_CMD,  SDHCI_SOFTWARE_RESET);
	reg_write(base, SDHCI_RESET_DATA, SDHCI_SOFTWARE_RESET);
	udelay(1000);
	ret = WAIT(reg_read(base, SDHCI_SOFTWARE_RESET));
	if (ret) {
		return ret;
	}

	udelay(1000);
	return ret;
}

static int config_width(uintptr_t base, uint32_t width)
{
	int ctrl = reg_read(base, SDHCI_HOST_CONTROL);
	reg_ctl1_t *reg = (void *)&ctrl;

	switch (width) {
	case MMC_BUS_WIDTH_1:
		reg->width     = 0;
		reg->ext_width = 0;
		break;
	case MMC_BUS_WIDTH_4:
		reg->width     = 1;
		reg->ext_width = 0;
		break;
	case MMC_BUS_WIDTH_8:
		reg->width     = 0;
		reg->ext_width = 1;
		break;
	default:
		return -1;
	}

	reg_write(base, ctrl, SDHCI_HOST_CONTROL);
	return 0;
}

static void reset(uintptr_t base)
{
	reg_write(base, SDHCI_POWER_OFF, SDHCI_POWER_CONTROL);
	udelay(1000);

	/* irq */
	reg_write(base, 0xffff, SDHCI_INT_STATUS);
	reg_write(base, 0xffff, SDHCI_ERR_STATUS);
	reg_write(base, 0xffff, SDHCI_INT_ENABLE);
	reg_write(base, 0xffff, SDHCI_ERR_ENABLE);
	reg_write(base, 0x0000, SDHCI_SIGNAL_ENABLE);
	reg_write(base, 0x0000, SDHCI_ERR_SIGNAL_ENABLE);

	/* config */
	reg_write(base, 0, SDHCI_HOST_CONTROL);
	reg_write(base,
		SDHCI_CTRL_V4_MODE |
		SDHCI_CTRL_64BIT_ADDR |
		SDHCI_CTRL_ASYNC,
		SDHCI_HOST_CONTROL2);
	reg_write(base, 0, SDHCI_TRANSFER_MODE);
	reg_write(base, 0, SDHCI_16BIT_BLK_CNT);
	reg_write(base, 0, SDHCI_32BIT_BLK_CNT);
	reg_write(base, 0, SDHCI_ARGUMENT);
	reg_write(base, 0, SDHCI_COMMAND);
	reg_write(base, SDHCI_DEFAULT_BLOCK_SIZE, SDHCI_BLOCK_SIZE);
	reg_write(base, SDHCI_TIMEOUT_DEFAULT, SDHCI_TIMEOUT_CONTROL);

	/* clock */
	int reg;

	reg = reg_read(base, SDHCI_CLOCK_CONTROL);
	reg &= SDHCI_CLOCK_EN;
	reg &= SDHCI_CLOCK_CARD_EN;
	reg &= SDHCI_CLOCK_PLL_EN;
	reg_write(base, reg, SDHCI_CLOCK_CONTROL);
	udelay(1000);

	led_on(base, false);
}

static int init_host(uintptr_t base)
{
	reset(base);
	reg_write(base, SDHCI_POWER_330 | SDHCI_POWER_ON, SDHCI_POWER_CONTROL);
	reg_write(base, SDHCI_CLOCK_EN, SDHCI_CLOCK_CONTROL);
	udelay(1000);
	return 0;
}

/*
 * ops
 */
static int dw_send_cmd(struct mmc_cmd *cmd)
{
	int ret;
	uintptr_t base = MMAVLSP_EMMC_BASE;
	uint32_t CmdRaw = 0;
	reg_cmd_t *Cmd = (void *)&CmdRaw;

	/* busy */
	ret = WAIT(reg_read(base, SDHCI_PRESENT_STATE) & (SDHCI_CMD_INHIBIT | SDHCI_DATA_INHIBIT));
	if (ret) {
		goto exit;
	}

	/* clean */
	reg_write(base, 0xffff, SDHCI_INT_STATUS);
	reg_write(base, 0xffff, SDHCI_ERR_STATUS);
	reg_write(base, 0x0, SDHCI_RESPONSE_0);
	reg_write(base, 0x0, SDHCI_RESPONSE_1);
	reg_write(base, 0x0, SDHCI_RESPONSE_2);
	reg_write(base, 0x0, SDHCI_RESPONSE_3);

	/* cmd */
	Cmd->index = cmd->cmd_idx;
	if (Cmd->index == MMC_CMD(14) ||
		Cmd->index == MMC_CMD(19) ||
		Cmd->index == MMC_CMD(17) ||
		Cmd->index == MMC_CMD(18) ||
		Cmd->index == MMC_CMD(21) ||
		Cmd->index == MMC_CMD(24) ||
		Cmd->index == MMC_CMD(25) ||
		Cmd->index == MMC_CMD(26) ||
		Cmd->index == MMC_CMD(27) ||
		Cmd->index == MMC_CMD(51) ||
		Cmd->index == MMC_CMD(49)) {
		Cmd->data_present = 1;   /* 0-nodata, 1-data */
	}

	/* mode */
	uint32_t Mode = reg_read(base, SDHCI_TRANSFER_MODE);
	reg_mode_t *mode = (void *)&Mode;

	if (Cmd->index == MMC_CMD(24) || Cmd->index == MMC_CMD(25)) {
		mode->xfer_dir = 0;
	} else {
		mode->xfer_dir = 1;
	}

	reg_write(base, Mode, SDHCI_TRANSFER_MODE);

	if (cmd->resp_type & MMC_RSP_136) {
		Cmd->resp_type = SDHCI_CMD_RESP_LONG;
	} else if (cmd->resp_type & (MMC_RSP_48 | MMC_RSP_BUSY)) {
		Cmd->resp_type = SDHCI_CMD_RESP_SHORT_BUSY;
	} else if (cmd->resp_type & MMC_RSP_48) {
		Cmd->resp_type = SDHCI_CMD_RESP_SHORT;
	} else {
		Cmd->resp_type = SDHCI_CMD_RESP_NONE;
	}
	if (cmd->resp_type & MMC_RSP_CRC) {
		Cmd->crc_chk = 1;
	}
	if (cmd->resp_type & MMC_RSP_CMD_IDX) {
		Cmd->index_chk = 1;
	}

	/* exec */
	led_on(base, true);
	reg_write(base, cmd->cmd_arg, SDHCI_ARGUMENT);
	reg_write(base, CmdRaw, SDHCI_COMMAND);

	/* resp */
	ret = WAIT(!(reg_read(base, SDHCI_INT_STATUS) & SDHCI_INT_RESPONSE));
	reg_write(base, SDHCI_INT_RESPONSE, SDHCI_INT_STATUS);
	if (ret) {
		goto exit;
	}

	cmd->resp_data[0] = reg_read(base, SDHCI_RESPONSE_0);
	cmd->resp_data[1] = reg_read(base, SDHCI_RESPONSE_1);
	cmd->resp_data[2] = reg_read(base, SDHCI_RESPONSE_2);
	cmd->resp_data[3] = reg_read(base, SDHCI_RESPONSE_3);

	/* error */
	if (reg_read(base, SDHCI_ERR_STATUS)) {
		reg_write(base, 0xffff, SDHCI_ERR_STATUS);
		ret = -1;
		goto exit;
	}

exit:
	led_on(base, false);
	return ret;
}

static int dw_prepare(int lba, uintptr_t buf, size_t size)
{
	int ret;
	uintptr_t base = MMAVLSP_EMMC_BASE;
	uint32_t Blocksize = size < SDHCI_DEFAULT_BLOCK_SIZE ? size : SDHCI_DEFAULT_BLOCK_SIZE;
	uint32_t Blocks = size / Blocksize;
	uint32_t Mode = 0;
	reg_mode_t *mode = (void *)&Mode;

	/* busy */
	ret = WAIT(reg_read(base, SDHCI_PRESENT_STATE) & (SDHCI_CMD_INHIBIT | SDHCI_DATA_INHIBIT));
	if (ret) {
		return -3;
	}

	if (Blocks > 1) {
		mode->block_cnt_en = 1;
		mode->multi_block  = 1;
	}

	reg_write(base, Blocksize, SDHCI_BLOCK_SIZE);
	reg_write(base, Blocks,    SDHCI_32BIT_BLK_CNT);
	reg_write(base, Mode,      SDHCI_TRANSFER_MODE);

	return 0;
}

static int dw_read(int lba, uintptr_t buf, size_t size)
{
	int ret;
	uintptr_t base = MMAVLSP_EMMC_BASE;
	uint32_t j;
	uint32_t Iter;
	uint32_t *P = (void *)buf;
	uint32_t Blocksize = reg_read(base, SDHCI_BLOCK_SIZE);
	uint32_t Blocks    = reg_read(base, SDHCI_32BIT_BLK_CNT);

	led_on(base, true);
	for (j = 0; j < Blocks; j++) {
		ret = WAIT(!(reg_read(base, SDHCI_INT_STATUS) & SDHCI_INT_DATA_AVAIL));
		reg_write(base, SDHCI_INT_DATA_AVAIL, SDHCI_INT_STATUS);
		if (ret) {
			goto exit;
		}
		for (Iter = 0; Iter < Blocksize / sizeof(uint32_t); Iter++) {
			*P++ = *(uint32_t *)(base + SDHCI_BUFFER);
		}
	}

	/* end */
	ret = WAIT(!(reg_read(base, SDHCI_INT_STATUS) & SDHCI_INT_DATA_END));
	reg_write(base, SDHCI_INT_DATA_END, SDHCI_INT_STATUS);
	if (ret) {
		goto exit;
	}

	/* multi */
	int mode = reg_read(base, SDHCI_TRANSFER_MODE);

	if (mode & SDHCI_TRNS_MULTI) {
		if (reg_read(base, SDHCI_32BIT_BLK_CNT)) {
			ret = -1;
			goto exit;
		}
	}

	/* error */
	if (reg_read(base, SDHCI_ERR_STATUS)) {
		ret = -1;
		reg_write(base, 0xffff, SDHCI_ERR_STATUS);
		goto exit;
	}

exit:
	led_on(base, false);
	return ret;
}

static int dw_write(int lba, uintptr_t buf, size_t size)
{
	int ret;
	uintptr_t base = MMAVLSP_EMMC_BASE;
	uint32_t j;
	uint32_t Iter;
	uint32_t *P = (void *)buf;
	uint32_t Blocksize = reg_read(base, SDHCI_BLOCK_SIZE);
	uint32_t Blocks    = reg_read(base, SDHCI_32BIT_BLK_CNT);

	led_on(base, true);
	for (j = 0; j < Blocks; j++) {
		ret = WAIT(!(reg_read(base, SDHCI_INT_STATUS) & SDHCI_INT_SPACE_AVAIL));
		reg_write(base, SDHCI_INT_SPACE_AVAIL, SDHCI_INT_STATUS);
		if (ret) {
			goto exit;
		}
		for (Iter = 0; Iter < Blocksize / sizeof(uint32_t); Iter++) {
			*(uint32_t *)(base + SDHCI_BUFFER) = *P++;
		}
	}

	/* end */
	ret = WAIT(!(reg_read(base, SDHCI_INT_STATUS) & SDHCI_INT_DATA_END));
	reg_write(base, SDHCI_INT_DATA_END, SDHCI_INT_STATUS);
	if (ret) {
		goto exit;
	}

	/* multi */
	if (reg_read(base, SDHCI_TRANSFER_MODE) & SDHCI_TRNS_MULTI) {
		ret = WAIT(reg_read(base, SDHCI_32BIT_BLK_CNT));
		if (ret) {
			goto exit;
		}
	}

	/* error */
	if (reg_read(base, SDHCI_ERR_STATUS)) {
		ret = -1;
		reg_write(base, 0xffff, SDHCI_ERR_STATUS);
		goto exit;
	}

exit:
	led_on(base, false);
	return ret;
}

static int dw_set_ios(unsigned int clk, unsigned int width)
{
	uintptr_t base = MMAVLSP_EMMC_BASE;

	clock_supply(base, clk);
	config_width(base, width);
	return 0;
}

static void dw_init(void)
{
	uintptr_t base = MMAVLSP_EMMC_BASE;

	init_host(base);
	clock_supply(base, SDHCI_INIT_CLOCK);
	speed_mode(base, SD_DEFAULT);
}

void dw_mshc_off(void)
{
	uintptr_t base = MMAVLSP_EMMC_BASE;

	reset(base);
}

static const struct mmc_ops dw_mmc_ops = {
	.init		= dw_init,
	.send_cmd	= dw_send_cmd,
	.set_ios	= dw_set_ios,
	.prepare	= dw_prepare,
	.read		= dw_read,
	.write		= dw_write,
};

int dw_mshc_init(void)
{
	int err;
	static struct mmc_device_info info;

	memset(&info, 0, sizeof(info));
#if 0
	/* eMMC */
	info.mmc_dev_type = MMC_IS_EMMC;
	info.ocr_voltage = OCR_3_2_3_3;
	err = mmc_init(&dw_mmc_ops, SDHCI_DEFAULT_CLOCK, MMC_BUS_WIDTH_8, MMC_FLAG_CMD23, &info);
#else
	/* SD */
	info.mmc_dev_type = MMC_IS_SD;
	info.ocr_voltage = OCR_3_2_3_3;
	err = mmc_init(&dw_mmc_ops, SDHCI_DEFAULT_CLOCK, MMC_BUS_WIDTH_4, 0, &info);
#endif
	return err;
}
