/*
 * Copyright (c) 2017-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMMU_V3_H
#define SMMU_V3_H

#include <stdint.h>
#include <lib/utils_def.h>
#include <platform_def.h>

/* SMMUv3 register offsets from device base */
#define SMMU_IDR0		U(0x0000)
#define SMMU_IDR1		U(0x0004)
#define SMMU_CR0		U(0x0020)
#define SMMU_CR0ACK		U(0x0024)
#define SMMU_GBPA		U(0x0044)
#define SMMU_S_IDR0		U(0x8000)
#define SMMU_S_IDR1		U(0x8004)
#define SMMU_S_CR0		U(0x8020)
#define SMMU_S_CR0ACK		U(0x8024)
#define SMMU_S_CR1		U(0x8028)
#define SMMU_S_INIT		U(0x803c)
#define SMMU_S_GBPA		U(0x8044)
#define SMMU_S_STRTAB_BASE	U(0x8080)
#define SMMU_S_STRTAB_BASE_CFG	U(0x8088)
#define SMMU_S_CMDQ_BASE	U(0x8090)
#define SMMU_S_CMDQ_PROD	U(0x8098)
#define SMMU_S_CMDQ_CONS	U(0x809c)
#define SMMU_S_EVENTQ_BASE	U(0x80a0)
#define SMMU_S_EVENTQ_PROD	U(0x80a8)
#define SMMU_S_EVENTQ_CONS	U(0x80ac)

/*
 * TODO: SMMU_ROOT_PAGE_OFFSET is platform specific.
 * Currently defined as a command line model parameter.
 */
#if ENABLE_RME

#define SMMU_ROOT_PAGE_OFFSET	(PLAT_ARM_SMMUV3_ROOT_REG_OFFSET)
#define SMMU_ROOT_IDR0		U(SMMU_ROOT_PAGE_OFFSET + 0x0000)
#define SMMU_ROOT_IIDR		U(SMMU_ROOT_PAGE_OFFSET + 0x0008)
#define SMMU_ROOT_CR0		U(SMMU_ROOT_PAGE_OFFSET + 0x0020)
#define SMMU_ROOT_CR0ACK	U(SMMU_ROOT_PAGE_OFFSET + 0x0024)
#define SMMU_ROOT_GPT_BASE	U(SMMU_ROOT_PAGE_OFFSET + 0x0028)
#define SMMU_ROOT_GPT_BASE_CFG	U(SMMU_ROOT_PAGE_OFFSET + 0x0030)
#define SMMU_ROOT_GPF_FAR	U(SMMU_ROOT_PAGE_OFFSET + 0x0038)
#define SMMU_ROOT_GPT_CFG_FAR	U(SMMU_ROOT_PAGE_OFFSET + 0x0040)
#define SMMU_ROOT_TLBI		U(SMMU_ROOT_PAGE_OFFSET + 0x0050)
#define SMMU_ROOT_TLBI_CTRL	U(SMMU_ROOT_PAGE_OFFSET + 0x0058)

#endif /* ENABLE_RME */

/* SMMU_CR0 and SMMU_CR0ACK register fields */
#define SMMU_CR0_SMMUEN			(1UL << 0)
#define SMMU_CR0_EVENTQEN		(1UL << 2)
#define SMMU_CR0_CMDQEN			(1UL << 3)

/* SMMU_GBPA register fields */
#define SMMU_GBPA_UPDATE		(1UL << 31)
#define SMMU_GBPA_ABORT			(1UL << 20)

/* SMMU_S_IDR1 register fields */
#define SMMU_S_IDR1_SECURE_IMPL		(1UL << 31)

/* SMMU_S_INIT register fields */
#define SMMU_S_INIT_INV_ALL		(1UL << 0)

/* SMMU_S_GBPA register fields */
#define SMMU_S_GBPA_UPDATE		(1UL << 31)
#define SMMU_S_GBPA_ABORT		(1UL << 20)

/* SMMU_ROOT_IDR0 register fields */
#define SMMU_ROOT_IDR0_ROOT_IMPL	(1UL << 0)

/* SMMU_ROOT_CR0 register fields */
#define SMMU_ROOT_CR0_GPCEN		(1UL << 1)
#define SMMU_ROOT_CR0_ACCESSEN		(1UL << 0)

/* STE definitions */
#define SMMU_STE_SIZE			64
#define SMMU_STE_0_V			(1UL << 0)
#define SMMU_STE_0_CONFIG_MASK		GENMASK(3, 1)
#define SMMU_STE_0_CONFIG_SHIFT		1
#define SMMU_STE_3_MEMATTR_MASK		GENMASK(3, 0)
#define SMMU_STE_3_MEMATTR_SHIFT	0
#define SMMU_STE_3_MTCFG		(1UL << 4)
#define SMMU_STE_3_ALLOCCFG_MASK	GENMASK(8, 5)
#define SMMU_STE_3_ALLOCCFG_SHIFT	5
#define SMMU_STE_3_SHCFG_MASK		GENMASK(13, 12)
#define SMMU_STE_3_SHCFG_SHIFT		12
#define SMMU_STE_3_NSCFG_MASK		GENMASK(15, 14)
#define SMMU_STE_3_NSCFG_SHIFT		14
#define SMMU_STE_3_PRIVCFG_MASK		GENMASK(17, 16)
#define SMMU_STE_3_PRIVCFG_SHIFT	16
#define SMMU_STE_3_INSTCFG_MASK		GENMASK(19, 18)
#define SMMU_STE_3_INSTCFG_SHIFT	18

#define SMMU_STE_CONFIG_BYPASS_BYPASS	4
#define SMMU_STE_MEMATTR_IWB_OWB	0xf
#define SMMU_STE_MTCFG_USE_INCOMING	0
#define SMMU_STE_MTCFG_REPLACE_MEMATTR	1
#define SMMU_STE_ALLOCCFG_USE_INCOMING	0
#define SMMU_STE_ALLOCCFG_RW		0xe
#define SMMU_STE_SHCFG_USE_INCOMING	1
#define SMMU_STE_SHCFG_OUTER		2
#define SMMU_STE_NSCFG_USE_INCOMING	0
#define SMMU_STE_NSCFG_NSEC		3
#define SMMU_STE_PRIVCFG_USE_INCOMING	0
#define SMMU_STE_INSTCFG_USE_INCOMING	0

int smmuv3_init(uintptr_t smmu_base);
int smmuv3_security_init(uintptr_t smmu_base);

int smmuv3_ns_set_abort_all(uintptr_t smmu_base);

int smmuv3_enable_queues(uintptr_t smmu_base, bool sec);

int smmuv3_invalidate_all(uintptr_t smmu_base);

int smmuv3_enable(uintptr_t smmu_base, bool sec);

#endif /* SMMU_V3_H */
