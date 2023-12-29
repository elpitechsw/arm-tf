/*
 * Copyright (c) 2022-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DDR_BS1000_H
#define DDR_BS1000_H

#include <bs1000_def.h>
#include <bs1000_dimm_spd.h>

#include "ddr_bs1000_ctrl.h"

#define DDR_CTRL_BASE_OFF	(DDR1_BASE - DDR0_BASE)

#define DDR_PORT_NUM		(BS1000_DIMM_NUM / 2)
#define DDR_PORT_CHIP(port)	((port) / DDR_PORT_NUM)
#define DDR_PORT_IN_CHIP(port)	((port) % DDR_PORT_NUM)

#endif /* DDR_BS1000_H */
