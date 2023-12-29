/*
 * Copyright (c) 2018-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/pmf/pmf.h>
#include <libfdt.h>

#include <baikal_pvt.h>
#include <baikal_scp.h>
#include <baikal_sip_svc.h>
#include <bm1000_cmu.h>
#include <bm1000_def.h>
#include <bm1000_efuse.h>
#include <bm1000_smmu.h>
#include <platform_def.h>

#define BAIKAL_SMC_CMU_PLL_SET_RATE	0
#define BAIKAL_SMC_CMU_PLL_GET_RATE	1
#define BAIKAL_SMC_CMU_PLL_ENABLE	2
#define BAIKAL_SMC_CMU_PLL_DISABLE	3
#define BAIKAL_SMC_CMU_PLL_ROUND_RATE	4
#define BAIKAL_SMC_CMU_PLL_IS_ENABLED	5
#define BAIKAL_SMC_CMU_CLKCH_SET_RATE	6
#define BAIKAL_SMC_CMU_CLKCH_GET_RATE	7
#define BAIKAL_SMC_CMU_CLKCH_ENABLE	8
#define BAIKAL_SMC_CMU_CLKCH_DISABLE	9
#define BAIKAL_SMC_CMU_CLKCH_ROUND_RATE	10
#define BAIKAL_SMC_CMU_CLKCH_IS_ENABLED	11
#define BAIKAL_SMC_CMU_CLKCH_MSHC_SPEED	12

static int baikal_get_cmu_descriptors(void *fdt)
{
	unsigned int cmuidx = 0;
	int offset = 0;
	uint64_t fpllreq;

	while (1) {
		offset = fdt_next_node(fdt, offset, NULL);
		if (offset < 0) {
			break;
		}

		if (!fdt_node_check_compatible(fdt, offset, "baikal,bm1000-cmu")) {
			int clock_offset;
			struct cmu_desc *const cmu = cmu_desc_get_by_idx(cmuidx);
			const int cmu_offset = offset;
			int num_of_chans;
			const fdt32_t *prop;
			int proplen;

			if (cmu == NULL) {
				ERROR("%s: unable to get cmu_desc with idx = %u\n",
				      __func__, cmuidx);
				return 2;
			}

			/* clear */
			cmu->base	     = 0;
			cmu->frefclk	     = 0;
			cmu->deny_pll_reconf = false;

			prop = fdt_getprop(fdt, cmu_offset, "reg", &proplen);
			if (prop == NULL || proplen != 16) {
				ERROR("%s: \"reg\" is undefined or invalid\n", __func__);
				continue;
			}

			cmu->base = fdt32_to_cpu(prop[1]);

			prop = fdt_getprop(fdt, cmu_offset, "clocks", NULL);
			if (prop == NULL) {
				ERROR("%s: cmu@%lx: \"clocks\" is undefined or invalid\n",
					__func__, cmu->base);
				cmu->base = 0;
				continue;
			}

			clock_offset = fdt_node_offset_by_phandle(fdt, fdt32_to_cpu(*prop));
			prop = fdt_getprop(fdt, clock_offset, "clock-frequency", &proplen);
			if (prop == NULL || proplen != 4) {
				ERROR("%s: cmu@%lx: \"clocks\" -> \"clock-frequency\" is undefined or invalid\n",
					__func__, cmu->base);
				cmu->base = 0;
				continue;
			}

			cmu->frefclk = fdt32_to_cpu(*prop);

			prop = fdt_getprop(fdt, cmu_offset, "clock-frequency", NULL);
			if (prop == NULL) {
				ERROR("%s: cmu@%lx: \"clock-frequency\" is undefined or invalid\n",
					__func__, cmu->base);
				cmu->base = 0;
				continue;
			}

			fpllreq = fdt32_to_cpu(*prop);

			prop = fdt_getprop(fdt, cmu_offset, "clock-indices", &proplen);
			if (prop != NULL) {
				num_of_chans = proplen / 4;
			} else {
				num_of_chans = 0;
			}

			if (num_of_chans > 1) {
				cmu->deny_pll_reconf = true;
			}


			if (cmu->base != MMAVLSP_CMU1_BASE && cmu->base != MMXGBE_CMU1_BASE) {
				cmu_pll_set_rate(cmu->base, cmu->frefclk, fpllreq);
			}

			INFO("CMU @ base=0x%08lx clk=%lu freq=%lu reconf=%d chans=%d\n",
			     cmu->base,
			     cmu->frefclk,
			     fpllreq,
			     cmu->deny_pll_reconf,
			     num_of_chans);

			++cmuidx;
		}
	}

	return 0;
}

static int baikal_sip_setup(void)
{
	int ret;
	void *fdt = (void *)(uintptr_t)BAIKAL_SEC_DTB_BASE;

#if ENABLE_PMF
	if (pmf_setup() != 0) {
		return 1;
	}
#endif
	ret = fdt_open_into(fdt, fdt, BAIKAL_DTB_MAX_SIZE);
	if (ret < 0) {
		ERROR("Invalid Device Tree at %p: error %d\n", fdt, ret);
		return -1;
	}

	return baikal_get_cmu_descriptors(fdt);
}

static int pvt_check_addr(const uintptr_t base)
{
	unsigned int i;
	static const uintptr_t pvt_bases[] = {
		MMCA57_0_PVT_BASE,
		MMCA57_1_PVT_BASE,
		MMCA57_2_PVT_BASE,
		MMCA57_3_PVT_BASE,
		MMMALI_PVT_BASE
	};

	for (i = 0; i < ARRAY_SIZE(pvt_bases); ++i) {
		if (pvt_bases[i] == base) {
			return 0;
		}
	}

	return 1;
}

static uintptr_t sip_smc_handler(uint32_t smc_fid,
				 u_register_t x1,
				 u_register_t x2,
				 u_register_t x3,
				 u_register_t x4,
				 void *cookie,
				 void *handle,
				 u_register_t flags)
{
	uint32_t local_smc_fid = smc_fid;
	uint64_t ret;
	uint64_t data[4];

	if (is_caller_secure(flags)) {
		ERROR("%s: SMC secure world's call (0x%x)\n", __func__, smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}

	if (GET_SMC_CC(smc_fid) == SMC_32) {
		/* 32-bit function, clear top parameter bits */
		x1 = (uint32_t)x1;
		x2 = (uint32_t)x2;
		x3 = (uint32_t)x3;
		x4 = (uint32_t)x4;

		/*
		 * Convert SMC FID to SMC64 to support SMC32/SMC64.
		 * SMC64 calls are expected to be the 64-bit equivalent
		 * to the 32-bit call, where applicable (ARM DEN 0028E).
		 */
		local_smc_fid |= SMC_64 << FUNCID_CC_SHIFT;
	}

	switch (local_smc_fid) {
	case BAIKAL_SMC_FLASH_ERASE:
	case BAIKAL_SMC_FLASH_INIT:
	case BAIKAL_SMC_FLASH_POSITION:
	case BAIKAL_SMC_FLASH_PULL:
	case BAIKAL_SMC_FLASH_PUSH:
	case BAIKAL_SMC_FLASH_READ:
	case BAIKAL_SMC_FLASH_WRITE:
		ret = baikal_smc_flash_handler(smc_fid, x1, x2, x3, x4, data);
		if (ret) {
			break;
		} else if (smc_fid == BAIKAL_SMC_FLASH_PULL) {
			SMC_RET4(handle, data[0], data[1], data[2], data[3]);
		} else if (smc_fid == BAIKAL_SMC_FLASH_INIT) {
			SMC_RET3(handle, ret, data[0], data[1]);
		}
		break;
	case BAIKAL_SMC_PVT_CMD:
		ret = pvt_check_addr(x2);
		if (ret) {
			break;
		}
		if (x1 == PVT_READ) {
			ret = pvt_read_reg(x2, x3);
		} else if (x1 == PVT_WRITE) {
			ret = pvt_write_reg(x2, x3, x4);
		} else {
			ERROR("%s: unhandled PVT SMC, x1:0x%lx\n", __func__, x1);
			ret = SMC_UNK;
		}
		break;
	case BAIKAL_SMC_CMU_CMD:
		switch (x2) {
		case BAIKAL_SMC_CMU_PLL_SET_RATE:
			ret = cmu_pll_set_rate(x1, x4, x3);
			break;
		case BAIKAL_SMC_CMU_PLL_GET_RATE:
			ret = cmu_pll_get_rate(x1, x4);
			break;
		case BAIKAL_SMC_CMU_PLL_ENABLE:
			ret = cmu_pll_enable(x1);
			break;
		case BAIKAL_SMC_CMU_PLL_DISABLE:
			ret = cmu_pll_disable(x1);
			break;
		case BAIKAL_SMC_CMU_PLL_ROUND_RATE:
			ret = cmu_pll_round_rate(x1, x4, x3);
			break;
		case BAIKAL_SMC_CMU_PLL_IS_ENABLED:
			ret = cmu_pll_is_enabled(x1);
			break;
		case BAIKAL_SMC_CMU_CLKCH_SET_RATE:
			ret = cmu_clkch_set_rate(x4, x1, x3);
			break;
		case BAIKAL_SMC_CMU_CLKCH_GET_RATE:
			ret = cmu_clkch_get_rate(x4, x1);
			break;
		case BAIKAL_SMC_CMU_CLKCH_ENABLE:
			ret = cmu_clkch_enable(x4, x1);
			break;
		case BAIKAL_SMC_CMU_CLKCH_DISABLE:
			ret = cmu_clkch_disable(x4, x1);
			break;
		case BAIKAL_SMC_CMU_CLKCH_ROUND_RATE:
			ret = cmu_clkch_round_rate(x4, x1, x3);
			break;
		case BAIKAL_SMC_CMU_CLKCH_IS_ENABLED:
			ret = cmu_clkch_is_enabled(x4, x1);
			break;
		default:
			ERROR("%s: unhandled CMU SMC, x2:0x%lx\n", __func__, x2);
			ret = SMC_UNK;
			break;
		}
		break;
	case BAIKAL_SMC_VDEC_SMMU_SET_CACHE:
		ret = mmvdec_smmu_set_domain_cache(x1, x2, x3);
		break;
	case BAIKAL_SMC_VDEC_SMMU_GET_CACHE:
		ret = mmvdec_smmu_get_domain_cache();
		break;
	case BAIKAL_SMC_VDU_UPDATE:
		ret = scp_cmd('V', x1, x2);
		break;
	case BAIKAL_SMC_SCP_LOG_DISABLE:
		ret = scp_cmd('T', 0, 0);
		break;
	case BAIKAL_SMC_SCP_LOG_ENABLE:
		ret = scp_cmd('t', 0, 0);
		break;
	case BAIKAL_SMC_EFUSE_GET_LOT:
		ret = efuse_get_lot();
		break;
	case BAIKAL_SMC_EFUSE_GET_SERIAL:
		ret = efuse_get_serial();
		break;
	case BAIKAL_SMC_EFUSE_GET_MAC:
		ret = efuse_get_mac();
		break;
	case BAIKAL_SMC_FLASH_LOCK:
		ret = scp_cmd('L', x1, 0);
		break;
	default:
#if ENABLE_PMF
		/* Dispatch PMF calls to PMF SMC handler and return its return value */
		if (is_pmf_fid(smc_fid)) {
			return pmf_smc_handler(smc_fid, x1, x2, x3, x4, cookie,	handle, flags);
		}
#endif
		ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
		ret = SMC_UNK;
		break;
	}

	SMC_RET1(handle, ret);
}

DECLARE_RT_SVC(
	baikal_sip_svc,
	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	baikal_sip_setup,
	sip_smc_handler
);
