/*
 * Copyright (c) 2020-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BAIKAL_SIP_SVC_H
#define BAIKAL_SIP_SVC_H

#include <stdint.h>

/* TODO: TF-A uses [0xc2000000:0xc200001f] range for PMF calls */
#define BAIKAL_SMC_CMU_CMD		0xc2000000
#define BAIKAL_SMC_PVT_CMD		0xc2000001
#define BAIKAL_SMC_FLASH_WRITE		0xc2000002
#define BAIKAL_SMC_FLASH_READ		0xc2000003
#define BAIKAL_SMC_FLASH_ERASE		0xc2000004
#define BAIKAL_SMC_FLASH_PUSH		0xc2000005
#define BAIKAL_SMC_FLASH_PULL		0xc2000006
#define BAIKAL_SMC_FLASH_POSITION	0xc2000007
#define BAIKAL_SMC_FLASH_INIT		0xc2000008
#define BAIKAL_SMC_FLASH_LOCK		0xc2000009
#define BAIKAL_SMC_VDU_UPDATE		0xc2000100
#define BAIKAL_SMC_SCP_LOG_DISABLE	0xc2000200
#define BAIKAL_SMC_SCP_LOG_ENABLE	0xc2000201
#define BAIKAL_SMC_EFUSE_GET_LOT	0xc2000202
#define BAIKAL_SMC_EFUSE_GET_SERIAL	0xc2000203
#define BAIKAL_SMC_EFUSE_GET_MAC	0xc2000204
#define BAIKAL_SMC_VDEC_SMMU_SET_CACHE	0xc2000300
#define BAIKAL_SMC_VDEC_SMMU_GET_CACHE	0xc2000301
#define BAIKAL_SMC_CLK_ROUND		0xc2000400
#define BAIKAL_SMC_CLK_SET		0xc2000401
#define BAIKAL_SMC_CLK_GET		0xc2000402
#define BAIKAL_SMC_CLK_ENABLE		0xc2000403
#define BAIKAL_SMC_CLK_DISABLE		0xc2000404
#define BAIKAL_SMC_CLK_IS_ENABLED	0xc2000405
#define BAIKAL_SMC_GMAC_DIV2_ENABLE	0xc2000500
#define BAIKAL_SMC_GMAC_DIV2_DISABLE	0xc2000501
#define BAIKAL_SMC_LSP_MUX		0xc2000600
#define BAIKAL_SMC_RST_SET		0xc2000700
#define BAIKAL_SMC_RST_CLR		0xc2000701

int64_t baikal_smc_flash_handler(const uint32_t smc,
				 const uint64_t x1,
				 const uint64_t x2,
				 const uint64_t x3,
				 const uint64_t x4,
				 uint64_t *data);

int64_t baikal_smc_clk_handler(const uint32_t smc,
			       const uint64_t x1,
			       const uint64_t x2,
			       const uint64_t x3,
			       const uint64_t x4);

int64_t baikal_smc_gmac_handler(const uint32_t smc,
				const uint64_t x1,
				const uint64_t x2,
				const uint64_t x3,
				const uint64_t x4);

int64_t baikal_smc_rst_handler(uint32_t smc,
			       uint64_t x1,
			       uint64_t x2);

#endif /* BAIKAL_SIP_SVC_H */
