/*
 * Copyright (c) 2020-2022, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CRC_H
#define CRC_H

#include <stddef.h>
#include <stdint.h>

uint16_t crc16(const void *data, size_t size, uint16_t crc);
uint32_t crc32(const void *data, size_t size, uint32_t crc);

#endif /* CRC_H */
