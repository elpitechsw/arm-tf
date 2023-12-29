/*
 * Copyright (c) 2022, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BM1000_SCP_FLASH_H
#define BM1000_SCP_FLASH_H

#include <stddef.h>
#include <stdint.h>

int scp_flash_init(void);
int scp_flash_erase(uint32_t addr, size_t size);
int scp_flash_read(uint32_t addr, void *buf, size_t size);
int scp_flash_write(uint32_t addr, void *data, size_t size);

#endif /* BM1000_SCP_FLASH_H */
