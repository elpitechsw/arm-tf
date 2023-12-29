/*
 * Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DW_GPIO_H
#define DW_GPIO_H

void gpio_dir_clr(const uintptr_t base, const unsigned int pin);
void gpio_dir_set(const uintptr_t base, const unsigned int pin);
void gpio_out_rst(const uintptr_t base, const unsigned int pin);
void gpio_out_set(const uintptr_t base, const unsigned int pin);

#endif /* DW_GPIO_H */
