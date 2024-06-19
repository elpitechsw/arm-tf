/*
 * Copyright (c) 2020-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/gic600_multichip.h>
#include <drivers/generic_delay_timer.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <baikal_def.h>
#include <baikal_gicv3.h>
#include <bs1000_cmu.h>
#include <bs1000_coresight.h>
#include <bs1000_dimm_spd.h>
#include <bs1000_scp_lcru.h>

#include "bs1000_pcie.h"
#include "bs1000_smmu.h"
#include "bs1000_usb.h"

#ifdef MULTICHIP_SYSTEM_COUNTERS_RESYNC
#include <baikal_interrupt_mgmt.h>
#include <dw_apb_timer.h>
#endif

void baikal_fdt_memory_update(void);
void baikal_fdt_ddr_node_enable(void);

#if (PLATFORM_CHIP_COUNT > 1)
static struct gic600_multichip_data bs1000_multichip_data __init = {
	.rt_owner_base = BAIKAL_GICD_BASE,
	.rt_owner = 0,
	.chip_count = PLATFORM_CHIP_COUNT,
	.chip_addrs = {
		1,
		0,
#if (PLATFORM_CHIP_COUNT > 2)
		0,
#endif
#if (PLATFORM_CHIP_COUNT > 3)
		0,
#endif
	},
	.spi_ids = {
		{BAIKAL_GICD_BASE, 32, 351},
		{BAIKAL_GICD_BASE, 352, 575},
#if (PLATFORM_CHIP_COUNT > 2)
		{0, 0},
#endif
#if (PLATFORM_CHIP_COUNT > 3)
		{0, 0},
#endif
	}
};

static uintptr_t bs1000_multichip_gicr_frames[] = {
	/* Chip 0's GICR Base */
	BAIKAL_GICR_BASE,
	/* Chip 1's GICR BASE */
	PLATFORM_ADDR_OUT_CHIP(1, BAIKAL_GICR_BASE),
#if (PLATFORM_CHIP_COUNT > 2)
	/* Chip 2's GICR BASE */
	PLATFORM_ADDR_OUT_CHIP(2, BAIKAL_GICR_BASE),
#endif
#if (PLATFORM_CHIP_COUNT > 3)
	/* Chip 3's GICR BASE */
	PLATFORM_ADDR_OUT_CHIP(3, BAIKAL_GICR_BASE),
#endif
	UL(0)	/* Zero Termination */
};
#endif

void bl31_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_REGION_FLAT(BL31_START,
				BL31_END - BL31_START,
				MT_MEMORY | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(BL_CODE_BASE,
				BL_CODE_END - BL_CODE_BASE,
				MT_CODE | MT_SECURE),

		{0}
	};

	const mmap_region_t plat_bs1000_mmap[] = {
		MAP_REGION_FLAT(MAILBOX_SRAM_BASE,
				MAILBOX_SRAM_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(MAILBOX_IRB_BASE,
				MAILBOX_IRB_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(SMMU_BASE,
				SMMU_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(UART_A1_BASE,
				UART_A1_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(QSPI1_BASE,
				QSPI1_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(GPIO32_BASE,
				GPIO32_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(I2C2_BASE,
				I2C2_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(I2C3_BASE,
				I2C3_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(GIC_BASE,
				GIC_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(CA75_BASE,
				CA75_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PCIE_BASE,
				PCIE_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(DDR_BASE,
				DDR_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(BAIKAL_NS_DTB_BASE,
				BAIKAL_DTB_MAX_SIZE,
				MT_MEMORY | MT_RW | MT_NS),

		MAP_REGION_FLAT(BAIKAL_SCMM_SMMU_BASE,
				BAIKAL_SCMM_SMMU_SIZE,
				MT_NON_CACHEABLE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(BAIKAL_SEC_DTB_BASE,
				BAIKAL_DTB_MAX_SIZE,
				MT_MEMORY | MT_RW | MT_SECURE),
#ifdef MULTICHIP_SYSTEM_COUNTERS_RESYNC
		MAP_REGION_FLAT(TIMERS_BASE,
				TIMERS_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(CORESIGHT_CFG_BASE,
				CORESIGHT_CFG_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),
#endif

#if (PLATFORM_CHIP_COUNT > 1)
		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(1, MAILBOX_SRAM_BASE),
				MAILBOX_SRAM_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(1, MAILBOX_IRB_BASE),
				MAILBOX_IRB_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(1, SMMU_BASE),
				SMMU_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(1, I2C2_BASE),
				I2C2_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(1, I2C3_BASE),
				I2C3_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(1, GIC_BASE),
				GIC_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(1, CA75_BASE),
				CA75_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(1, PCIE_BASE),
				PCIE_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(1, DDR_BASE),
				DDR_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),
#ifdef MULTICHIP_SYSTEM_COUNTERS_RESYNC
		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(1, CORESIGHT_CFG_BASE),
				CORESIGHT_CFG_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),
#endif

#endif

#if (PLATFORM_CHIP_COUNT > 2)
		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(2, MAILBOX_SRAM_BASE),
				MAILBOX_SRAM_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(2, MAILBOX_IRB_BASE),
				MAILBOX_IRB_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(2, SMMU_BASE),
				SMMU_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(2, I2C2_BASE),
				I2C2_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(2, I2C3_BASE),
				I2C3_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(2, GIC_BASE),
				GIC_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(2, CA75_BASE),
				CA75_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(2, PCIE_BASE),
				PCIE_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(2, DDR_BASE),
				DDR_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),
#ifdef MULTICHIP_SYSTEM_COUNTERS_RESYNC
		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(2, CORESIGHT_CFG_BASE),
				CORESIGHT_CFG_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),
#endif

#endif

#if (PLATFORM_CHIP_COUNT > 3)
		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(3, MAILBOX_SRAM_BASE),
				MAILBOX_SRAM_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(3, MAILBOX_IRB_BASE),
				MAILBOX_IRB_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(3, SMMU_BASE),
				SMMU_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(3, I2C2_BASE),
				I2C2_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(3, I2C3_BASE),
				I2C3_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(3, GIC_BASE),
				GIC_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(3, CA75_BASE),
				CA75_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(3, PCIE_BASE),
				PCIE_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(3, DDR_BASE),
				DDR_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),
#ifdef MULTICHIP_SYSTEM_COUNTERS_RESYNC
		MAP_REGION_FLAT(PLATFORM_ADDR_OUT_CHIP(3, CORESIGHT_CFG_BASE),
				CORESIGHT_CFG_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),
#endif

#endif
		{0}
	};

	unsigned int chip_idx;

	for (chip_idx = 0; chip_idx < PLATFORM_CHIP_COUNT; ++chip_idx) {
		/* Enable non-secure access */
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_SC_CFG_EHCI),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_SC_CFG_GIC),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_SC_CFG_GMAC0),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_SC_CFG_GMAC1),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_SC_CFG_LSP),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_SC_CFG_OHCI),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_SC_CFG_SCP),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_SC_CFG_SMMU),
			      NIC_GPV_REGIONSEC_NONSECURE);

		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_LSP_CFG_ESPI),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_LSP_CFG_GPIO16),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_LSP_CFG_GPIO32),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_LSP_CFG_GPIO8_1),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_LSP_CFG_GPIO8_2),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_LSP_CFG_QSPI1),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_LSP_CFG_QSPI2),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_LSP_CFG_I2C2),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_LSP_CFG_I2C3),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_LSP_CFG_I2C4),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_LSP_CFG_I2C5),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_LSP_CFG_I2C6),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_LSP_CFG_TIMERS),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_LSP_CFG_UART_A1),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_LSP_CFG_UART_A2),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_LSP_CFG_UART_S),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, NIC_LSP_CFG_WDT),
			      NIC_GPV_REGIONSEC_NONSECURE);

		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR0_NIC_CFG_CTRL),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR1_NIC_CFG_CTRL),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR2_NIC_CFG_CTRL),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR3_NIC_CFG_CTRL),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR4_NIC_CFG_CTRL),
			      NIC_GPV_REGIONSEC_NONSECURE);
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR5_NIC_CFG_CTRL),
			      NIC_GPV_REGIONSEC_NONSECURE);
	}

	bs1000_coresight_init();
	pcie_init();

	setup_page_tables(bl_regions, plat_bs1000_mmap);
	enable_mmu_el3(0);
}

void bl31_platform_setup(void)
{
	unsigned int chip_idx;

	generic_delay_timer_init();

	for (chip_idx = 0; chip_idx < PLATFORM_CHIP_COUNT; ++chip_idx) {
		/* Deassert resets */
		scp_lcru_clrbits(PLATFORM_ADDR_OUT_CHIP(chip_idx, SCP_GPR_MM_RST_CTL2),
				 SCP_GPR_MM_RST_CTL2_LSP_SS_APB	     |
				 SCP_GPR_MM_RST_CTL2_LSP_GPIO_DIV    |
				 SCP_GPR_MM_RST_CTL2_LSP_GPIO32	     |
				 SCP_GPR_MM_RST_CTL2_LSP_GPIO32_APB  |
				 SCP_GPR_MM_RST_CTL2_LSP_GPIO16	     |
				 SCP_GPR_MM_RST_CTL2_LSP_GPIO16_APB  |
				 SCP_GPR_MM_RST_CTL2_LSP_GPIO8_1     |
				 SCP_GPR_MM_RST_CTL2_LSP_GPIO8_1_APB |
				 SCP_GPR_MM_RST_CTL2_LSP_GPIO8_2     |
				 SCP_GPR_MM_RST_CTL2_LSP_GPIO8_2_APB |
				 SCP_GPR_MM_RST_CTL2_LSP_TIMER1	     |
				 SCP_GPR_MM_RST_CTL2_LSP_TIMER2	     |
				 SCP_GPR_MM_RST_CTL2_LSP_TIMER3	     |
				 SCP_GPR_MM_RST_CTL2_LSP_TIMER4	     |
				 SCP_GPR_MM_RST_CTL2_LSP_TIMERS_APB  |
				 SCP_GPR_MM_RST_CTL2_LSP_WDT	     |
				 SCP_GPR_MM_RST_CTL2_LSP_WDT_APB     |
				 SCP_GPR_MM_RST_CTL2_LSP_UART_S	     |
				 SCP_GPR_MM_RST_CTL2_LSP_UART_S_APB  |
				 SCP_GPR_MM_RST_CTL2_LSP_UART_A1     |
				 SCP_GPR_MM_RST_CTL2_LSP_UART_A1_APB |
				 SCP_GPR_MM_RST_CTL2_LSP_UART_A2     |
				 SCP_GPR_MM_RST_CTL2_LSP_UART_A2_APB |
				 SCP_GPR_MM_RST_CTL2_LSP_WDT_DIV);

		scp_lcru_clrbits(PLATFORM_ADDR_OUT_CHIP(chip_idx, SCP_GPR_MM_RST_CTL3),
				 SCP_GPR_MM_RST_CTL3_LSP_SPI1	  |
				 SCP_GPR_MM_RST_CTL3_LSP_SPI1_APB |
				 SCP_GPR_MM_RST_CTL3_LSP_SPI2	  |
				 SCP_GPR_MM_RST_CTL3_LSP_SPI2_APB |
				 SCP_GPR_MM_RST_CTL3_LSP_I2C2	  |
				 SCP_GPR_MM_RST_CTL3_LSP_I2C2_APB |
				 SCP_GPR_MM_RST_CTL3_LSP_I2C3	  |
				 SCP_GPR_MM_RST_CTL3_LSP_I2C3_APB |
				 SCP_GPR_MM_RST_CTL3_LSP_I2C4	  |
				 SCP_GPR_MM_RST_CTL3_LSP_I2C4_APB |
				 SCP_GPR_MM_RST_CTL3_LSP_I2C5	  |
				 SCP_GPR_MM_RST_CTL3_LSP_I2C5_APB |
				 SCP_GPR_MM_RST_CTL3_LSP_I2C6	  |
				 SCP_GPR_MM_RST_CTL3_LSP_I2C6_APB |
				 SCP_GPR_MM_RST_CTL3_LSP_ESPI	  |
				 SCP_GPR_MM_RST_CTL3_LSP_ESPI_RST);
	}

#if (PLATFORM_CHIP_COUNT > 1)
	baikal_override_gicr_frames(bs1000_multichip_gicr_frames);
	gic600_multichip_init(&bs1000_multichip_data);
#endif
	baikal_gic_driver_init();
	baikal_gic_init();
	bs1000_smmu_s_init();
	bs1000_usb_init();

#ifdef BAIKAL_MUX_INIT
	/* setup mux state */
	scp_lcru_clrsetbits(SCP_GPR_LSP_CTL,
			    SCP_GPR_LSP_CTL_SEL_PERIPH_MASK,
			    BAIKAL_MUX_INIT << SCP_GPR_LSP_CTL_SEL_PERIPH_SHIFT);
#endif

	baikal_fdt_memory_update();
#if (PLATFORM_CHIP_COUNT == 1)
	/* Skipped in multichip config due to SPI interrupts limit */
	baikal_fdt_ddr_node_enable();
#endif
	memcpy((void *)BAIKAL_NS_DTB_BASE,
	       (void *)BAIKAL_SEC_DTB_BASE,
	       BAIKAL_DTB_MAX_SIZE);
}

#ifdef MULTICHIP_SYSTEM_COUNTERS_RESYNC

#define BS1000_SYNC_TIMER_RATE	100 /* times per second */

static void bs1000_sync_timer_handler(void *data)
{
	bs1000_resync_system_counters();
}

void bl31_plat_runtime_setup(void)
{
	int rc;

	/* Enable periodic system counters resynchronization */

	baikal_el3_interrupt_config();

	rc = dw_apb_timer_register(BAIKAL_SYNC_TIMER_BASE,
				   BAIKAL_SYNC_TIMER_FREQ,
				   BAIKAL_SYNC_TIMER_IRQ,
				   bs1000_sync_timer_handler,
				   NULL);
	if (rc < 0) {
		ERROR("BL31: system counters synchronization timer registration error (%d)\n",
		      rc);
		return;
	}
	rc = dw_apb_timer_start_periodic(rc, BS1000_SYNC_TIMER_RATE);
	if (rc < 0) {
		ERROR("BL31: system counters synchronization timer start error (%d)\n",
		      rc);
		return;
	}
}
#endif