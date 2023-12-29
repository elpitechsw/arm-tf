/*
 * Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DDR_CTRL_H
#define DDR_CTRL_H

#include "ddr_bs1000.h"
#include "ddr_main.h"
#include "ddr_io.h"

#define DDR_CTRL_QOS_LPR	29
#define MR5_DM                  0

#define BS_DDRC_WRITE(...)	ddr_io_ctrl_write(__VA_ARGS__)
#define BS_DDRC_READ(...)	ddr_io_ctrl_read(__VA_ARGS__)

void umctl2_exit_SR(int port);
void umctl2_enter_SR(int port);
int ctrl_prepare_phy_init(int port);
int ctrl_init(int port, struct ddr_configuration *data);
void ctrl_complete_phy_init(int port, struct ddr_configuration *data);

#endif /* DDR_CTRL_H */
