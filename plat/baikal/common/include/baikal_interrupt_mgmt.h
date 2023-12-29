/*
 * Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BAIKAL_INTERRUPT_MGMT_H
#define BAIKAL_INTERRUPT_MGMT_H

#include <bl31/interrupt_mgmt.h>

int request_intr_type_el3(uint32_t id, interrupt_type_handler_t handler);

void baikal_el3_interrupt_config(void);

#endif /* BAIKAL_INTERRUPT_MGMT_H */
