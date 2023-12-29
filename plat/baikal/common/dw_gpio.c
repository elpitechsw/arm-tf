/*
 * Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <lib/mmio.h>

#include <dw_gpio.h>

#define SWPORTA_DR	0x00
#define SWPORTA_DDR	0x04

void gpio_dir_clr(const uintptr_t base, const unsigned int pin)
{
	assert(pin < 32);

	mmio_clrbits_32(base + SWPORTA_DDR, 1 << pin);
}

void gpio_dir_set(const uintptr_t base, const unsigned int pin)
{
	assert(pin < 32);

	mmio_setbits_32(base + SWPORTA_DDR, 1 << pin);
}

void gpio_out_rst(const uintptr_t base, const unsigned int pin)
{
	assert(pin < 32);

	mmio_clrbits_32(base + SWPORTA_DR, 1 << pin);
}

void gpio_out_set(const uintptr_t base, const unsigned int pin)
{
	assert(pin < 32);

	mmio_setbits_32(base + SWPORTA_DR, 1 << pin);
}
