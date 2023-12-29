/*
 * Copyright (c) 2019-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * Author: Pavel Parkhomenko <pavel.parkhomenko@baikalelectronics.ru>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef BM1000_VDU_H
#define BM1000_VDU_H

/* Controller register offsets */
#define BAIKAL_VDU_CR1		0x000
#define BAIKAL_VDU_HTR		0x008
#define BAIKAL_VDU_VTR1		0x00c
#define BAIKAL_VDU_VTR2		0x010
#define BAIKAL_VDU_PCTR		0x014
#define BAIKAL_VDU_ISR		0x018
#define BAIKAL_VDU_IMR		0x01c
#define BAIKAL_VDU_IVR		0x020
#define BAIKAL_VDU_ISCR		0x024
#define BAIKAL_VDU_DBAR		0x028
#define BAIKAL_VDU_DEAR		0x030
#define BAIKAL_VDU_PWMFR	0x034
#define BAIKAL_VDU_PWMDCR	0x038
#define BAIKAL_VDU_HVTER	0x044
#define BAIKAL_VDU_HPPLOR	0x048
#define BAIKAL_VDU_GPIOR	0x1f8
#define BAIKAL_VDU_CIR		0x1fc
#define BAIKAL_VDU_MRR		0xffc

/*******************************************************************************
 * Register components (register bits)
 *
 * This should make life easier to program specific settings in the different
 * registers by simplifying the setting up of the individual bits of each
 * register and then assembling the final register value.
 ******************************************************************************/

/* Register: HTR */
#define HOR_AXIS_PANEL(hbp, hfp, hsw, hor_res) (uint32_t)	\
		(((uint32_t)(((hsw) - 1) & 0xff)      << 24) |	\
		 ((uint32_t)((hbp) & 0xff)            << 16) |	\
		 ((uint32_t)(((hor_res) / 16) & 0xff) <<  8) |	\
		 ((uint32_t)((hfp) & 0xff)            <<  0))

/* Register: VTR1 */
#define VER_AXIS_PANEL(vbp, vfp, vsw) (uint32_t)		\
		(((uint32_t)((vbp) & 0xff) << 16) |		\
		 ((uint32_t)((vfp) & 0xff) <<  8) |		\
		 ((uint32_t)((vsw) & 0xff) <<  0))

/* Register: HVTER */
#define TIMINGS_EXT(hbp, hfp, hsw, vbp, vfp, vsw) (uint32_t)	\
		(((uint32_t)((vsw) / 256) << 24) |		\
		 ((uint32_t)((hsw) / 256) << 16) |		\
		 ((uint32_t)((vbp) / 256) << 12) |		\
		 ((uint32_t)((vfp) / 256) <<  8) |		\
		 ((uint32_t)((hbp) / 256) <<  4) |		\
		 ((uint32_t)((hfp) / 256) <<  0))

#define BAIKAL_VDU_CR1_FDW_4_WORDS	(0 << 16)
#define BAIKAL_VDU_CR1_FDW_8_WORDS	(1 << 16)
#define BAIKAL_VDU_CR1_FDW_16_WORDS	(2 << 16)
#define BAIKAL_VDU_CR1_OPS_LCD18	(0 << 13)
#define BAIKAL_VDU_CR1_OPS_LCD24	(1 << 13)
#define BAIKAL_VDU_CR1_OPS_555		(1 << 12)
#define BAIKAL_VDU_CR1_VSP		(1 << 11)
#define BAIKAL_VDU_CR1_HSP		(1 << 10)
#define BAIKAL_VDU_CR1_DEP		(1 << 8)
#define BAIKAL_VDU_CR1_BGR		(1 << 5)
#define BAIKAL_VDU_CR1_BPP_MASK		GENMASK(4, 2)
#define BAIKAL_VDU_CR1_BPP1		(0 << 2)
#define BAIKAL_VDU_CR1_BPP2		(1 << 2)
#define BAIKAL_VDU_CR1_BPP4		(2 << 2)
#define BAIKAL_VDU_CR1_BPP8		(3 << 2)
#define BAIKAL_VDU_CR1_BPP16		(4 << 2)
#define BAIKAL_VDU_CR1_BPP18		(5 << 2)
#define BAIKAL_VDU_CR1_BPP24		(6 << 2)
#define BAIKAL_VDU_CR1_LCE		(1 << 0)

#define BAIKAL_VDU_HPPLOR_HPOE		(1UL << 31)

#define BAIKAL_VDU_PCTR_PCR		(1 << 10)
#define BAIKAL_VDU_PCTR_PCI		(1 << 9)

#define BAIKAL_VDU_MRR_DEAR_RQ_MASK	GENMASK(2, 0)
#define BAIKAL_VDU_MRR_OUTSTND_RQ(x)	((x >> 1) << 0)

#define BAIKAL_VDU_INTR_BAU		(1 << 7)
#define BAIKAL_VDU_INTR_VCT		(1 << 6)

#define BAIKAL_VDU_ISCR_VSC_OFF		0
#define BAIKAL_VDU_ISCR_VSC_VSW		4
#define BAIKAL_VDU_ISCR_VSC_VBP		5
#define BAIKAL_VDU_ISCR_VSC_VACTIVE	6
#define BAIKAL_VDU_ISCR_VSC_VFP		7

#define BAIKAL_VDU_GPIOR_UHD_SNGL_PORT	(0 << 18)
#define BAIKAL_VDU_GPIOR_UHD_DUAL_PORT	(1 << 18)
#define BAIKAL_VDU_GPIOR_UHD_QUAD_PORT	(2 << 18)
#define BAIKAL_VDU_GPIOR_UHD_ENB	(1 << 17)

#define BAIKAL_VDU_PWMFR_PWMPCR		(1 << 24)
#define BAIKAL_VDU_PWMFR_PWMFCI		(1 << 23)
#define BAIKAL_VDU_PWMFR_PWMFCE		(1 << 22)
#define BAIKAL_VDU_PWMFR_PWMFCD_MASK	GENMASK(21, 0)
#define BAIKAL_VDU_PWMFR_PWMFCD(x)	((x) << 0)

#define BAIKAL_VDU_PERIPH_ID		0x0090550f

#endif /* BM1000_VDU_H */
