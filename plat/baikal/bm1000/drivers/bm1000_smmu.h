/*
 * Copyright (c) 2020-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * Author: Pavel Parkhomenko <pavel.parkhomenko@baikalelectronics.ru>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BM1000_SMMU_H
#define BM1000_SMMU_H

#include <stdbool.h>

#define SMMU_SACR			0x10
#define SMMU_SACR_NORMALIZE		(1 << 27)
#define SMMU_STLBIALL			0x60
#define SMMU_STLBGSYNC			0x70
#define SMMU_STLBGSTATUS		0x74
#define SMMU_STLBGSTATUS_GSACTIVE	(1 << 0)

int mmvdec_smmu_set_normalize(bool normalize);
int mmvdec_smmu_set_domain_cache(int awdomain, int ardomain, int axcache);
int mmvdec_smmu_get_domain_cache(void);
int mmvdec_smmu_set_qos(int awqos, int arqos);

#endif /* BM1000_SMMU_H */
