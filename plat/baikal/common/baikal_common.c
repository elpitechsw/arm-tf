/*
 * Copyright (c) 2021-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/smccc.h>
#include <plat/common/platform.h>
#include <services/arm_arch_svc.h>

#include <baikal_def.h>

#define JEDEC_BAIKAL_BKID	U(0x8)
#define JEDEC_BAIKAL_MFID	U(0x31)

int32_t plat_get_soc_revision(void)
{
	return 0;
}

int32_t plat_get_soc_version(void)
{
	uint32_t manfid = SOC_ID_SET_JEP_106(JEDEC_BAIKAL_BKID, JEDEC_BAIKAL_MFID);

	return (int32_t)(manfid);
}

unsigned int plat_get_syscnt_freq2(void)
{
	return SYS_COUNTER_FREQ_IN_TICKS;
}

int32_t plat_is_smccc_feature_available(u_register_t fid)
{
	switch (fid) {
	case SMCCC_ARCH_SOC_ID:
		return SMC_ARCH_CALL_SUCCESS;
	default:
		return SMC_ARCH_CALL_NOT_SUPPORTED;
	}
}
