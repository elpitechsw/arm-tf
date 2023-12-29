/*
 * Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl31/interrupt_mgmt.h>
#include <plat/common/platform.h>

#include <platform_def.h>

typedef struct baikal_intr_info_type_el3 {
	uint32_t id;
	interrupt_type_handler_t handler;
} baikal_intr_info_type_el3_t;

static baikal_intr_info_type_el3_t type_el3_interrupt_table[MAX_INTR_EL3];

int request_intr_type_el3(uint32_t id, interrupt_type_handler_t handler)
{
	static uint32_t index;
	uint32_t i;

	if (handler == NULL || index >= MAX_INTR_EL3) {
		return -EINVAL;
	}
	for (i = 0; i < index; i++) {
		if (id == type_el3_interrupt_table[i].id) {
			return -EALREADY;
		}
	}

	type_el3_interrupt_table[index].id = id;
	type_el3_interrupt_table[index].handler = handler;
	index++;

	return 0;
}

static uint64_t baikal_el3_interrupt_handler(uint32_t id, uint32_t flags,
					     void *handle, void *cookie)
{
	uint32_t intr_id;
	uint32_t i;
	interrupt_type_handler_t handler = NULL;

	intr_id = plat_ic_get_pending_interrupt_id();

	(void)plat_ic_acknowledge_interrupt();

	for (i = 0; i < MAX_INTR_EL3; i++) {
		if (intr_id == type_el3_interrupt_table[i].id) {
			handler = type_el3_interrupt_table[i].handler;
			break;
		}
	}
	if (handler != NULL) {
		handler(intr_id, flags, handle, cookie);
	}

	plat_ic_end_of_interrupt(intr_id);

	return (uint64_t)handle;
}

int baikal_el3_interrupt_config(void)
{
	uint64_t flags = 0U;

	set_interrupt_rm_flag(flags, NON_SECURE);
	set_interrupt_rm_flag(flags, SECURE);
	return register_interrupt_type_handler(INTR_TYPE_EL3,
					       baikal_el3_interrupt_handler, flags);
}
