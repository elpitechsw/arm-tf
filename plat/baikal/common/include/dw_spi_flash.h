/*
 * Copyright (c) 2018-2022, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DW_SPI_FLASH_H
#define DW_SPI_FLASH_H

#include <stddef.h>
#include <stdint.h>

int spi_flash_init(int line);
int spi_flash_read(int line, uint32_t adr, void *data, size_t size);
int spi_flash_erase(int line, uint32_t adr, size_t size);
int spi_flash_write(int line, uint32_t adr, void *data, size_t size);

#endif /* DW_SPI_FLASH_H */
