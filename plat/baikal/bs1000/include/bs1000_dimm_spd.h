/*
 * Copyright (c) 2022-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BS1000_DIMM_SPD_H
#define BS1000_DIMM_SPD_H

#define BS1000_DIMM_NUM		12 /* number of per-chip DDR slots */

const void *baikal_dimm_spd_get(const unsigned int chip_idx,
				const unsigned int dimm_idx);

#endif /* BS1000_DIMM_SPD_H */
