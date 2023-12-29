/*
 * Copyright (c) 2021-2022, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BS1000_CA75_H
#define BS1000_CA75_H

#include <stdint.h>

void ca75_core_enable(const u_register_t mpidr);
void ca75_core_warm_reset(const u_register_t mpidr);

#endif /* BS1000_CA75_H */
