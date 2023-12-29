/*
 * Copyright (c) 2020-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <drivers/generic_delay_timer.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include <baikal_bl1_stack.h>
#include <baikal_bootflash.h>
#include <baikal_console.h>
#include <baikal_io_storage.h>
#include <bs1000_def.h>
#include <crc.h>
#include <memtest.h>
#include <platform_def.h>

int dram_init(void);

static uint64_t trusted_mailbox[1 + PLATFORM_CORE_COUNT]
	__section(".trusted_mailbox") __used;
CASSERT(sizeof(trusted_mailbox) == BAIKAL_TRUSTED_MAILBOX_SIZE,
	assert_trusted_mailbox_size);

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

	err  = memtest_rand64(BL1_XLAT_BASE, BL1_XLAT_SIZE, sizeof(uint64_t), read_cntpct_el0());
#if DEBUG
	err |= memtest_rand8(BL1_XLAT_BASE, BL1_XLAT_SIZE, sizeof(uint8_t), read_cntpct_el0());
#endif
	if (err) {
		ERROR("%s: DRAM error\n", __func__);
		plat_panic_handler();
	}
}

void bl1_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_REGION_FLAT(BL1_RW_BASE,
				BL1_RW_LIMIT - BL1_RW_BASE,
				MT_NON_CACHEABLE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(BL_CODE_BASE,
				BL1_CODE_END - BL_CODE_BASE,
				MT_NON_CACHEABLE | MT_RO | MT_SECURE | MT_EXECUTE),

		{0}
	};

	const mmap_region_t plat_bs1000_mmap[] = {
		MAP_REGION_FLAT(NS_DRAM0_BASE,
				NS_DRAM0_SIZE,
				MT_MEMORY | MT_RW | MT_NS),

		MAP_REGION_FLAT(SEC_DRAM_BASE,
				SEC_DRAM_SIZE,
				MT_MEMORY | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(QSPI1_BASE,
				QSPI1_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(UART_A1_BASE,
				UART_A1_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		{0}
	};

	setup_page_tables(bl_regions, plat_bs1000_mmap);
	enable_mmu_el3(0);
}

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

void bl1_platform_setup(void)
{
	plat_baikal_io_setup();
}
