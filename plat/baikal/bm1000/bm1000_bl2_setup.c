/*
 * Copyright (c) 2018-2022, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <libfdt.h>
#ifdef SPD_opteed
#include <optee_utils.h>
#endif

#include <baikal_console.h>
#include <baikal_def.h>
#include <baikal_fdt.h>
#include <baikal_io_storage.h>
#include <bm1000_private.h>
#include <bm1000_smbus.h>
#include <crc.h>
#include <ddr_spd.h>
#include <dw_i2c.h>
#include <platform_def.h>
#include <spd.h>

/*
 * The next 2 constants identify the extents of the code & RO data region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned. It is the responsibility of the linker script to ensure that
 * __RO_START__ and __RO_END__ linker symbols refer to page-aligned addresses.
 */
#define BL2_RO_BASE (unsigned long)(&__RO_START__)
#define BL2_RO_LIMIT (unsigned long)(&__RO_END__)

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

void bl2_platform_setup(void)
{
	void *fdt = (void *)(uintptr_t)BAIKAL_SEC_DTB_BASE;
	int ret;

	ret = fdt_open_into(fdt, fdt, BAIKAL_DTB_MAX_SIZE);
	if (ret >= 0) {
		unsigned int dimm_idx;
		/*
		 * DBM has 4 DIMM slots with the following SPD addresses:
		 * 0x50, 0x51, 0x52, 0x53. But only 0x50 and 0x52 are used by SCP FW.
		 */
		const uint8_t dimm_spd_addrs[] = {0x50, 0x52};
		unsigned long long total_capacity = 0;

		struct spd_container *const spd_ptr = (void *)PLAT_DDR_SPD_BASE;

		/* Read DDR4 DIMM SPD EEPROMs for base configuration and DRAM parameters */
		for (dimm_idx = 0; dimm_idx < ARRAY_SIZE(dimm_spd_addrs); ++dimm_idx) {
			if (spd_ptr->content[dimm_idx].mem_type == SPD_MEMTYPE_DDR4) {
				unsigned int half_bus_shift = 0;

				if (dimm_idx) {
					if (mmio_read_32(MMDDR1_CTRL_BASE) & BIT(12)) {
						half_bus_shift = 1;
					}
				} else {
					if (mmio_read_32(MMDDR0_CTRL_BASE) & BIT(12)) {
						half_bus_shift = 1;
					}
				}

				const unsigned long long dimm_capacity =
					spd_get_baseconf_dimm_capacity(&spd_ptr->content[dimm_idx])
						>> half_bus_shift;

				if (dimm_capacity > 0) {
#ifdef BAIKAL_DUAL_CHANNEL_MODE
					if (spd_ptr->dual_channel[dimm_idx] == 'y') {
						INFO("DIMM%u: capacity is %lld MiB\n", dimm_idx,
									dimm_capacity * 2 / (1024 * 1024));
						total_capacity += dimm_capacity * 2;
					} else {
						INFO("DIMM%u: capacity is %lld MiB\n", dimm_idx,
									dimm_capacity / (1024 * 1024));
						total_capacity += dimm_capacity;
					}
#else
					INFO("DIMM%u: capacity is %lld MiB\n", dimm_idx, dimm_capacity / (1024 * 1024));
					total_capacity += dimm_capacity;
#endif
					dt_enable_mc_node(fdt, dimm_idx == 0 ? MMDDR0_CTRL_BASE : MMDDR1_CTRL_BASE);
				} else {
					ERROR("DIMM%u: capacity is unidentified\n", dimm_idx);
				}
			}
		}

		if (total_capacity > 0) {
			uint64_t region_descs[3][2];
			unsigned int region_num;

			region_descs[0][0] = REGION_DRAM0_BASE;
			region_descs[1][0] = REGION_DRAM1_BASE;
			region_descs[2][0] = REGION_DRAM2_BASE;

			if (total_capacity <= REGION_DRAM0_SIZE) {
				region_descs[0][1] = total_capacity;
				region_num = 1;
			} else {
				region_descs[0][1] = REGION_DRAM0_SIZE;
				if (total_capacity <= (REGION_DRAM0_SIZE + REGION_DRAM1_SIZE)) {
					region_descs[1][1] = total_capacity - REGION_DRAM0_SIZE;
					region_num = 2;
				} else {
					region_descs[1][1] = REGION_DRAM1_SIZE;
					region_descs[2][1] = total_capacity - (REGION_DRAM0_SIZE + REGION_DRAM1_SIZE);
					region_num = 3;
				}
			}

			fdt_memory_node_set(fdt, region_descs, region_num);
		}

		ret = fdt_pack(fdt);
		if (ret < 0) {
			ERROR("Failed to pack FDT at %p, error %d\n", fdt, ret);
		}

		flush_dcache_range((unsigned long)fdt, BAIKAL_DTB_MAX_SIZE);
	} else {
		ERROR("Invalid FDT at %p, error %d\n", fdt, ret);
	}
}

void bl2_plat_arch_setup(void)
{
	baikal_configure_mmu_el1(bl2_tzram_layout.total_base,
				 bl2_tzram_layout.total_size,
				 BL2_RO_BASE, BL2_RO_LIMIT,
				 BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_END);
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	int err = 0;
	unsigned int mode;
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);
#ifdef SPD_opteed
	bl_mem_params_node_t *pager_mem_params = NULL;
	bl_mem_params_node_t *paged_mem_params = NULL;
#endif
	assert(bl_mem_params);

	switch (image_id) {
#ifdef __aarch64__
	case BL32_IMAGE_ID:
#ifdef SPD_opteed
		pager_mem_params = get_bl_mem_params_node(BL32_EXTRA1_IMAGE_ID);
		assert(pager_mem_params);

		paged_mem_params = get_bl_mem_params_node(BL32_EXTRA2_IMAGE_ID);
		assert(paged_mem_params);

		err = parse_optee_header(&bl_mem_params->ep_info,
					 &pager_mem_params->image_info,
					 &paged_mem_params->image_info);
		if (err != 0) {
			WARN("OPTEE header parse error.\n");
		}

		/*
		 * OP-TEE expect to receive DTB address in x2.
		 * This will be copied into x2 by dispatcher.
		 */
		bl_mem_params->ep_info.args.arg3 = BAIKAL_SEC_DTB_BASE;
#endif
		/*
		 * The Secure Payload Dispatcher service is responsible for
		 * setting the SPSR prior to entry into the BL3-2 image.
		 */
		bl_mem_params->ep_info.spsr = 0;
		break;
#endif
	case BL33_IMAGE_ID:
		memcpy((void *)BAIKAL_NS_DTB_BASE,
		       (void *)BAIKAL_SEC_DTB_BASE,
		       BAIKAL_DTB_MAX_SIZE);

		flush_dcache_range((uintptr_t)BAIKAL_NS_DTB_BASE, BAIKAL_DTB_MAX_SIZE);

		/* Figure out what mode we enter the non-secure world in */
		mode = (el_implemented(2) != EL_IMPL_NONE) ? MODE_EL2 : MODE_EL1;

		/* BL33 expects to receive the primary CPU MPID (through r0) */
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
		bl_mem_params->ep_info.spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
		break;
	}

	return err;
}
