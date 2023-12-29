/*
 * Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <errno.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <baikal_interrupt_mgmt.h>
#include <platform_def.h>
#include <dw_apb_timer.h>

typedef struct dw_apb_timer {
	uintptr_t	base;
	uint32_t	freq;
	uint32_t	irq;
	dw_apb_timer_handler_t handler;
	void *data;
} dw_apb_timer_t;

static dw_apb_timer_t timers[MAX_DW_APB_TIMERS];
static uint32_t n_timers;

#define TIMER_N_LOAD_COUNT		0x00
#define TIMER_N_CURRENT_VALUE		0x04
#define TIMER_N_CONTROL			0x08
#define TIMER_CONTROL_ENABLE		BIT(0)
#define TIMER_CONTROL_MODE_PERIODIC	BIT(1)
#define TIMER_CONTROL_INT		BIT(2)
#define TIMER_N_EOI			0x0c

static uint64_t dw_apb_timer_handler(uint32_t id, uint32_t flags,
				     void *handle, void *cookie)
{
	uint32_t i;

	for (i = 0; i < n_timers; ++i) {
		if (id == timers[i].irq) {
			/* deassert interrupt */
			mmio_read_32(timers[i].base + TIMER_N_EOI);
			/* call timer handler */
			timers[i].handler(timers[i].data);
			break;
		}
	}
	return 0;
}

int dw_apb_timer_register(uintptr_t base, unsigned long freq, uint32_t irq,
			  dw_apb_timer_handler_t handler, void *data)
{
	uint32_t i;
	int ret;

	if (handler == NULL || n_timers >= MAX_DW_APB_TIMERS) {
		return -EINVAL;
	}
	for (i = 0; i < n_timers; ++i) {
		if (base == timers[i].base || irq == timers[i].irq) {
			return -EALREADY;
		}
	}

	ret = request_intr_type_el3(irq, dw_apb_timer_handler);
	if (ret < 0) {
		return ret;
	}
	timers[n_timers].base = base;
	timers[n_timers].freq = freq;
	timers[n_timers].irq = irq;
	timers[n_timers].handler = handler;
	timers[n_timers].data = data;

	return n_timers++;
}

int dw_apb_timer_start_periodic(uint32_t id, unsigned long rate)
{
	if (id >= n_timers) {
		return -EINVAL;
	}

	mmio_clrbits_32(timers[id].base + TIMER_N_CONTROL, TIMER_CONTROL_ENABLE);
	mmio_read_32(timers[id].base + TIMER_N_EOI);
	mmio_clrsetbits_32(timers[id].base + TIMER_N_CONTROL,
			   TIMER_CONTROL_INT,
			   TIMER_CONTROL_MODE_PERIODIC);
	mmio_write_32(timers[id].base + TIMER_N_LOAD_COUNT, timers[id].freq / rate);
	mmio_setbits_32(timers[id].base + TIMER_N_CONTROL, TIMER_CONTROL_ENABLE);

	return 0;
}

int dw_apb_timer_stop(uint32_t id)
{
	if (id >= n_timers) {
		return -EINVAL;
	}

	mmio_clrbits_32(timers[id].base + TIMER_N_CONTROL, TIMER_CONTROL_ENABLE);

	return 0;
}
