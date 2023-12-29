/*
 * Copyright (c) 2022-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef DDR_IO_H
#define DDR_IO_H

#include <stdint.h>

void ddr_io_lcru_write(int port, uint32_t offs, int value);
void ddr_io_gpr_write(int port, uint32_t offs, int value);
void ddr_io_phy_write(int port, uint32_t offs, int value);
void ddr_io_ctrl_write(int port, uint32_t offs, int value);

uint32_t ddr_io_lcru_read(int port, uint32_t offs);
uint32_t ddr_io_gpr_read(int port, uint32_t offs);
uint32_t ddr_io_phy_read(int port, uint32_t offs);
uint32_t ddr_io_ctrl_read(int port, uint32_t offs);

#endif /* DDR_IO_H */
