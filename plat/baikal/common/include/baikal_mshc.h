/*
 * Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BAIKAL_MSHC_H
#define BAIKAL_MSHC_H

#include <drivers/mmc.h>

#define SDHCI_INIT_CLOCK			(300 * 1000)
#define SDHCI_DEFAULT_CLOCK			(50 * 1000 * 1000)
#define SDHCI_DEFAULT_BLOCK_SIZE		512

/* Registers */
#define SDHCI_DMA_ADDRESS			0x00
#define SDHCI_ARGUMENT2				0x00
#define SDHCI_32BIT_BLK_CNT			0x00

#define SDHCI_BLOCK_SIZE			0x04
#define  SDHCI_MAKE_BLKSZ(dma, blksz)		((((dma) & 0x7) << 12) | ((blksz) & 0xfff))

#define SDHCI_16BIT_BLK_CNT			0x06

#define SDHCI_ARGUMENT				0x08

#define SDHCI_TRANSFER_MODE			0x0c
#define  SDHCI_TRNS_DMA				(1 << 0)
#define  SDHCI_TRNS_BLK_CNT_EN			(1 << 1)
#define  SDHCI_TRNS_AUTO_CMD12			(1 << 2)
#define  SDHCI_TRNS_AUTO_CMD23			(1 << 3)
#define  SDHCI_TRNS_AUTO_SEL			0x0c
#define  SDHCI_TRNS_READ			(1 << 4)
#define  SDHCI_TRNS_MULTI			(1 << 5)

#define SDHCI_COMMAND				0x0e
#define  SDHCI_CMD_RESP_NONE			0x00
#define  SDHCI_CMD_RESP_LONG			0x01
#define  SDHCI_CMD_RESP_SHORT			0x02
#define  SDHCI_CMD_RESP_SHORT_BUSY		0x03

#define SDHCI_RESPONSE				0x10
#define SDHCI_RESPONSE_0			(SDHCI_RESPONSE + 0x4 * 0)
#define SDHCI_RESPONSE_1			(SDHCI_RESPONSE + 0x4 * 1)
#define SDHCI_RESPONSE_2			(SDHCI_RESPONSE + 0x4 * 2)
#define SDHCI_RESPONSE_3			(SDHCI_RESPONSE + 0x4 * 3)

#define SDHCI_BUFFER				0x20

#define SDHCI_PRESENT_STATE			0x24
#define  SDHCI_CMD_INHIBIT			(1 << 0)
#define  SDHCI_DATA_INHIBIT			(1 << 1)
#define  SDHCI_DATA_74				0x000000f0
#define  SDHCI_DOING_WRITE			(1 << 8)
#define  SDHCI_DOING_READ			(1 << 9)
#define  SDHCI_SPACE_AVAILABLE			(1 << 10)
#define  SDHCI_DATA_AVAILABLE			(1 << 11)
#define  SDHCI_CARD_PRESENT			(1 << 16)
#define  SDHCI_CARD_PRES_SHIFT			16
#define  SDHCI_CD_STABLE			(1 << 17)
#define  SDHCI_CD_LVL				(1 << 18)
#define  SDHCI_CD_LVL_SHIFT			18
#define  SDHCI_WRITE_PROTECT			(1 << 19)
#define  SDHCI_DATA_30				0x00f00000
#define  SDHCI_DATA_LVL_SHIFT			20
#define  SDHCI_DATA_0_LVL_MASK			0x00100000
#define  SDHCI_CMD_LVL				(1 << 24)
#define  SDHCI_VOLTAGE_STABLE			(1 << 25)

#define SDHCI_HOST_CONTROL			0x28
#define  SDHCI_CTRL_LED				0x01
#define  SDHCI_CTRL_4BITBUS			0x02
#define  SDHCI_CTRL_HISPD			0x04
#define  SDHCI_CTRL_DMA_MASK			0x18
#define  SDHCI_CTRL_SDMA			0x00
#define  SDHCI_CTRL_ADMA1			0x08
#define  SDHCI_CTRL_ADMA32			0x10
#define  SDHCI_CTRL_ADMA64			0x18
#define  SDHCI_CTRL_ADMA3			0x18
#define  SDHCI_CTRL_8BITBUS			0x20
#define  SDHCI_CTRL_CDTEST_INS			0x40
#define  SDHCI_CTRL_CDTEST_EN			0x80

#define SDHCI_POWER_CONTROL			0x29
#define  SDHCI_POWER_OFF			(0x0 << 0)
#define  SDHCI_POWER_ON				(0x1 << 0)
#define  SDHCI_POWER_180			(0x5 << 1)
#define  SDHCI_POWER_300			(0x6 << 1)
#define  SDHCI_POWER_330			(0x7 << 1)
#define  SDHCI_EMMC_POWER_120			(0x5 << 1)
#define  SDHCI_EMMC_POWER_180			(0x6 << 1)
#define  SDHCI_EMMC_POWER_300			(0x7 << 1)
#define  SDHCI_POWER2_ON			(0x1 << 4)
#define  SDHCI_POWER2_120			(0x4 << 5)
#define  SDHCI_POWER2_180			(0x5 << 5)

#define SDHCI_BLOCK_GAP_CONTROL			0x2a

#define SDHCI_WAKE_UP_CONTROL			0x2b
#define  SDHCI_WAKE_ON_INT			0x01
#define  SDHCI_WAKE_ON_INSERT			0x02
#define  SDHCI_WAKE_ON_REMOVE			0x04

#define SDHCI_CLOCK_CONTROL			0x2c
#define  SDHCI_CLOCK_FREG_SELECT_LOW		(0xff << 8)
#define  SDHCI_CLOCK_FREG_SELECT_HI		(3 << 6)
#define  SDHCI_CLOCK_GEN_SELECT			(1 << 5)
#define  SDHCI_CLOCK_RESERVED			(1 << 4)
#define  SDHCI_CLOCK_PLL_EN			(1 << 3)
#define  SDHCI_CLOCK_CARD_EN			(1 << 2)
#define  SDHCI_CLOCK_STABLE			(1 << 1)
#define  SDHCI_CLOCK_EN				(1 << 0)

#define SDHCI_TIMEOUT_CONTROL			0x2e
#define  SDHCI_TIMEOUT_DEFAULT			0xe

#define SDHCI_SOFTWARE_RESET			0x2f
#define  SDHCI_RESET_ALL			0x01
#define  SDHCI_RESET_CMD			0x02
#define  SDHCI_RESET_DATA			0x04

#define SDHCI_INT_STATUS			0x30
#define SDHCI_INT_ENABLE			0x34
#define SDHCI_SIGNAL_ENABLE			0x38
#define  SDHCI_INT_RESPONSE			(1 << 0)
#define  SDHCI_INT_DATA_END			(1 << 1)
#define  SDHCI_INT_BLK_GAP			(1 << 2)
#define  SDHCI_INT_DMA_END			(1 << 3)
#define  SDHCI_INT_SPACE_AVAIL			(1 << 4)
#define  SDHCI_INT_DATA_AVAIL			(1 << 5)
#define  SDHCI_INT_CARD_INSERT			(1 << 6)
#define  SDHCI_INT_CARD_REMOVE			(1 << 7)
#define  SDHCI_INT_CARD_INT			(1 << 8)
#define  SDHCI_INT_RETUNE			(1 << 12)
#define  SDHCI_INT_FX				(1 << 13)
#define  SDHCI_INT_CQE				(1 << 14)
#define  SDHCI_INT_ERROR			(1 << 15)

#define SDHCI_ERR_STATUS			0x32
#define SDHCI_ERR_ENABLE			0x36
#define SDHCI_ERR_SIGNAL_ENABLE			0x3a
#define  SDHCI_ERR_TIMEOUT			(1 << 0)
#define  SDHCI_ERR_CRC				(1 << 1)
#define  SDHCI_ERR_END_BIT			(1 << 2)
#define  SDHCI_ERR_INDEX			(1 << 3)
#define  SDHCI_ERR_DATA_TIMEOUT			(1 << 4)
#define  SDHCI_ERR_DATA_CRC			(1 << 5)
#define  SDHCI_ERR_DATA_END_BIT			(1 << 6)
#define  SDHCI_ERR_BUS_POWER			(1 << 7)
#define  SDHCI_ERR_AUTO_CMD_ERR			(1 << 8)
#define  SDHCI_ERR_ADMA				(1 << 9)
#define  SDHCI_ERR_TUNING			(1 << 10)
#define  SDHCI_ERR_RESP				(1 << 11)
#define  SDHCI_ERR_BOOT				(1 << 12)

#define SDHCI_AUTO_CMD_STATUS			0x3c
#define  SDHCI_AUTO_CMD_TIMEOUT			0x00000002
#define  SDHCI_AUTO_CMD_CRC			0x00000004
#define  SDHCI_AUTO_CMD_END_BIT			0x00000008
#define  SDHCI_AUTO_CMD_INDEX			0x00000010

#define SDHCI_HOST_CONTROL2			0x3e
#define  SDHCI_CTRL_UHS_MASK			0x0007
#define  SDHCI_CTRL_UHS_SDR12			0x0000
#define  SDHCI_CTRL_UHS_SDR25			0x0001
#define  SDHCI_CTRL_UHS_SDR50			0x0002
#define  SDHCI_CTRL_UHS_SDR104			0x0003
#define  SDHCI_CTRL_UHS_DDR50			0x0004
#define  SDHCI_CTRL_HS400			0x0007
#define  SDHCI_CTRL_VDD_180			0x0008
#define  SDHCI_CTRL_DRV_TYPE_MASK		0x0030
#define  SDHCI_CTRL_DRV_TYPE_B			0x0000
#define  SDHCI_CTRL_DRV_TYPE_A			0x0010
#define  SDHCI_CTRL_DRV_TYPE_C			0x0020
#define  SDHCI_CTRL_DRV_TYPE_D			0x0030
#define  SDHCI_CTRL_EXEC_TUNING			0x0040
#define  SDHCI_CTRL_TUNED_CLK			0x0080
#define  SDHCI_CMD23_ENABLE			0x0800
#define  SDHCI_CTRL_V4_MODE			0x1000
#define  SDHCI_CTRL_64BIT_ADDR			0x2000
#define  SDHCI_CTRL_ASYNC			0x4000
#define  SDHCI_CTRL_PRESET_VAL_ENABLE		0x8000

#define SDHCI_CAPABILITIES			0x40
#define  SDHCI_TIMEOUT_CLK_MASK			0x0000003f
#define  SDHCI_TIMEOUT_CLK_SHIFT		0
#define  SDHCI_TIMEOUT_CLK_UNIT			0x00000080
#define  SDHCI_CLOCK_BASE_MASK			0x00003f00
#define  SDHCI_CLOCK_V3_BASE_MASK		0x0000ff00
#define  SDHCI_CLOCK_BASE_SHIFT			8
#define  SDHCI_MAX_BLOCK_MASK			0x00030000
#define  SDHCI_MAX_BLOCK_SHIFT			16
#define  SDHCI_CAN_DO_8BIT			0x00040000
#define  SDHCI_CAN_DO_ADMA2			0x00080000
#define  SDHCI_CAN_DO_ADMA1			0x00100000
#define  SDHCI_CAN_DO_HISPD			0x00200000
#define  SDHCI_CAN_DO_SDMA			0x00400000
#define  SDHCI_CAN_DO_SUSPEND			0x00800000
#define  SDHCI_CAN_VDD_330			0x01000000
#define  SDHCI_CAN_VDD_300			0x02000000
#define  SDHCI_CAN_VDD_180			0x04000000
#define  SDHCI_CAN_64BIT_V4			0x08000000
#define  SDHCI_CAN_64BIT			0x10000000
#define  SDHCI_SUPPORT_SDR50			0x00000001
#define  SDHCI_SUPPORT_SDR104			0x00000002
#define  SDHCI_SUPPORT_DDR50			0x00000004
#define  SDHCI_SUPPORT_UHS2			0x00000008
#define  SDHCI_DRIVER_TYPE_A			0x00000010
#define  SDHCI_DRIVER_TYPE_C			0x00000020
#define  SDHCI_DRIVER_TYPE_D			0x00000040
#define  SDHCI_RETUNING_TIMER_COUNT_MASK	0x00000f00
#define  SDHCI_RETUNING_TIMER_COUNT_SHIFT	8
#define  SDHCI_USE_SDR50_TUNING			0x00002000
#define  SDHCI_RETUNING_MODE_MASK		0x0000c000
#define  SDHCI_RETUNING_MODE_SHIFT		14
#define  SDHCI_CLOCK_MUL_MASK			0x00ff0000
#define  SDHCI_CLOCK_MUL_SHIFT			16
#define  SDHCI_CAN_DO_ADMA3			0x08000000
#define  SDHCI_SUPPORT_VDD2_18			0x10000000
#define  SDHCI_SUPPORT_HS400			0x80000000

#define SDHCI_CAPABILITIES_1			0x44

#define SDHCI_MAX_CURRENT			0x48
#define  SDHCI_MAX_CURRENT_LIMIT		0xff
#define  SDHCI_MAX_CURRENT_330_MASK		0x0000ff
#define  SDHCI_MAX_CURRENT_330_SHIFT		0
#define  SDHCI_MAX_CURRENT_300_MASK		0x00ff00
#define  SDHCI_MAX_CURRENT_300_SHIFT		8
#define  SDHCI_MAX_CURRENT_180_MASK		0xff0000
#define  SDHCI_MAX_CURRENT_180_SHIFT		16
#define  SDHCI_MAX_CURRENT_MULTIPLIER		4

/* 4c-4f reserved for more max current */

#define SDHCI_SET_ACMD12_ERROR			0x50
#define SDHCI_SET_INT_ERROR			0x52

#define SDHCI_ADMA_ERROR			0x54

/* 55-57 reserved */

#define SDHCI_ADMA_ADDRESS			0x58
#define SDHCI_ADMA_ADDRESS_HI			0x5c

/* 60-fb reserved */

#define SDHCI_PRESET_INIT			0x60
#define SDHCI_PRESET_DS				0x62
#define SDHCI_PRESET_HS				0x64
#define SDHCI_PRESET_FOR_SDR12			0x66
#define SDHCI_PRESET_FOR_SDR25			0x68
#define SDHCI_PRESET_FOR_SDR50			0x6a
#define SDHCI_PRESET_FOR_SDR104			0x6c
#define SDHCI_PRESET_FOR_DDR50			0x6e
#define SDHCI_PRESET_FOR_HS400			0x74
#define  SDHCI_PRESET_DRV_MASK			0xc000
#define  SDHCI_PRESET_DRV_SHIFT			14
#define  SDHCI_PRESET_CLKGEN_SEL_MASK		0x400
#define  SDHCI_PRESET_CLKGEN_SEL_SHIFT		10
#define  SDHCI_PRESET_SDCLK_FREQ_MASK		0x3ff
#define  SDHCI_PRESET_SDCLK_FREQ_SHIFT		0

#define SDHCI_SLOT_INT_STATUS			0xfc

#define SDHCI_HOST_VERSION			0xfe
#define  SDHCI_VENDOR_VER_MASK			0xff00
#define  SDHCI_VENDOR_VER_SHIFT			8
#define  SDHCI_SPEC_VER_MASK			0x00ff
#define  SDHCI_SPEC_VER_SHIFT			0
#define  SDHCI_SPEC_100				0
#define  SDHCI_SPEC_200				1
#define  SDHCI_SPEC_300				2
#define  SDHCI_SPEC_400				3
#define  SDHCI_SPEC_410				4
#define  SDHCI_SPEC_420				5

#define SDHCI_MSHC_VER				0x504
#define SDHCI_MSHC_CTRL				0x508
#define SDHCI_EMMC_CONTROL			0x52c
#define  SDHCI_EMMC_TYPE_MMC			(1 << 0)
#define  SDHCI_EMMC_CRC_DISABLE			(1 << 1)
#define  SDHCI_EMMC_DONT_RESET			(1 << 2)

typedef struct {
	uint32_t size			:11 - 0 + 1;	/* 11 - 0 */
	uint32_t boundary		:14 - 12 + 1;	/* 14 - 12 */
	uint32_t _			:1;		/* 15 */
} reg_block_t;

typedef struct {
	uint32_t dma_en			:1;		/* 0 */
	/* 0 - disable */
	/* 1 - enable */
	uint32_t block_cnt_en		:1;		/* 1 */
	/* 0 - disable */
	/* 1 - enable */
	uint32_t auto_cmd		:3 - 2 + 1;	/* 3 - 2 */
	/* 0 - disable */
	/* 1 - cmd12_enable */
	/* 2 - cmd23_enable */
	/* 3 - auto_sel */
	uint32_t xfer_dir		:1;		/* 4 */
	/* 0 - write */
	/* 1 - read */
	uint32_t multi_block		:1;		/* 5 */
	/* 0 - single */
	/* 1 - multi */
	uint32_t resp_type		:1;		/* 6 */
	/* 0 - R1, memory */
	/* 1 - R5, sdio */
	uint32_t resp_err_chk_en	:1;		/* 7 */
	/* 0 - disable */
	/* 1 - enable */
	uint32_t resp_irq_dis		:1;		/* 8 */
	/* 0 - enable */
	/* 1 - disable */
	uint32_t _			:15 - 9 + 1;	/* 15 - 9 */
} reg_mode_t;

typedef struct {
	uint32_t resp_type		:1 - 0 + 1;	/* 1 - 0 */
	/* 0 - noresp */
	/* 1 - len136 */
	/* 2 - len48 */
	/* 3 - len48b */
	uint32_t flag			:1;		/* 2 */
	/* 0 - main */
	/* 1 - sub */
	uint32_t crc_chk		:1;		/* 3 */
	/* 0 - no check */
	/* 1 - check */
	uint32_t index_chk		:1;		/* 4 */
	/* 0 - no check */
	/* 1 - check */
	uint32_t data_present		:1;		/* 5 */
	/* 0 - no data */
	/* 1 - data */
	uint32_t type			:7 - 6 + 1;	/* 7 - 6 */
	/* 0 - normal */
	/* 1 - suspend */
	/* 2 - resume */
	/* 3 - abort */
	uint32_t index			:13 - 8 + 1;	/* 13 - 8 */
	uint32_t _			:15 - 14 + 1;	/* 15 - 14 */
} reg_cmd_t;

typedef struct {
	uint32_t cmd_busy		:1;		/* 0 */
	uint32_t data_busy		:1;		/* 1 */
	uint32_t data_active		:1;		/* 2 */
	uint32_t re_tune		:1;		/* 3 */
	uint32_t dat_line_7_4		:7 - 4 + 1;	/* 7 - 4 */
	uint32_t wr_active		:1;		/* 8 */
	uint32_t rd_active		:1;		/* 9 */
	uint32_t buf_wr_en		:1;		/* 10 */
	uint32_t buf_rd_en		:1;		/* 11 */
	uint32_t _			:15 - 12 + 1;	/* 15 - 12 */
	uint32_t card_inserted		:1;		/* 16 */
	uint32_t card_stable		:1;		/* 17 */
	uint32_t card_detect		:1;		/* 18 */
	uint32_t wr_protect		:1;		/* 19 */
	uint32_t dat_line_3_0		:23 - 20 + 1;	/* 23 - 20 */
	uint32_t cmd_line		:1;		/* 24 */
	uint32_t voltage_stable		:1;		/* 25 */
	uint32_t __			:1;		/* 26 */
	uint32_t cmd_err		:1;		/* 27 */
	uint32_t sub_cmd		:1;		/* 28 */
	uint32_t dormant		:1;		/* 29 */
	uint32_t sync			:1;		/* 30 */
	uint32_t uhs2			:1;		/* 31 */
} reg_state_t;

typedef struct {
	uint32_t vdd1_on		:1;		/* 0 */
	uint32_t vdd1			:3 - 1 + 1;	/* 3 - 1 */
	uint32_t vdd2_on		:1;		/* 4 */
	uint32_t vdd2			:7 - 5 + 1;	/* 7 - 5 */
} reg_pwr_t;

typedef struct {
	uint32_t int_en			:1;		/* 0 */
	uint32_t int_ready		:1;		/* 1 */
	uint32_t ext_en			:1;		/* 2 */
	uint32_t pll_en			:1;		/* 3 */
	uint32_t _			:1;		/* 4 */
	uint32_t select			:1;		/* 5 */
	uint32_t freq_hi		:7 - 6 + 1;	/* 7 - 6 */
	uint32_t freq_low		:15 - 8 + 1;	/* 15 - 8 */
} reg_clk_t;

typedef struct {
	uint32_t all			:1;		/* 0 */
	uint32_t cmd			:1;		/* 1 */
	uint32_t data			:1;		/* 2 */
	uint32_t _			:7 - 3 + 1;	/* 7 - 3 */
} reg_reset_t;

typedef struct {
	uint32_t cmd			:1;		/* 0 */
	uint32_t xfer			:1;		/* 1 */
	uint32_t bgap			:1;		/* 2 */
	uint32_t dma			:1;		/* 3 */
	uint32_t buf_wr			:1;		/* 4 */
	uint32_t buf_rd			:1;		/* 5 */
	uint32_t card_insert		:1;		/* 6 */
	uint32_t card_remove		:1;		/* 7 */
	uint32_t card_irq		:1;		/* 8 */
	uint32_t _			:11 - 9 + 1;	/* 11 - 9 */
	uint32_t tune			:1;		/* 12 */
	uint32_t fx			:1;		/* 13 */
	uint32_t cqe			:1;		/* 14 */
	uint32_t err			:1;		/* 15 */
} reg_int_t;

typedef struct {
	uint32_t led			:1;		/* 0 */
	/* 0 - off */
	/* 1 - on */
	uint32_t width			:1;		/* 1 */
	/* 0 - 1-bit */
	/* 1 - 4-bit */
	uint32_t speed			:1;		/* 2 */
	/* 0 - normal */
	/* 1 - high */
	uint32_t dma			:4 - 3 + 1;	/* 4 - 3 */
	/* 0 - sdma */
	/* 2 - adma2 */
	/* 3 - adma3 */
	uint32_t ext_width		:1;		/* 5 */
	/* 0 - default */
	/* 1 - 8-bit */
	uint32_t detect_lvl		:1;		/* 6 */
	uint32_t detect_sig		:1;		/* 7 */
} reg_ctl1_t;

typedef struct {
	uint32_t mode			:2 - 0 + 1;	/* 2 - 0 */
	/* 0 - sdr12 */
	/* 1 - sdr25 */
	/* 2 - sdr50 */
	/* 3 - sdr104 */
	/* 4 - ddr50 */
	/* 7 - uhs2 */
	uint32_t signal			:1;		/* 3 */
	/* 0 - 3.3V */
	/* 1 - 1.8V */
	uint32_t strength		:1; /* TODO: 2? 5 - 4 */
	/* 0 - TypeB */
	/* 1 - TypeA */
	/* 2 - TypeC */
	/* 3 - TypeD */
	uint32_t tune			:1;		/* 6 */
	/* 0 - not */
	/* 1 - execut */
	uint32_t sample			:1;		/* 7 */
	/* 0 - fixed clock */
	/* 1 - tuned clock */
	uint32_t _			:9 - 8 + 1;	/* 9 - 8 */
	uint32_t adma2_mode		:1;		/* 10 */
	/* 0 - 16bit data len */
	/* 1 - 26bit data len */
	uint32_t cmd32			:1;		/* 11 */
	/* 0 - off */
	/* 1 - on */
	uint32_t version		:1;		/* 12 */
	uint32_t __			:1;		/* 13 */
	uint32_t async			:1;		/* 14 */
	/* 0 - off */
	/* 1 - on */
	uint32_t preset			:1;		/* 15 */
} reg_ctl2_t;

#define WAIT(X) ({			\
	int Ret = 0;			\
	int Try = 100 * 1000;		\
	while (X) {			\
		if (!Try--) {		\
			Ret = -1;	\
			break;		\
		}			\
		udelay(1);		\
	};				\
	Ret;				\
})

enum SpeedMode {
	SD_DEFAULT,
	SD_HIGH,
	SD_12,
	SD_25,
	SD_50,
	SD_104,
	DD_50
};

enum CmdType {
	SdCommandTypeBc,	/* Broadcast commands, no response */
	SdCommandTypeBcr,	/* Broadcast commands with response */
	SdCommandTypeAc,	/* Addressed(point-to-point) commands */
	SdCommandTypeAdtc	/* Addressed(point-to-point) data transfer commands */
};

enum RespType {
	SdResponseTypeNo,
	SdResponseTypeR1,
	SdResponseTypeR1b,
	SdResponseTypeR2,
	SdResponseTypeR3,
	SdResponseTypeR4,
	SdResponseTypeR5,
	SdResponseTypeR5b,
	SdResponseTypeR6,
	SdResponseTypeR7
};

typedef struct dw_mshc_params {
	uintptr_t		base;
	int			clk;
	int			width;
	int			flags;
	enum mmc_device_type	mmc_dev_type;
} dw_mshc_params_t;

int dw_mshc_init(void);
int dw_mshc_read(uint32_t adr, void *dst, size_t size);
int dw_mshc_write(uint32_t adr, void *src, size_t size);
int dw_mshc_erase(uint32_t adr, size_t size);
void dw_mshc_off(void);

#define DIV_ROUND_UP(n, d)	((n) / (d) + !!((n) % (d)))
#define ROUND_UP2(n, d)		(((n) / (d) + !!((n) % (d))) * d)
#define ROUND_DOWN2(n, d)	(((n) / (d)) * (d))
#define ROUND_UP(n)		ROUND_UP2((n), MMC_BLOCK_SIZE)
#define ROUND_DOWN(n)		ROUND_DOWN2((n), MMC_BLOCK_SIZE)

#endif /* BAIKAL_MSHC_H */
