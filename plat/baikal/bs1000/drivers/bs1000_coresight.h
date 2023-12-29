/*
 * Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BS1000_CORESIGHT_H
#define BS1000_CORESIGHT_H

void bs1000_coresight_init(void);

#ifdef MULTICHIP_SYSTEM_COUNTERS_RESYNC
void bs1000_resync_system_counters(void);
#endif

#endif /* BS1000_CORESIGHT_H */
