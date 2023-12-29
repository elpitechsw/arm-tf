/*
 * Copyright (c) 2018-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <drivers/generic_delay_timer.h>
#include <lib/mmio.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#include <baikal_bl1_stack.h>
#include <baikal_bootflash.h>
#include <baikal_console.h>
#include <baikal_io_storage.h>
#include <bm1000_cmu.h>
#include <bm1000_def.h>
#include <bm1000_private.h>
#include <crc.h>
#include <ddr_main.h>
#include <memtest.h>
#include <platform_def.h>
#include <ddr_spd.h>

#include "bm1000_splash.h"

#include <baikal_bootflash.h>
#include <baikal_mshc.h>
#include <baikal_def.h>

static uint64_t trusted_mailbox[1 + PLATFORM_CORE_COUNT]
	__section(".trusted_mailbox") __used;
CASSERT(sizeof(trusted_mailbox) == BAIKAL_TRUSTED_MAILBOX_SIZE,
	assert_trusted_mailbox_size);

/* Allocate space in static memory for DDR SPD content */
const struct spd_container spd_content = {0};

#if !DEBUG
static char msg_buf[300];
#endif

/*
 * Cannot use default weak implementation in bl1_main.c because BL1 RW and BL2
 * data size exceeds Mailbox SRAM.
 */
int bl1_plat_handle_post_image_load(unsigned int image_id)
{
	static meminfo_t bl2_tzram_layout;
	image_desc_t *image_desc;
	entry_point_info_t *ep_info;

	if (image_id != BL2_IMAGE_ID) {
		return 0;
	}

	/* Get the image descriptor */
	image_desc = bl1_plat_get_image_desc(BL2_IMAGE_ID);
	assert(image_desc != NULL);

	/* Get the entry point info */
	ep_info = &image_desc->ep_info;

	bl2_tzram_layout.total_base = BL2_BASE;
	bl2_tzram_layout.total_size = BL2_SIZE;

	flush_dcache_range((uintptr_t)&bl2_tzram_layout, sizeof(meminfo_t));
	ep_info->args.arg1 = (uintptr_t)&bl2_tzram_layout;
#if DEBUG
	bl1_stack_area_check();
#endif
	VERBOSE("BL1: BL2 memory layout address = %p\n",
		(void *)&bl2_tzram_layout);

	return 0;
}

static void ccn_hnf_sam_setup(void)
{
	unsigned int i;
	uintptr_t pccn = PLAT_ARM_CCN_BASE + CCN_HNF_OFFSET + 8;
	uint64_t snf0 = 0, snf1 = 0;

	if (cmu_pll_is_enabled(MMDDR0_CMU0_BASE)) {
		snf0 = 4;
	}

	if (cmu_pll_is_enabled(MMDDR1_CMU0_BASE)) {
		snf1 = 14;
	}

	if (!snf0 && !snf1) {
		ERROR("%s: no DDRs\n", __func__);
		return;
	} else if (!snf0) {
		snf0 = snf1;
	} else if (!snf1) {
		snf1 = snf0;
	}

	INFO("%s(%lu, %lu)\n", __func__, snf0, snf1);
	for (i = 0; i < 4; ++i) {
		mmio_write_64(pccn, snf0);
		pccn += 0x10000;
	}

	for (i = 0; i < 4; ++i) {
		mmio_write_64(pccn, snf1);
		pccn += 0x10000;
	}

	dsbish();
	isb();
}

static void ccn_xp_set_qos(int xp, int dev, int qos)
{
	uint64_t val;

	val = mmio_read_64(XP_DEV_QOS_CONTROL(xp, dev));
	val &= ~DEV_QOS_OVERRIDE_MASK;
	val |= (DEV_QOS_OVERRIDE_EN | DEV_QOS_OVERRIDE(qos));
	mmio_write_64(XP_DEV_QOS_CONTROL(xp, dev), val);
}

void bl1_early_platform_setup(void)
{
	int err;

	baikal_console_boot_init();

	assert(trusted_mailbox == (void *)BAIKAL_TRUSTED_MAILBOX_BASE);
#if DEBUG
	INFO("BL1: crc32:0x%08x size:%lu\n",
		crc32(BL1_RO_BASE, (uintptr_t)__BL1_ROM_END__ - (uintptr_t)BL1_RO_BASE, 0),
		(uintptr_t)__BL1_ROM_END__ - (uintptr_t)BL1_RO_BASE);

	bl1_stack_area_fill();
#endif
	write_cntfrq_el0(plat_get_syscnt_freq2());
	generic_delay_timer_init();

	dram_init();
	ccn_hnf_sam_setup();
	err  = memtest_rand64(BL1_XLAT_BASE, BL1_XLAT_SIZE, sizeof(uint64_t), read_cntpct_el0());
#if DEBUG
	err |= memtest_rand8(BL1_XLAT_BASE, BL1_XLAT_SIZE, sizeof(uint8_t), read_cntpct_el0());
#endif
	if (err) {
		ERROR("%s: DRAM error\n", __func__);
		plat_panic_handler();
	}

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	plat_arm_interconnect_init();

	/* Enable Interconnect coherency for the primary CPU's cluster */
	plat_arm_interconnect_enter_coherency();

	/* Override A57 XP QoS values */
	ccn_xp_set_qos(0, 1, 0x8);
	ccn_xp_set_qos(4, 1, 0x8);
	ccn_xp_set_qos(5, 1, 0x8);
	ccn_xp_set_qos(9, 1, 0x8);
}

void baikal_configure_mmu_bl1(unsigned long total_base,
			      unsigned long total_size,
			      unsigned long ro_start,
			      unsigned long ro_limit,
			      unsigned long coh_start,
			      unsigned long coh_limit);

void bl1_plat_arch_setup(void)
{
	baikal_configure_mmu_bl1(BL1_RW_BASE, BL1_RW_LIMIT - BL1_RW_BASE,
				 BL1_RO_BASE, BL1_RO_LIMIT,
				 BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_END);
}

void bl1_platform_setup(void)
{
	uint8_t *spd_ptr = (uint8_t *)PLAT_DDR_SPD_BASE;

	memcpy(spd_ptr, &spd_content, sizeof(struct spd_container));
	plat_baikal_io_setup();
	generic_delay_timer_init();
	mmavlsp_init();
#if !DEBUG
	snprintf(msg_buf, sizeof(msg_buf), "BE-M1000\nBL1: %s\nBL1: %s\n", version_string, build_message);
	lvds_early_splash(msg_buf);
	mmxgbe_init();
	hdmi_early_splash(msg_buf);
#endif
}
