/*
 * Copyright (c) 2018-2021, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_compat.h>

#include <baikal_scp.h>
#include <bm1000_def.h>
#include <platform_def.h>

#define MAP_DEVICE0	MAP_REGION_FLAT(DEVICE0_BASE,			\
					DEVICE0_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_DEVICE1	MAP_REGION_FLAT(DEVICE1_BASE,			\
					DEVICE1_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_MAILBOX_IRB	MAP_REGION_FLAT(MAILBOX_IRB_BASE,		\
					MAILBOX_IRB_SIZE,		\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_SHARED_RAM	MAP_REGION_FLAT(SHARED_SRAM_BASE,		\
					SHARED_SRAM_SIZE,		\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_BL32_MEM	MAP_REGION_FLAT(BL32_MEM_BASE, BL32_MEM_SIZE,	\
					MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_NS_DRAM0	MAP_REGION_FLAT(NS_DRAM0_BASE, NS_DRAM0_SIZE,	\
					MT_MEMORY | MT_RW | MT_NS)

#define MAP_NS_DRAM1	MAP_REGION_FLAT(NS_DRAM1_BASE, NS_DRAM1_SIZE,	\
					MT_MEMORY | MT_RW | MT_NS)

#define MAP_FRAMEBUFFER	MAP_REGION_FLAT(FB0_BASE, (3 * FB_SIZE),	\
					MT_MEMORY | MT_RW | MT_NS)

#define MAP_SEC_DRAM	MAP_REGION_FLAT(SEC_DRAM_BASE, SEC_DRAM_SIZE,	\
					MT_MEMORY | MT_RW | MT_SECURE)

/*
 * Table of regions for various BL stages to map using the MMU.
 * This doesn't include TZRAM as the 'mem_layout' argument passed to
 * arm_configure_mmu_elx() will give the available subset of that.
 */
#ifdef IMAGE_BL1
static const mmap_region_t plat_baikal_mmap[] = {
	MAP_MAILBOX_IRB,
	MAP_NS_DRAM0,
	MAP_SEC_DRAM,
	MAP_NS_DRAM1,
	MAP_FRAMEBUFFER,
	MAP_SHARED_RAM,
	MAP_DEVICE0,
	MAP_DEVICE1,
	{0}
};

void baikal_configure_mmu_bl1(unsigned long total_base,
			      unsigned long total_size,
			      unsigned long ro_start,
			      unsigned long ro_limit,
			      unsigned long coh_start,
			      unsigned long coh_limit)
{
	mmap_add_region(total_base, total_base,
			total_size,
			MT_NON_CACHEABLE | MT_RW | MT_SECURE);

	mmap_add_region(ro_start, ro_start,
			ro_limit - ro_start,
			MT_NON_CACHEABLE | MT_RO | MT_EXECUTE | MT_SECURE);

	mmap_add_region(coh_start, coh_start,
			coh_limit - coh_start,
			MT_MEMORY | MT_RW | MT_SECURE);

	mmap_add(plat_baikal_mmap);
	init_xlat_tables();
	enable_mmu_el3(0);
}
#endif

#ifdef IMAGE_BL2
static const mmap_region_t plat_baikal_mmap[] = {
	MAP_NS_DRAM0,
	MAP_NS_DRAM1,
	MAP_SEC_DRAM,
	MAP_DEVICE0,
	MAP_DEVICE1,
	{0}
};
#endif

#ifdef IMAGE_BL31
static const mmap_region_t plat_baikal_mmap[] = {
	MAP_MAILBOX_IRB,
	MAP_NS_DRAM0,
	MAP_FRAMEBUFFER,
	MAP_SHARED_RAM,
	MAP_DEVICE0,
	MAP_DEVICE1,
	MAP_BL32_MEM,
	{0}
};
#endif

/*
 * Macro generating the code for the function setting up the pagetables as per
 * the platform memory map & initialize the mmu, for the given exception level
 */
#define DEFINE_CONFIGURE_MMU_EL(_el)					\
	void baikal_configure_mmu_el##_el(unsigned long total_base,	\
					  unsigned long total_size,	\
					  unsigned long ro_start,	\
					  unsigned long ro_limit,	\
					  unsigned long coh_start,	\
					  unsigned long coh_limit)	\
	{								\
		mmap_add_region(total_base, total_base,			\
				total_size,				\
				MT_MEMORY | MT_RW | MT_SECURE);		\
		mmap_add_region(ro_start, ro_start,			\
				ro_limit - ro_start,			\
				MT_NON_CACHEABLE | MT_RO | MT_EXECUTE | MT_SECURE); \
		mmap_add_region(coh_start, coh_start,			\
				coh_limit - coh_start,			\
				MT_NON_CACHEABLE | MT_RW | MT_SECURE);	\
		mmap_add(plat_baikal_mmap);				\
		init_xlat_tables();					\
		enable_mmu_el##_el(0);					\
	}

/* Define EL1 and EL3 variants of the function initialising the MMU */
DEFINE_CONFIGURE_MMU_EL(1)
DEFINE_CONFIGURE_MMU_EL(3)
