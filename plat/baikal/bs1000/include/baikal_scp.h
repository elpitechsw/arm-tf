/*
 * Copyright (c) 2020-2021, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BAIKAL_SCP_H
#define BAIKAL_SCP_H

#include <stdint.h>

int scp_cmd(unsigned int chip, uint8_t op, uint32_t arg0, uint32_t arg1);
void *scp_buf(unsigned int chip);

#endif /* BAIKAL_SCP_H */
