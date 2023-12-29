/*
 * Copyright (c) 2022-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DDR_PHY_MAIN_H
#define DDR_PHY_MAIN_H

#include "../ddr_main.h"

int phy_main(int port, struct ddr_configuration *data);
int phyinit_G_ExecFW(int port);
void phyinit_C_PhyConfig(int port, struct ddr_configuration *data);
void phyinit_E_setDfiClk(int port);
void phyinit_calcMb(void *mb, struct ddr_configuration *data, int training_2d);
void phyinit_D_LoadIMEM(int port, struct ddr_configuration *data, int training_2d);
void phyinit_F_LoadDMEM(int port, struct ddr_configuration *data, const void *mb, int training_2d);
void phyinit_I_LoadPIE(int port, struct ddr_configuration *data);
int phyinit_H_readMsgBlock(int port, struct ddr_configuration *data);

#endif /* DDR_PHY_MAIN_H */
