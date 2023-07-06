/*
 * Copyright (c) 2021-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BAIKAL_DEF_H
#define BAIKAL_DEF_H

#include <bm1000_def.h>

#define BAIKAL_BL31_PLAT_PARAM_VAL	ULL(0x0f1e2d3c4b5a6978)
#define BAIKAL_PRIMARY_CPU		U(0)

#define BAIKAL_GICD_BASE		MMUSB_GICD_BASE
#define BAIKAL_GICR_BASE		MMUSB_GICR_BASE

#define BAIKAL_IRQ_SEC_SGI_0		8
#define BAIKAL_IRQ_SEC_SGI_1		9
#define BAIKAL_IRQ_SEC_SGI_2		10
#define BAIKAL_IRQ_SEC_SGI_3		11
#define BAIKAL_IRQ_SEC_SGI_4		12
#define BAIKAL_IRQ_SEC_SGI_5		13
#define BAIKAL_IRQ_SEC_SGI_6		14
#define BAIKAL_IRQ_SEC_SGI_7		15

#define BAIKAL_CONSOLE_16550
#define BAIKAL_UART_BASE		MMAVLSP_UART1_BASE
#define BAIKAL_UART_CLK_IN_HZ		7372800
#define BAIKAL_UART_BAUDRATE		115200

#define BAIKAL_BOOT_SPI_BASE		MMAVLSP_SPI_BASE
#define BAIKAL_BOOT_SPI_BAUDR		16
#define BAIKAL_BOOT_SPI_SS_LINE		0
#define BAIKAL_BOOT_SPI_GPIO_BASE	MMAVLSP_GPIO32_BASE
#define BAIKAL_BOOT_SPI_CS_GPIO_PIN	24
#define BAIKAL_BOOT_SPI_SUBSECTOR	(4 * 1024)
#define BAIKAL_BOOT_SPI_SIZE		(32 * 1024 * 1024)

#define BAIKAL_I2C_ICLK_FREQ		100000000
#define BAIKAL_SMBUS_ICLK_FREQ		50000000
#define SYS_COUNTER_FREQ_IN_TICKS	ULL(50000000)

#if defined(BAIKAL_DBM10)
# define BAIKAL_DDR_CUSTOM_CLOCK_FREQ	800
# define BAIKAL_SPD_TXRX(targetaddr, txbuf, txbufsize, rxbuf, rxbufsize) \
	i2c_txrx(MMAVLSP_I2C2_BASE, BAIKAL_I2C_ICLK_FREQ,		 \
		 targetaddr, txbuf, txbufsize, rxbuf, rxbufsize)
#elif defined(BAIKAL_DBM20)
# define BAIKAL_DUAL_CHANNEL_MODE
# define BAIKAL_DDR_CUSTOM_CLOCK_FREQ	1200
# define BAIKAL_SPD_TXRX(targetaddr, txbuf, txbufsize, rxbuf, rxbufsize) \
	smbus_txrx(MMAVLSP_SMBUS1_BASE,					 \
		   BAIKAL_SMBUS_ICLK_FREQ, SMBUS_SHT_100KHZ, 100000,	 \
		   targetaddr, txbuf, txbufsize, rxbuf, rxbufsize)
# if !DEBUG /* This code doesn't fit into debug tf-arm bl1 */
# define BAIKAL_DDR_SETTINGS_IN_FLASH
# endif
#elif defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
# define BAIKAL_DDR_SETTINGS_IN_FLASH
# define BAIKAL_DDR_CUSTOM_CLOCK_FREQ	1200
# define BAIKAL_SPD_TXRX(targetaddr, txbuf, txbufsize, rxbuf, rxbufsize) \
	smbus_txrx(MMAVLSP_SMBUS1_BASE,					 \
		   BAIKAL_SMBUS_ICLK_FREQ, SMBUS_SHT_100KHZ, 100000,	 \
		   targetaddr, txbuf, txbufsize, rxbuf, rxbufsize)
# define BAIKAL_LVDS_CLKEN_GPIO_PIN	17
# define BAIKAL_HDMI_CLKEN_GPIO_PIN	18
#elif defined(ELPITECH)
# ifndef BAIKAL_DDR_CUSTOM_CLOCK_FREQ
#  define BAIKAL_DDR_CUSTOM_CLOCK_FREQ	1200
# endif
# define BAIKAL_SPD_TXRX(targetaddr, txbuf, txbufsize, rxbuf, rxbufsize) \
	smbus_txrx(MMAVLSP_SMBUS1_BASE,					 \
		   BAIKAL_SMBUS_ICLK_FREQ, SMBUS_SHT_100KHZ, 100000,	 \
		   targetaddr, txbuf, txbufsize, rxbuf, rxbufsize)
#elif defined(BAIKAL_QEMU)
# define BAIKAL_DIMM_SPD_STATIC
# undef  SYS_COUNTER_FREQ_IN_TICKS
# define SYS_COUNTER_FREQ_IN_TICKS	ULL((1000 * 1000 * 1000) / 16)
#endif

#endif /* BAIKAL_DEF_H */
