/*
 * Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DDR_PHY_MISC_H
#define DDR_PHY_MISC_H

#define PHY_EQL_DFE     1
#define PHY_EQL_FFE     2

#define PMU_MSG_EOINIT        0x00  /* End of initialization */
#define PMU_MSG_EOTR_WLFINE   0x01  /* End of fine write leveling */
#define PMU_MSG_EOTR_RDEN     0x02  /* End of read enable training */
#define PMU_MSG_EOTR_RDDL     0x03  /* End of read delay center optimization */
#define PMU_MSG_EOTR_WRDL     0x04  /* End of write delay center optimization */
#define PMU_MSG_EOTR_RDDL_2D  0x05  /* End of 2D read delay/voltage center optimization */
#define PMU_MSG_EOTR_WRDL_2D  0x06  /* End of 2D write delay /voltage center optimization */
#define PMU_MSG_EOFRW_PASS    0x07  /* Training has run successfully (firmware complete) */
#define PMU_MSG_SREAM         0x08  /* Start of streaming message (verboase) */
#define PMU_MSG_EOTR_RDLAT    0x09  /* End of max read latency training */
#define PMU_MSG_EOTR_RDDQ     0x0a  /* End of read dq deskew training */
#define PMU_MSG_RSRV          0x0b  /* Reserved */
#define PMU_MSG_EOTR_ALLDB    0x0c  /* End of all DB training (MREP/DWL/MRD/MWD complete) */
#define PMU_MSG_EOTR_CA       0x0d  /* End of CA training */
#define PMU_MSG_EOTR_RDMPR    0xfd  /* End of MPR read delay center optimization */
#define PMU_MSG_EOTR_WLCOR    0xfe  /* End of Write leveling coarse delay */
#define PMU_MSG_EOFRW_FAIL    0xff  /* Training has failed (firmware complete) */

enum drv_stren_t {
	DrvStrenFSDqP_T = 1,
	DrvStrenFSDqN_T,
	ODTStrenP_T,
	ODTStrenN_T,
	ADrvStrenP_T,
	ADrvStrenN_T
};

struct pmu_msg_info {
	int id;
	const char *info;
};

int phyinit_G_WaitFWDone(int port);
void phyinit_LoadPieProdCode_udimm(int port);
void phyinit_LoadPieProdCode_rdimm(int port);
int phyinit_mapDrvStren(int DrvStren_ohm, enum drv_stren_t TargetCSR);

#endif /* DDR_PHY_MISC_H */
