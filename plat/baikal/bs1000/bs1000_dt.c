/*
 * Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <libfdt.h>

#include <bs1000_dimm_spd.h>
#include <baikal_def.h>
#include <baikal_fdt.h>
#include <ddr_spd.h>
#include <crc.h>
#include <spd.h>

static uint64_t baikal_detect_sdram_capacity(unsigned int chip_idx)
{
	unsigned int dimm_idx;
	uint64_t total_capacity = 0;

	for (dimm_idx = 0; ; ++dimm_idx) {
		const uint8_t *buf = baikal_dimm_spd_get(chip_idx, dimm_idx);

		if (buf == NULL) {
			break;
		}

		if (crc16(buf, 126, 0) == spd_get_baseconf_crc(buf)) {
			const uint64_t dimm_capacity = spd_get_baseconf_dimm_capacity(buf);

			INFO("DIMM%u_%u: %lu MiB\n",
			     (chip_idx * BS1000_DIMM_NUM + dimm_idx) / 2,
			     dimm_idx % 2,
			     dimm_capacity / (1024 * 1024));

			total_capacity += dimm_capacity;
		}
	}
#ifdef BAIKAL_QEMU
	if (total_capacity == 0) {
		total_capacity = (uint64_t)2 * 1024 * 1024 * 1024;
	}
#endif
	return total_capacity;
}

static void dt_enable_mc_node(void *fdt, const uintptr_t base)
{
	int node = -1;

	for (;;) {
		const uint32_t *prop;
		int proplen;
		uint64_t reg;

		node = fdt_node_offset_by_compatible(fdt, node, "baikal,bs1000-edac-mc");
		if (node < 0) {
			ERROR("%s: unable to find 'edac-mc' with 'reg' = 0x%lx\n", __func__, base);
			return;
		}

		prop = fdt_getprop(fdt, node, "reg", &proplen);
		if (prop == NULL) {
			ERROR("%s: 'reg' is not found\n", __func__);
			continue;
		} else if (proplen != 16) {
			ERROR("%s: incorrect 'reg' property length\n", __func__);
			continue;
		}

		/* Avoid 'fdt64_to_cpu()' with prop pointer: it could lead to unaligned access */
		reg  = fdt32_to_cpu(prop[0]);
		reg <<= 32;
		reg |= fdt32_to_cpu(prop[1]);

		if (reg == base) {
			int err;

			err = fdt_setprop_string(fdt, node, "status", "okay");
			if (err) {
				ERROR("%s: unable to set 'status' property of node\n", __func__);
			}

			return;
		}
	}
}

#define MAX_NUM_MEM_REGIONS	3
#define MAX_NODE_NAME_SIZE	20

void fdt_memory_node_set(void *fdt,
			 const uint64_t region_descs[][2],
			 const unsigned int region_num)
{
	int err;
	uint64_t memregs[MAX_NUM_MEM_REGIONS][2];
	char node_name[MAX_NODE_NAME_SIZE];
	int node;
	unsigned int region;

	err = snprintf(node_name, MAX_NODE_NAME_SIZE, "/memory@%lx",
		       region_descs[0][0]);
	if (err < 0 || err >= MAX_NODE_NAME_SIZE) {
		ERROR("%s: memory subnode name too long\n", __func__);
		return;
	}
	node = fdt_path_offset(fdt, node_name);
	if (node == -FDT_ERR_NOTFOUND) {
		node = fdt_add_subnode(fdt, 0, node_name + 1);
		if (node < 0) {
			ERROR("%s: unable to add memory subnode\n", __func__);
			return;
		}

		err = fdt_setprop_string(fdt, node, "device_type", "memory");
		if (err) {
			ERROR("%s: unable to set 'device_type' of memory subnode\n", __func__);
			return;
		}
	}

	for (region = 0; region < region_num; ++region) {
		memregs[region][0] = cpu_to_fdt64(region_descs[region][0]);
		memregs[region][1] = cpu_to_fdt64(region_descs[region][1]);
	}

	err = fdt_setprop(fdt, node, "reg", memregs, sizeof(memregs[0]) * region_num);
	if (err) {
		ERROR("%s: unable to set 'reg' property of memory subnode\n", __func__);
		return;
	}

	err = fdt_setprop_u32(fdt, node, "numa-node-id",
			      PLATFORM_ADDR_CHIP(region_descs[0][0]));
	if (err) {
		ERROR("%s: unable to set 'numa-node-id' property of memory subnode\n", __func__);
		return;
	}
}

void baikal_fdt_memory_update(void)
{
	void *fdt = (void *)BAIKAL_SEC_DTB_BASE;
	uint64_t region_descs[MAX_NUM_MEM_REGIONS][2];
	unsigned int region_num;
	int ret;
	uint64_t remain_capacity;
	unsigned int chip_idx;

	ret = fdt_open_into(fdt, fdt, BAIKAL_DTB_MAX_SIZE);
	if (ret < 0) {
		ERROR("%s: failed to open FDT @ %p, error %d\n", __func__, fdt, ret);
		return;
	}

	for (chip_idx = 0; chip_idx < PLATFORM_CHIP_COUNT; ++chip_idx) {
		region_descs[0][0] = PLATFORM_ADDR_OUT_CHIP(chip_idx,
							    REGION_DRAM0_BASE);
		region_descs[0][1] = REGION_DRAM0_SIZE;
		region_descs[1][0] = PLATFORM_ADDR_OUT_CHIP(chip_idx,
							    REGION_DRAM1_BASE);
		region_descs[1][1] = REGION_DRAM1_SIZE;
		region_descs[2][0] = PLATFORM_ADDR_OUT_CHIP(chip_idx,
							    REGION_DRAM2_BASE);
		region_descs[2][1] = REGION_DRAM2_SIZE;
		remain_capacity = baikal_detect_sdram_capacity(chip_idx);
		if (remain_capacity > 0) {
			for (region_num = 0; region_num < MAX_NUM_MEM_REGIONS; ++region_num) {
				if (remain_capacity <= region_descs[region_num][1]) {
					region_descs[region_num++][1] = remain_capacity;
					break;
				}
				remain_capacity -= region_descs[region_num][1];
			}
			fdt_memory_node_set(fdt, region_descs, region_num);
		}
	}

	ret = fdt_pack(fdt);
	if (ret < 0) {
		ERROR("%s: failed to pack FDT @ %p, error %d\n", __func__, fdt, ret);
	}
}

void baikal_fdt_ddr_node_enable(void)
{
	int ret;
	uintptr_t base[] = {
		DDR0_CTRL_BASE,
		DDR1_CTRL_BASE,
		DDR2_CTRL_BASE,
		DDR3_CTRL_BASE,
		DDR4_CTRL_BASE,
		DDR5_CTRL_BASE
	};
	struct ddr4_spd_eeprom *spd_content;
	void *fdt = (void *)BAIKAL_SEC_DTB_BASE;
	unsigned int chip_idx, dimm_idx, ddr_idx;

	ret = fdt_open_into(fdt, fdt, BAIKAL_DTB_MAX_SIZE);
	if (ret < 0) {
		ERROR("%s: failed to open FDT @ %p, error %d\n", __func__, fdt, ret);
		return;
	}

	for (chip_idx = 0; chip_idx < PLATFORM_CHIP_COUNT; ++chip_idx) {
		for (dimm_idx = 0; ; ++dimm_idx) {
			spd_content = (struct ddr4_spd_eeprom *)baikal_dimm_spd_get(chip_idx, dimm_idx);
			if (spd_content == NULL) {
				break;
			}
			if (dimm_idx % 2)
				continue;
			if (spd_content->mem_type == SPD_MEMTYPE_DDR4) {
				ddr_idx = dimm_idx / 2;
				if (ddr_idx >= ARRAY_SIZE(base)) {
					ERROR("%s: wrong dimm index - %d\n", __func__, dimm_idx);
					break;
				}
				dt_enable_mc_node(fdt, PLATFORM_ADDR_OUT_CHIP(chip_idx, base[ddr_idx]));
			}
		}
	}
}
