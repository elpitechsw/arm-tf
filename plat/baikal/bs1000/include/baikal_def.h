/*
 * Copyright (c) 2021-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BAIKAL_DEF_H
#define BAIKAL_DEF_H

#include <bs1000_def.h>

#define BAIKAL_BL31_PLAT_PARAM_VAL	ULL(0x0f1e2d3c4b5a6978)
#define BAIKAL_PRIMARY_CPU		U(0)

#define BAIKAL_GICD_BASE		GICD_BASE
#define BAIKAL_GICR_BASE		GICR_BASE

#define BAIKAL_IRQ_SEC_SGI_0		8
#define BAIKAL_IRQ_SEC_SGI_1		9
#define BAIKAL_IRQ_SEC_SGI_2		10
#define BAIKAL_IRQ_SEC_SGI_3		11
#define BAIKAL_IRQ_SEC_SGI_4		12
#define BAIKAL_IRQ_SEC_SGI_5		13
#define BAIKAL_IRQ_SEC_SGI_6		14
#define BAIKAL_IRQ_SEC_SGI_7		15

#define BAIKAL_CONSOLE_PL011
#define BAIKAL_UART_BASE		UART_A1_BASE
#define BAIKAL_UART_CLK_IN_HZ		100000000
#define BAIKAL_UART_BAUDRATE		115200

#define BAIKAL_BOOT_SPI_BASE		QSPI1_BASE
#define BAIKAL_BOOT_SPI_BAUDR		16
#define BAIKAL_BOOT_SPI_SS_LINE		0
#define BAIKAL_BOOT_SPI_GPIO_BASE	GPIO32_BASE
#define BAIKAL_BOOT_SPI_SUBSECTOR	(4 * 1024)
#if defined(BAIKAL_MBS_1S) || defined(BAIKAL_MBS_2S)
#define BAIKAL_BOOT_SPI_SIZE		(64 * 1024 * 1024)
#else
#define BAIKAL_BOOT_SPI_SIZE		(32 * 1024 * 1024)
#endif

#define BAIKAL_I2C_ICLK_FREQ		200000000
#define SYS_COUNTER_FREQ_IN_TICKS	ULL(25000000)

#if defined(BAIKAL_QEMU)
# undef  SYS_COUNTER_FREQ_IN_TICKS
# define SYS_COUNTER_FREQ_IN_TICKS	ULL((1000 * 1000 * 1000) / 16)
#endif

#ifdef MULTICHIP_SYSTEM_COUNTERS_RESYNC
#define BAIKAL_SYNC_TIMER_BASE		(TIMERS_BASE + 0x3c)
#define BAIKAL_SYNC_TIMER_FREQ		50000000
#define BAIKAL_SYNC_TIMER_IRQ		90
#endif

#if defined(ELPITECH)
# define BAIKAL_MUX_INIT	0x1
#endif

#endif /* BAIKAL_DEF_H */
