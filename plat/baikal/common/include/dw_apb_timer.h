/*
 * Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DW_APB_TIMER_H
#define DW_APB_TIMER_H

typedef void (*dw_apb_timer_handler_t)(void *data);

int dw_apb_timer_register(uintptr_t base, unsigned long freq, uint32_t irq,
			  dw_apb_timer_handler_t handler, void *data);
int dw_apb_timer_start_periodic(uint32_t id, unsigned long rate);
int dw_apb_timer_stop(uint32_t id);

#endif /* DW_APB_TIMER_H */
