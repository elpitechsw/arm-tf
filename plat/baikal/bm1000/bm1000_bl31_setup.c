/*
 * Copyright (c) 2018-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <drivers/generic_delay_timer.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#include <baikal_gicv3.h>
#include <bm1000_cmu.h>
#include <bm1000_def.h>
#include <bm1000_private.h>
#include <platform_def.h>

#include "bm1000_mmca57.h"
#include "bm1000_splash.h"

#define XSTR(x) STR(x)
#define STR(x) #x

/*
 * The next 3 constants identify the extents of the code, RO data region and the
 * limit of the BL3-1 image.  These addresses are used by the MMU setup code and
 * therefore they must be page-aligned.  It is the responsibility of the linker
 * script to ensure that __RO_START__, __RO_END__ & __BL31_END__ linker symbols
 * refer to page-aligned addresses.
 */
#define BL31_RO_BASE (unsigned long)(&__RO_START__)
#define BL31_RO_LIMIT (unsigned long)(&__RO_END__)
#define BL31_END (unsigned long)(&__BL31_END__)

extern uint8_t bl31_logo[];

static void bl31_splash(void)
{
	modeline_t old_lvds_mode, new_lvds_mode;
	int do_lvds_init;

#ifdef SDK_VERSION
	snprintf(sdk_version, SDK_VERSION_SIZE, "== v%s ==", XSTR(SDK_VERSION));
#endif
	display_logo_and_version(MMXGBE_VDU_BASE, FB2_BASE, &hdmi_video_mode, bl31_logo);
#if DEBUG
	vdu_init(MMXGBE_VDU_BASE, (uintptr_t)FB2_BASE, &hdmi_video_mode);
#endif
	memcpy(&old_lvds_mode, &lvds_video_mode, sizeof(lvds_video_mode));
	memcpy(&new_lvds_mode, &lvds_video_mode, sizeof(lvds_video_mode));
	if (!fdt_get_panel(&new_lvds_mode)) {
		display_logo_and_version(MMAVLSP_VDU_BASE, FB0_BASE, &new_lvds_mode, bl31_logo);
#if !DEBUG
		do_lvds_init = memcmp(&old_lvds_mode, &new_lvds_mode, sizeof(old_lvds_mode));
#else
		do_lvds_init = 1;
#endif
		if (do_lvds_init) {
			vdu_init(MMAVLSP_VDU_BASE, (uintptr_t)FB0_BASE,
				 &new_lvds_mode);
		}
	}
}

void bl31_plat_arch_setup(void)
{
	plat_arm_interconnect_init();
	plat_arm_interconnect_enter_coherency();

	baikal_configure_mmu_el3(BL31_RO_BASE, (BL31_END - BL31_RO_BASE),
				 BL31_RO_BASE, BL31_RO_LIMIT,
				 BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_END);
}

void bl31_platform_setup(void)
{
	generic_delay_timer_init();
#if DEBUG
	INFO("Init AVLSP...\n");
	mmavlsp_init();
#endif
	INFO("Init CA57 PVTs...\n");
	cmu_clkch_enable_by_base(MMCA57_0_PVT_CLKCHCTL, PVT_CLKCH_DIV);
	cmu_clkch_enable_by_base(MMCA57_1_PVT_CLKCHCTL, PVT_CLKCH_DIV);
	cmu_clkch_enable_by_base(MMCA57_2_PVT_CLKCHCTL, PVT_CLKCH_DIV);
	cmu_clkch_enable_by_base(MMCA57_3_PVT_CLKCHCTL, PVT_CLKCH_DIV);

	INFO("Init XGbE...\n");
#if DEBUG
	mmxgbe_init();
	hdmi_tx_init();
#endif
	mmxgbe_ns_access();

	INFO("Init USB...\n");
	mmusb_init();

	INFO("Init Mali...\n");
	mmmali_init();

	INFO("Init Mali PVT...\n");
	cmu_clkch_enable_by_base(MMMALI_PVT_CLKCHCTL, PVT_CLKCH_DIV);

	INFO("Init PCIe...\n");
	mmpcie_init();

	INFO("Init VDec...\n");
	mmvdec_init();

	INFO("Init CoreSight...\n");
	mmcoresight_init();

	baikal_gic_driver_init();
	baikal_gic_init();

	bl31_splash();
}

void bl31_plat_enable_mmu(uint32_t flags)
{
	enable_mmu_el3(flags);
}
