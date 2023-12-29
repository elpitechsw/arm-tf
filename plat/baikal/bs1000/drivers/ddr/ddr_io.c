/*
 * Copyright (c) 2022-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <lib/mmio.h>

#include <platform_def.h>

#include "ddr_bs1000.h"

void ddr_io_lcru_write(int port, uint32_t offs, int value)
{
	unsigned int chip_idx = DDR_PORT_CHIP(port),
		     chip_port = DDR_PORT_IN_CHIP(port);

	if (chip_port < 3) {
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR0_CMU0_BASE) +
			DDR_CTRL_BASE_OFF * chip_port + offs, value);
	} else {
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR3_CMU0_BASE) +
			DDR_CTRL_BASE_OFF * (chip_port % 3) + offs, value);
	}
}

void ddr_io_gpr_write(int port, uint32_t offs, int value)
{
	unsigned int chip_idx = DDR_PORT_CHIP(port),
		     chip_port = DDR_PORT_IN_CHIP(port);

	if (chip_port < 3) {
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR0_GPR_BASE) +
			DDR_CTRL_BASE_OFF * chip_port + offs, value);
	} else {
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR3_GPR_BASE) +
			DDR_CTRL_BASE_OFF * (chip_port % 3) + offs, value);
	}
}

void ddr_io_phy_write(int port, uint32_t offs, int value)
{
	unsigned int chip_idx = DDR_PORT_CHIP(port),
		     chip_port = DDR_PORT_IN_CHIP(port);

	if (chip_port < 3) {
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR0_PHY_BASE) +
			DDR_CTRL_BASE_OFF * chip_port + offs, value);
	} else {
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR3_PHY_BASE) +
			DDR_CTRL_BASE_OFF * (chip_port % 3) + offs, value);
	}
}

void ddr_io_ctrl_write(int port, uint32_t offs, int value)
{
	unsigned int chip_idx = DDR_PORT_CHIP(port),
		     chip_port = DDR_PORT_IN_CHIP(port);

	if (chip_port < 3) {
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR0_CTRL_BASE) +
			DDR_CTRL_BASE_OFF * chip_port + offs, value);
	} else {
		mmio_write_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR3_CTRL_BASE) +
			DDR_CTRL_BASE_OFF * (chip_port % 3) + offs, value);
	}
}

uint32_t ddr_io_lcru_read(int port, uint32_t offs)
{
	unsigned int chip_idx = DDR_PORT_CHIP(port),
		     chip_port = DDR_PORT_IN_CHIP(port);

	if (chip_port < 3) {
		return mmio_read_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR0_CMU0_BASE) +
			DDR_CTRL_BASE_OFF * chip_port + offs);
	} else {
		return mmio_read_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR3_CMU0_BASE) +
			DDR_CTRL_BASE_OFF * (chip_port % 3) + offs);
	}
}

uint32_t ddr_io_gpr_read(int port, uint32_t offs)
{
	unsigned int chip_idx = DDR_PORT_CHIP(port),
		     chip_port = DDR_PORT_IN_CHIP(port);

	if (chip_port < 3) {
		return mmio_read_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR0_GPR_BASE) +
			DDR_CTRL_BASE_OFF * chip_port + offs);
	} else {
		return mmio_read_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR3_GPR_BASE) +
			DDR_CTRL_BASE_OFF * (chip_port % 3) + offs);
	}
}

uint32_t ddr_io_phy_read(int port, uint32_t offs)
{
	unsigned int chip_idx = DDR_PORT_CHIP(port),
		     chip_port = DDR_PORT_IN_CHIP(port);

	if (chip_port < 3) {
		return mmio_read_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR0_PHY_BASE) +
			DDR_CTRL_BASE_OFF * chip_port + offs);
	} else {
		return mmio_read_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR3_PHY_BASE) +
			DDR_CTRL_BASE_OFF * (chip_port % 3) + offs);
	}
}

uint32_t ddr_io_ctrl_read(int port, uint32_t offs)
{
	unsigned int chip_idx = DDR_PORT_CHIP(port),
		     chip_port = DDR_PORT_IN_CHIP(port);

	if (chip_port < 3) {
		return mmio_read_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR0_CTRL_BASE) +
			DDR_CTRL_BASE_OFF * chip_port + offs);
	} else {
		return mmio_read_32(PLATFORM_ADDR_OUT_CHIP(chip_idx, DDR3_CTRL_BASE) +
			DDR_CTRL_BASE_OFF * (chip_port % 3) + offs);
	}
}
