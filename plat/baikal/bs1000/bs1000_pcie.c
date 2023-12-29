/*
 * Copyright (c) 2022-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <libfdt.h>
#include <plat/common/platform.h>

#include <baikal_fdt.h>
#include <bs1000_def.h>

#include "bs1000_pcie.h"

#define PCIE_TYPE_RC	1
#define PCIE_TYPE_EP	2

void pcie_init(void)
{
	void *fdt = (void *)(uintptr_t)BAIKAL_SEC_DTB_BASE;
	unsigned int chip_idx, idx;
	int node = -1;

	const uintptr_t pcie_dbis[14] = {
		PCIE0_P0_DBI_BASE,
		PCIE0_P1_DBI_BASE,
		PCIE1_P0_DBI_BASE,
		PCIE1_P1_DBI_BASE,
		PCIE2_P0_DBI_BASE,
		PCIE2_P1_DBI_BASE,
		PCIE3_P0_DBI_BASE,
		PCIE3_P1_DBI_BASE,
		PCIE3_P2_DBI_BASE,
		PCIE3_P3_DBI_BASE,
		PCIE4_P0_DBI_BASE,
		PCIE4_P1_DBI_BASE,
		PCIE4_P2_DBI_BASE,
		PCIE4_P3_DBI_BASE
	};

	const uintptr_t pcie_nic_cfg_ps[ARRAY_SIZE(pcie_dbis)] = {
		PCIE0_NIC_CFG_P0,
		PCIE0_NIC_CFG_P1,
		PCIE1_NIC_CFG_P0,
		PCIE1_NIC_CFG_P1,
		PCIE2_NIC_CFG_P0,
		PCIE2_NIC_CFG_P1,
		PCIE3_NIC_CFG_P0,
		PCIE3_NIC_CFG_P1,
		PCIE3_NIC_CFG_P2,
		PCIE3_NIC_CFG_P3,
		PCIE4_NIC_CFG_P0,
		PCIE4_NIC_CFG_P1,
		PCIE4_NIC_CFG_P2,
		PCIE4_NIC_CFG_P3
	};

	const uintptr_t pcie_nic_slv_ps[ARRAY_SIZE(pcie_dbis)] = {
		PCIE0_NIC_SLV_P0,
		PCIE0_NIC_SLV_P1,
		PCIE1_NIC_SLV_P0,
		PCIE1_NIC_SLV_P1,
		PCIE2_NIC_SLV_P0,
		PCIE2_NIC_SLV_P1,
		PCIE3_NIC_SLV_P0,
		PCIE3_NIC_SLV_P1,
		PCIE3_NIC_SLV_P2,
		PCIE3_NIC_SLV_P3,
		PCIE4_NIC_SLV_P0,
		PCIE4_NIC_SLV_P1,
		PCIE4_NIC_SLV_P2,
		PCIE4_NIC_SLV_P3
	};

	unsigned int pcie_lanes[PLATFORM_CHIP_COUNT][ARRAY_SIZE(pcie_dbis)];
	unsigned int pcie_types[PLATFORM_CHIP_COUNT][ARRAY_SIZE(pcie_dbis)];

	if (fdt_open_into(fdt, fdt, BAIKAL_DTB_MAX_SIZE)) {
		return;
	}

	memset(pcie_lanes, 0, sizeof(pcie_lanes));
	memset(pcie_types, 0, sizeof(pcie_types));

	/* Configure PCIe in accordance with FDT */
	for (;;) {
		bool endpoint;
		unsigned int lanes = 0;
		const uint32_t *prop;
		int proplen;
		uint64_t reg;

		node = fdt_next_node(fdt, node, NULL);
		if (node < 0) {
			break;
		}

		if (!fdt_node_is_enabled(fdt, node)) {
			continue;
		}

		if (fdt_node_check_compatible(fdt, node, "baikal,bs1000-pcie") == 0) {
			endpoint = false;
		} else if (fdt_node_check_compatible(fdt, node, "baikal,bs1000-pcie-ep") == 0) {
			endpoint = true;
		} else {
			continue;
		}

		prop = fdt_getprop(fdt, node, "reg", &proplen);
		if (prop == NULL || proplen <= 0 || proplen % 16) {
			continue;
		}

		reg  = fdt32_to_cpu(prop[0]);
		reg <<= 32;
		reg |= fdt32_to_cpu(prop[1]);

		chip_idx = PLATFORM_ADDR_CHIP(reg);

		prop = fdt_getprop(fdt, node, "num-lanes", &proplen);
		if (prop != NULL && proplen == 4) {
			lanes = fdt32_to_cpu(prop[0]);
		}

		for (idx = 0; idx < ARRAY_SIZE(pcie_dbis); ++idx) {
			if (PLATFORM_ADDR_IN_CHIP(reg) == pcie_dbis[idx]) {
				assert(pcie_lanes[chip_idx][idx] == 0);
				assert(pcie_types[chip_idx][idx] == 0);

				pcie_lanes[chip_idx][idx] = lanes;
				pcie_types[chip_idx][idx] = endpoint ?
					PCIE_TYPE_EP : PCIE_TYPE_RC;
				break;
			}
		}
	}

	for (chip_idx = 0; chip_idx < PLATFORM_CHIP_COUNT; ++chip_idx) {
		for (idx = 0; idx < ARRAY_SIZE(pcie_dbis); ++idx) {
			if (pcie_types[chip_idx][idx] == 0) {
				continue;
			}

			/* Enable PCIe non-secure access */
			mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, pcie_nic_cfg_ps[idx]),
				      NIC_GPV_REGIONSEC_NONSECURE);

			switch (pcie_dbis[idx]) {
			case PCIE0_P0_DBI_BASE:
			case PCIE0_P1_DBI_BASE:
				mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE0_NIC_CFG_SMMU),
					      NIC_GPV_REGIONSEC_NONSECURE);
				mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE0_NIC_CFG_PHY),
					      NIC_GPV_REGIONSEC_NONSECURE);
				break;
			case PCIE1_P0_DBI_BASE:
			case PCIE1_P1_DBI_BASE:
				mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE1_NIC_CFG_SMMU),
					      NIC_GPV_REGIONSEC_NONSECURE);
				mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE1_NIC_CFG_PHY),
					      NIC_GPV_REGIONSEC_NONSECURE);
				break;
			case PCIE2_P0_DBI_BASE:
			case PCIE2_P1_DBI_BASE:
				mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE2_NIC_CFG_SMMU),
					      NIC_GPV_REGIONSEC_NONSECURE);
				mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE2_NIC_CFG_PHY),
					      NIC_GPV_REGIONSEC_NONSECURE);
				break;
			case PCIE3_P0_DBI_BASE:
			case PCIE3_P1_DBI_BASE:
			case PCIE3_P2_DBI_BASE:
			case PCIE3_P3_DBI_BASE:
				mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE3_NIC_CFG_SMMU),
					      NIC_GPV_REGIONSEC_NONSECURE);
				mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE3_NIC_CFG_PHY),
					      NIC_GPV_REGIONSEC_NONSECURE);
				break;
			case PCIE4_P0_DBI_BASE:
			case PCIE4_P1_DBI_BASE:
			case PCIE4_P2_DBI_BASE:
			case PCIE4_P3_DBI_BASE:
				mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE4_NIC_CFG_SMMU),
					      NIC_GPV_REGIONSEC_NONSECURE);
				mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE4_NIC_CFG_PHY),
					      NIC_GPV_REGIONSEC_NONSECURE);
				break;
			}

			mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, pcie_nic_slv_ps[idx]),
				      NIC_GPV_REGIONSEC_NONSECURE);

			/* Set PCIe device type */
			switch (pcie_dbis[idx]) {
			case PCIE0_P0_DBI_BASE:
				if (pcie_types[chip_idx][idx] == PCIE_TYPE_EP) {
					mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE0_GPR_SS_MODE_CTL),
							PCIE_GPR_SS_MODE_CTL_DEVTYPE_MASK);
				} else {
					mmio_clrsetbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE0_GPR_SS_MODE_CTL),
							   PCIE_GPR_SS_MODE_CTL_DEVTYPE_MASK,
							   0x40);
				}

				break;
			case PCIE1_P0_DBI_BASE:
				if (pcie_types[chip_idx][idx] == PCIE_TYPE_EP) {
					mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE1_GPR_SS_MODE_CTL),
							PCIE_GPR_SS_MODE_CTL_DEVTYPE_MASK);
				} else {
					mmio_clrsetbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE1_GPR_SS_MODE_CTL),
							   PCIE_GPR_SS_MODE_CTL_DEVTYPE_MASK,
							   0x40);
				}

				break;
			case PCIE2_P0_DBI_BASE:
				if (pcie_types[chip_idx][idx] == PCIE_TYPE_EP) {
					mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE2_GPR_SS_MODE_CTL),
							PCIE_GPR_SS_MODE_CTL_DEVTYPE_MASK);
				} else {
					mmio_clrsetbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE2_GPR_SS_MODE_CTL),
							   PCIE_GPR_SS_MODE_CTL_DEVTYPE_MASK,
							   0x40);
				}

				break;
			default:
				if (pcie_types[chip_idx][idx] == PCIE_TYPE_EP) {
					ERROR("pcie@0x%llx: incorrect device type\n",
						PLATFORM_ADDR_OUT_CHIP(chip_idx, pcie_dbis[idx]));
					plat_panic_handler();
				}

				break;
			}
		}

		/* Set PCIe subsystem mode */
		if (pcie_types[chip_idx][1] > 0) {
			if (pcie_lanes[chip_idx][0] <= 8 &&
			    pcie_lanes[chip_idx][1] <= 8) {
				mmio_clrsetbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE0_GPR_SS_MODE_CTL),
						   PCIE_GPR_SS_MODE_CTL_SSMODE_MASK,
						   0x2);
			} else {
				ERROR("Chip%u: PCIe0: incorrect subsystem mode\n", chip_idx);
				plat_panic_handler();
			}
		} else if (pcie_types[chip_idx][0] > 0) {
			if (pcie_lanes[chip_idx][0] <= 16) {
				mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE0_GPR_SS_MODE_CTL),
						PCIE_GPR_SS_MODE_CTL_SSMODE_MASK);
			} else {
				ERROR("Chip%u: PCIe0: incorrect subsystem mode\n", chip_idx);
				plat_panic_handler();
			}
		}

		if (pcie_types[chip_idx][3] > 0) {
			if (pcie_lanes[chip_idx][2] <= 8 &&
			    pcie_lanes[chip_idx][3] <= 8) {
				mmio_clrsetbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE1_GPR_SS_MODE_CTL),
						   PCIE_GPR_SS_MODE_CTL_SSMODE_MASK,
						   0x2);
			} else {
				ERROR("Chip%u: PCIe1: incorrect subsystem mode\n", chip_idx);
				plat_panic_handler();
			}
		} else if (pcie_types[chip_idx][2] > 0) {
			if (pcie_lanes[chip_idx][2] <= 16) {
				mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE1_GPR_SS_MODE_CTL),
						PCIE_GPR_SS_MODE_CTL_SSMODE_MASK);
			} else {
				ERROR("Chip%u: PCIe1: incorrect subsystem mode\n", chip_idx);
				plat_panic_handler();
			}
		}

		if (pcie_types[chip_idx][5] > 0) {
			if (pcie_lanes[chip_idx][4] <= 8 &&
			    pcie_lanes[chip_idx][5] <= 8) {
				mmio_clrsetbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE2_GPR_SS_MODE_CTL),
						   PCIE_GPR_SS_MODE_CTL_SSMODE_MASK,
						   0x2);
			} else {
				ERROR("Chip%u: PCIe2: incorrect subsystem mode\n", chip_idx);
				plat_panic_handler();
			}
		} else if (pcie_types[chip_idx][4] > 0) {
			if (pcie_lanes[chip_idx][4] <= 16) {
				mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE2_GPR_SS_MODE_CTL),
						PCIE_GPR_SS_MODE_CTL_SSMODE_MASK);
			} else {
				ERROR("Chip%u: PCIe2: incorrect subsystem mode\n", chip_idx);
				plat_panic_handler();
			}
		}

		if (pcie_types[chip_idx][7] > 0) {
			if (pcie_lanes[chip_idx][6] <= 4 &&
			    pcie_lanes[chip_idx][7] <= 4 &&
			    pcie_lanes[chip_idx][8] <= 4 &&
			    pcie_lanes[chip_idx][9] <= 4) {
				mmio_clrsetbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE3_GPR_SS_MODE_CTL),
						   PCIE_GPR_SS_MODE_CTL_SSMODE_MASK,
						   0x2);
			} else {
				ERROR("Chip%u: PCIe3: incorrect subsystem mode\n", chip_idx);
			}
		} else if (pcie_types[chip_idx][9] > 0) {
			if (pcie_lanes[chip_idx][6] <= 8 &&
			    pcie_lanes[chip_idx][8] <= 4 &&
			    pcie_lanes[chip_idx][9] <= 4) {
				mmio_clrsetbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE3_GPR_SS_MODE_CTL),
						   PCIE_GPR_SS_MODE_CTL_SSMODE_MASK,
						   0x3);
			} else {
				ERROR("Chip%u: PCIe3: incorrect subsystem mode\n", chip_idx);
			}
		} else if (pcie_types[chip_idx][8] > 0) {
			if (pcie_lanes[chip_idx][6] <= 8 &&
			    pcie_lanes[chip_idx][8] <= 8) {
				mmio_clrsetbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE3_GPR_SS_MODE_CTL),
						   PCIE_GPR_SS_MODE_CTL_SSMODE_MASK,
						   0x1);
			} else {
				ERROR("Chip%u: PCIe3: incorrect subsystem mode\n", chip_idx);
			}
		} else if (pcie_types[chip_idx][6] > 0) {
			if (pcie_lanes[chip_idx][6] <= 16) {
				mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE3_GPR_SS_MODE_CTL),
						PCIE_GPR_SS_MODE_CTL_SSMODE_MASK);
			} else {
				ERROR("Chip%u: PCIe3: incorrect subsystem mode\n", chip_idx);
			}
		}

		if (pcie_types[chip_idx][11] > 0) {
			if (pcie_lanes[chip_idx][10] <= 4 &&
			    pcie_lanes[chip_idx][11] <= 4 &&
			    pcie_lanes[chip_idx][12] <= 4 &&
			    pcie_lanes[chip_idx][13] <= 4) {
				mmio_clrsetbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE4_GPR_SS_MODE_CTL),
						   PCIE_GPR_SS_MODE_CTL_SSMODE_MASK,
						   0x2);
			} else {
				ERROR("Chip%u: PCIe4: incorrect subsystem mode\n", chip_idx);
			}
		} else if (pcie_types[chip_idx][13] > 0) {
			if (pcie_lanes[chip_idx][10] <= 8 &&
			    pcie_lanes[chip_idx][12] <= 4 &&
			    pcie_lanes[chip_idx][13] <= 4) {
				mmio_clrsetbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE4_GPR_SS_MODE_CTL),
						   PCIE_GPR_SS_MODE_CTL_SSMODE_MASK,
						   0x3);
			} else {
				ERROR("Chip%u: PCIe4: incorrect subsystem mode\n", chip_idx);
			}
		} else if (pcie_types[chip_idx][12] > 0) {
			if (pcie_lanes[chip_idx][10] <= 8 &&
			    pcie_lanes[chip_idx][12] <= 8) {
				mmio_clrsetbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE4_GPR_SS_MODE_CTL),
						   PCIE_GPR_SS_MODE_CTL_SSMODE_MASK,
						   0x1);
			} else {
				ERROR("Chip%u: PCIe4: incorrect subsystem mode\n", chip_idx);
			}
		} else if (pcie_types[chip_idx][10] > 0) {
			if (pcie_lanes[chip_idx][10] <= 16) {
				mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE4_GPR_SS_MODE_CTL),
						PCIE_GPR_SS_MODE_CTL_SSMODE_MASK);
			} else {
				ERROR("Chip%u: PCIe4: incorrect subsystem mode\n", chip_idx);
			}
		}

		/* Set ITS target address */
		for (idx = 0; idx < ARRAY_SIZE(pcie_dbis); ++idx) {
			if (pcie_types[chip_idx][idx] != PCIE_TYPE_RC) {
				continue;
			}

			switch (pcie_dbis[idx]) {
			case PCIE0_P0_DBI_BASE:
			case PCIE0_P1_DBI_BASE:
				mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE0_GPR_ITS0_TRGTADDR_CTL),
					      PLATFORM_ADDR_OUT_CHIP(chip_idx, GITS0_TRANSLATER) >> 16);
				break;
			case PCIE1_P0_DBI_BASE:
			case PCIE1_P1_DBI_BASE:
				mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE1_GPR_ITS0_TRGTADDR_CTL),
					      PLATFORM_ADDR_OUT_CHIP(chip_idx, GITS2_TRANSLATER) >> 16);
				break;
			case PCIE2_P0_DBI_BASE:
			case PCIE2_P1_DBI_BASE:
				mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE2_GPR_ITS0_TRGTADDR_CTL),
					      PLATFORM_ADDR_OUT_CHIP(chip_idx, GITS4_TRANSLATER) >> 16);
				break;
			case PCIE3_P0_DBI_BASE:
				mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE3_GPR_ITS0_TRGTADDR_CTL),
					      PLATFORM_ADDR_OUT_CHIP(chip_idx, GITS6_TRANSLATER) >> 16);
				break;
			case PCIE3_P1_DBI_BASE:
				mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE3_GPR_ITS1_TRGTADDR_CTL),
					      PLATFORM_ADDR_OUT_CHIP(chip_idx, GITS7_TRANSLATER) >> 16);
				break;
			case PCIE3_P2_DBI_BASE:
				mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE3_GPR_ITS2_TRGTADDR_CTL),
					      PLATFORM_ADDR_OUT_CHIP(chip_idx, GITS8_TRANSLATER) >> 16);
				break;
			case PCIE3_P3_DBI_BASE:
				mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE3_GPR_ITS3_TRGTADDR_CTL),
					      PLATFORM_ADDR_OUT_CHIP(chip_idx, GITS9_TRANSLATER) >> 16);
				break;
			case PCIE4_P0_DBI_BASE:
				mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE4_GPR_ITS0_TRGTADDR_CTL),
					      PLATFORM_ADDR_OUT_CHIP(chip_idx, GITS11_TRANSLATER) >> 16);
				break;
			case PCIE4_P1_DBI_BASE:
				mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE4_GPR_ITS1_TRGTADDR_CTL),
					      PLATFORM_ADDR_OUT_CHIP(chip_idx, GITS12_TRANSLATER) >> 16);
				break;
			case PCIE4_P2_DBI_BASE:
				mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE4_GPR_ITS2_TRGTADDR_CTL),
					      PLATFORM_ADDR_OUT_CHIP(chip_idx, GITS13_TRANSLATER) >> 16);
				break;
			case PCIE4_P3_DBI_BASE:
				mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE4_GPR_ITS3_TRGTADDR_CTL),
					      PLATFORM_ADDR_OUT_CHIP(chip_idx, GITS14_TRANSLATER) >> 16);
				break;
			}
		}

		/* Assert power-up reset */
		for (idx = 0; idx < ARRAY_SIZE(pcie_dbis); ++idx) {
			if (pcie_types[chip_idx][idx] == 0) {
				continue;
			}

			switch (pcie_dbis[idx]) {
			case PCIE0_P0_DBI_BASE:
			case PCIE0_P1_DBI_BASE:
				mmio_setbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE0_GPR_PWRUP_RST_CTL),
						PCIE_GPR_PWRUP_RST_CTL_PWRUP_RST);
				break;
			case PCIE1_P0_DBI_BASE:
			case PCIE1_P1_DBI_BASE:
				mmio_setbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE1_GPR_PWRUP_RST_CTL),
						PCIE_GPR_PWRUP_RST_CTL_PWRUP_RST);
				break;
			case PCIE2_P0_DBI_BASE:
			case PCIE2_P1_DBI_BASE:
				mmio_setbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE2_GPR_PWRUP_RST_CTL),
						PCIE_GPR_PWRUP_RST_CTL_PWRUP_RST);
				break;
			case PCIE3_P0_DBI_BASE:
			case PCIE3_P1_DBI_BASE:
			case PCIE3_P2_DBI_BASE:
			case PCIE3_P3_DBI_BASE:
				mmio_setbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE3_GPR_PWRUP_RST_CTL),
						PCIE_GPR_PWRUP_RST_CTL_PWRUP_RST);
				break;
			case PCIE4_P0_DBI_BASE:
			case PCIE4_P1_DBI_BASE:
			case PCIE4_P2_DBI_BASE:
			case PCIE4_P3_DBI_BASE:
				mmio_setbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE4_GPR_PWRUP_RST_CTL),
						PCIE_GPR_PWRUP_RST_CTL_PWRUP_RST);
				break;
			}
		}

		/* Clear PERST */
		mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE0_GPR_PWRUP_RST_CTL),
				PCIE_GPR_PWRUP_RST_CTL_P0_PERST |
				PCIE_GPR_PWRUP_RST_CTL_P1_PERST);

		mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE1_GPR_PWRUP_RST_CTL),
				PCIE_GPR_PWRUP_RST_CTL_P0_PERST |
				PCIE_GPR_PWRUP_RST_CTL_P1_PERST);

		mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE2_GPR_PWRUP_RST_CTL),
				PCIE_GPR_PWRUP_RST_CTL_P0_PERST |
				PCIE_GPR_PWRUP_RST_CTL_P1_PERST);

		mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE3_GPR_PWRUP_RST_CTL),
				PCIE_GPR_PWRUP_RST_CTL_P0_PERST |
				PCIE_GPR_PWRUP_RST_CTL_P1_PERST |
				PCIE_GPR_PWRUP_RST_CTL_P2_PERST |
				PCIE_GPR_PWRUP_RST_CTL_P3_PERST);

		mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE4_GPR_PWRUP_RST_CTL),
				PCIE_GPR_PWRUP_RST_CTL_P0_PERST |
				PCIE_GPR_PWRUP_RST_CTL_P1_PERST |
				PCIE_GPR_PWRUP_RST_CTL_P2_PERST |
				PCIE_GPR_PWRUP_RST_CTL_P3_PERST);

		/* Change PERST source to the register value */
		mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE0_GPR_PWRUP_RST_CTL),
				PCIE_GPR_PWRUP_RST_CTL_P0_PERST_EN |
				PCIE_GPR_PWRUP_RST_CTL_P1_PERST_EN);

		mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE1_GPR_PWRUP_RST_CTL),
				PCIE_GPR_PWRUP_RST_CTL_P0_PERST_EN |
				PCIE_GPR_PWRUP_RST_CTL_P1_PERST_EN);

		mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE2_GPR_PWRUP_RST_CTL),
				PCIE_GPR_PWRUP_RST_CTL_P0_PERST_EN |
				PCIE_GPR_PWRUP_RST_CTL_P1_PERST_EN);

		mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE3_GPR_PWRUP_RST_CTL),
				PCIE_GPR_PWRUP_RST_CTL_P0_PERST_EN |
				PCIE_GPR_PWRUP_RST_CTL_P1_PERST_EN |
				PCIE_GPR_PWRUP_RST_CTL_P2_PERST_EN |
				PCIE_GPR_PWRUP_RST_CTL_P3_PERST_EN);

		mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE4_GPR_PWRUP_RST_CTL),
				PCIE_GPR_PWRUP_RST_CTL_P0_PERST_EN |
				PCIE_GPR_PWRUP_RST_CTL_P1_PERST_EN |
				PCIE_GPR_PWRUP_RST_CTL_P2_PERST_EN |
				PCIE_GPR_PWRUP_RST_CTL_P3_PERST_EN);

		/* Deassert power-up reset */
		for (idx = 0; idx < ARRAY_SIZE(pcie_dbis); ++idx) {
			if (pcie_types[chip_idx][idx] == 0) {
				continue;
			}

			switch (pcie_dbis[idx]) {
			case PCIE0_P0_DBI_BASE:
			case PCIE0_P1_DBI_BASE:
				mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE0_GPR_PWRUP_RST_CTL),
						PCIE_GPR_PWRUP_RST_CTL_PWRUP_RST);
				break;
			case PCIE1_P0_DBI_BASE:
			case PCIE1_P1_DBI_BASE:
				mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE1_GPR_PWRUP_RST_CTL),
						PCIE_GPR_PWRUP_RST_CTL_PWRUP_RST);
				break;
			case PCIE2_P0_DBI_BASE:
			case PCIE2_P1_DBI_BASE:
				mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE2_GPR_PWRUP_RST_CTL),
						PCIE_GPR_PWRUP_RST_CTL_PWRUP_RST);
				break;
			case PCIE3_P0_DBI_BASE:
			case PCIE3_P1_DBI_BASE:
			case PCIE3_P2_DBI_BASE:
			case PCIE3_P3_DBI_BASE:
				mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE3_GPR_PWRUP_RST_CTL),
						PCIE_GPR_PWRUP_RST_CTL_PWRUP_RST);
				break;
			case PCIE4_P0_DBI_BASE:
			case PCIE4_P1_DBI_BASE:
			case PCIE4_P2_DBI_BASE:
			case PCIE4_P3_DBI_BASE:
				mmio_clrbits_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, PCIE4_GPR_PWRUP_RST_CTL),
						PCIE_GPR_PWRUP_RST_CTL_PWRUP_RST);
				break;
			}
		}
	}
}
