/*
 * Copyright (c) 2020-2021, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/bl_common.h>
#include <common/desc_image_load.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <baikal_console.h>
#include <baikal_def.h>
#include <baikal_io_storage.h>
#include <platform_def.h>

/* Data structure which holds the extents of the trusted SRAM for BL2 */
static meminfo_t bl2_tzram_layout __aligned(CACHE_WRITEBACK_GRANULE);

void bl2_early_platform_setup2(u_register_t arg0,
			       u_register_t arg1,
			       u_register_t arg2,
			       u_register_t arg3)
{
	meminfo_t *mem_layout = (meminfo_t *)arg1;

	baikal_console_boot_init();

	/* Setup the BL2 memory layout */
	bl2_tzram_layout = *mem_layout;
	plat_baikal_io_setup();
}

void bl2_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_REGION_FLAT(bl2_tzram_layout.total_base,
				bl2_tzram_layout.total_size,
				MT_MEMORY | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(BL_CODE_BASE,
				BL_CODE_END - BL_CODE_BASE,
				MT_CODE | MT_SECURE),

		{0}
	};

	const mmap_region_t plat_bs1000_mmap[] = {
		MAP_REGION_FLAT(SEC_DRAM_BASE,
				SEC_DRAM_SIZE,
				MT_MEMORY | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(UART_A1_BASE,
				UART_A1_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE),

		MAP_REGION_FLAT(NS_DRAM1_BASE,
				NS_DRAM1_SIZE,
				MT_MEMORY | MT_RW | MT_NS),

		{0}
	};

	setup_page_tables(bl_regions, plat_bs1000_mmap);
	enable_mmu_el1(0);
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	if (image_id == BL33_IMAGE_ID) {
		unsigned int mode;
		bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);

		assert(bl_mem_params);

		/* Figure out what mode we enter the non-secure world in */
		mode = (el_implemented(2) != EL_IMPL_NONE) ? MODE_EL2 : MODE_EL1;

		/* BL33 expects to receive the primary CPU MPID (through r0) */
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
		bl_mem_params->ep_info.spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	}

	return 0;
}

void bl2_platform_setup(void)
{
}
