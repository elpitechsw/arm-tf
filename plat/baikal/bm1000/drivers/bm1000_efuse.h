/*
 * Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BM1000_EFUSE_H
#define BM1000_EFUSE_H

#include <stdint.h>

int64_t efuse_get_lot(void);
int32_t efuse_get_mac(void);
int32_t efuse_get_serial(void);

#endif /* BM1000_EFUSE_H */
