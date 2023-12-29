/*
 * Copyright (c) 2018-2022, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <libfdt.h>

#include <bm1000_private.h>
#include <platform_def.h>

int fdt_memory_node_read(uint64_t region_descs[3][2])
{
	void *fdt = (void *)(uintptr_t)BAIKAL_SEC_DTB_BASE;
	int memoff;
	unsigned int region;
	const uint32_t *prop;
	int proplen;
	int ret;

	for (region = 0; region < 3; ++region) {
		region_descs[region][0] = 0;
		region_descs[region][1] = 0;
	}

	ret = fdt_open_into(fdt, fdt, BAIKAL_DTB_MAX_SIZE);
	if (ret < 0) {
		ERROR("%s: unable to open FDT (%d)\n", __func__, ret);
		return ret;
	}

	memoff = fdt_path_offset(fdt, "/memory@80000000");
	if (memoff == -FDT_ERR_NOTFOUND) {
		ERROR("%s: node is not found\n", __func__);
		return memoff;
	}

	prop = fdt_getprop(fdt, memoff, "reg", &proplen);
	if (prop == NULL) {
		ERROR("%s: 'reg' is not found\n", __func__);
		return -1;
	} else if (!proplen || (proplen % 16) || proplen > 48) {
		ERROR("%s: incorrect 'reg' property length\n", __func__);
		return -1;
	}

	for (region = 0; region < proplen / 16; ++region) {
		uint64_t val;

		/* Avoid 'fdt64_to_cpu()' with prop pointer: it could lead to unaligned access */
		val  = fdt32_to_cpu(prop[4 * region + 0]);
		val <<= 32;
		val |= fdt32_to_cpu(prop[4 * region + 1]);
		region_descs[region][0] = val;

		val  = fdt32_to_cpu(prop[4 * region + 2]);
		val <<= 32;
		val |= fdt32_to_cpu(prop[4 * region + 3]);
		region_descs[region][1] = val;
	}

	return 0;
}

void dt_enable_mc_node(void *fdt, const uintptr_t base)
{
	int node = -1;

	for (;;) {
		const uint32_t *prop;
		int proplen;
		uint64_t reg;

		node = fdt_node_offset_by_compatible(fdt, node, "baikal,bm1000-edac-mc");
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
