/*
 * Copyright (c) 2022, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BAIKAL_BOOTFLASH_H
#define BAIKAL_BOOTFLASH_H

#include <stddef.h>
#include <stdint.h>

int bootflash_init(void);
int bootflash_erase(uint32_t addr, size_t size);
int bootflash_read(uint32_t addr, void *buf, size_t size);
int bootflash_write(uint32_t addr, void *data, size_t size);

#endif /* BAIKAL_BOOTFLASH_H */
