/*
 * Copyright (c) 2020-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/pmf/pmf.h>

#include <baikal_pvt.h>
#include <baikal_sip_svc.h>
#include <bs1000_cmu.h>
#include <bs1000_def.h>
#include <bs1000_scp_lcru.h>

static int baikal_sip_setup(void)
{
#if ENABLE_PMF
	if (pmf_setup() != 0) {
		return 1;
	}
#endif
	return cmu_desc_init();
}

static int pvt_check_addr(const uintptr_t base)
{
	unsigned int i;
	static const uintptr_t pvt_bases[] = {
		CA75_0_PVT_BASE,
		CA75_1_PVT_BASE,
		CA75_2_PVT_BASE,
		CA75_3_PVT_BASE,
		CA75_4_PVT_BASE,
		CA75_5_PVT_BASE,
		CA75_6_PVT_BASE,
		CA75_7_PVT_BASE,
		CA75_8_PVT_BASE,
		CA75_9_PVT_BASE,
		CA75_10_PVT_BASE,
		CA75_11_PVT_BASE,
		DDR0_PVT_BASE,
		DDR1_PVT_BASE,
		DDR2_PVT_BASE,
		DDR3_PVT_BASE,
		DDR4_PVT_BASE,
		DDR5_PVT_BASE,
		PCIE0_PVT_BASE,
		PCIE1_PVT_BASE,
		PCIE2_PVT_BASE,
		PCIE3_PVT_BASE,
		PCIE4_PVT_BASE
	};

	for (i = 0; i < ARRAY_SIZE(pvt_bases); ++i) {
		if (pvt_bases[i] == PLATFORM_ADDR_IN_CHIP(base)) {
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
	case BAIKAL_SMC_CLK_ROUND:
	case BAIKAL_SMC_CLK_SET:
	case BAIKAL_SMC_CLK_GET:
	case BAIKAL_SMC_CLK_ENABLE:
	case BAIKAL_SMC_CLK_DISABLE:
	case BAIKAL_SMC_CLK_IS_ENABLED:
		ret = baikal_smc_clk_handler(smc_fid, x1, x2, x3, x4);
		break;
	case BAIKAL_SMC_GMAC_DIV2_ENABLE:
	case BAIKAL_SMC_GMAC_DIV2_DISABLE:
		ret = baikal_smc_gmac_handler(smc_fid, x1, x2, x3, x4);
		break;
	case BAIKAL_SMC_LSP_MUX:
		ret = scp_lcru_clrsetbits(PLATFORM_ADDR_OUT_CHIP(x1, SCP_GPR_LSP_CTL),
					  SCP_GPR_LSP_CTL_SEL_PERIPH_MASK,
					  x2 << SCP_GPR_LSP_CTL_SEL_PERIPH_SHIFT);
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
