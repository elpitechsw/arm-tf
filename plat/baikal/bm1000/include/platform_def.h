/*
 * Copyright (c) 2018-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <plat/common/common_def.h>

#ifdef IMAGE_BL1
# define PLATFORM_STACK_SIZE		U(0x800)
#else
# define PLATFORM_STACK_SIZE		U(0x2000)
#endif

#define PLATFORM_MAX_CPUS_PER_CLUSTER	U(2)
#define PLATFORM_CLUSTER_COUNT		U(4)
#define PLATFORM_CORE_COUNT_PER_CLUSTER	PLATFORM_MAX_CPUS_PER_CLUSTER
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER_COUNT * \
					 PLATFORM_CORE_COUNT_PER_CLUSTER)

#define PLAT_NUM_PWR_DOMAINS		(PLATFORM_CLUSTER_COUNT + \
					 PLATFORM_CORE_COUNT)

#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL1
#define PLAT_MAX_RET_STATE		U(1)
#define PLAT_MAX_OFF_STATE		U(2)

/* Local power state for power domains in Run state */
#define PLAT_LOCAL_STATE_RUN		U(0)
/* Local power state for retention. Valid only for CPU power domains */
#define PLAT_LOCAL_STATE_RET		U(1)
/* Local power state for power-down. Valid for CPU and cluster power domains */
#define PLAT_LOCAL_STATE_OFF		U(2)

/*
 * Macros used to parse state information from State-ID if it is using the
 * recommended encoding for State-ID.
 */
#define PLAT_LOCAL_PSTATE_WIDTH		U(4)
#define PLAT_LOCAL_PSTATE_MASK		((U(1) << PLAT_LOCAL_PSTATE_WIDTH) - 1)

/* Physical and virtual address space limits */
#define PLAT_PHY_ADDR_SPACE_SIZE	(ULL(1) << 40)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(ULL(1) << 40)

#define CACHE_WRITEBACK_SHIFT		U(6)
#define CACHE_WRITEBACK_GRANULE		(U(1) << CACHE_WRITEBACK_SHIFT)

#define MAX_MMAP_REGIONS		16
#define MAX_XLAT_TABLES			8
#define MAX_IO_DEVICES			3
#define MAX_IO_HANDLES			6

/* Partition memory into secure ROM, non-secure DRAM, secure "SRAM", and secure DRAM */
#define NS_DRAM0_BASE			0x80000000
#define NS_DRAM0_SIZE			0x10000000

#define SEC_DRAM_BASE			0x90000000
#define SEC_DRAM_SIZE			0x1000000

#define FB_SIZE				(64 * 1024 * 1024) /* 64 MiB is sufficient for any available resolution */
#define NS_DRAM1_BASE			0xa0000000
#define NS_DRAM1_SIZE			(0x20000000 - 3 * FB_SIZE) /* Reserve space for two framebuffers */
#define NS_DRAM1_LIMIT			(NS_DRAM1_BASE + NS_DRAM1_SIZE)
#define FB0_BASE			NS_DRAM1_LIMIT
#define FB1_BASE			(FB0_BASE + FB_SIZE)
#define FB2_BASE			(FB1_BASE + FB_SIZE)

/* Load pageable part of OP-TEE at end of secure DRAM */
#define BAIKAL_OPTEE_PAGEABLE_LOAD_BASE	(SEC_DRAM_BASE + SEC_DRAM_SIZE - \
					 BAIKAL_OPTEE_PAGEABLE_LOAD_SIZE)
#define BAIKAL_OPTEE_PAGEABLE_LOAD_SIZE	0x400000

/* BL1 lives in SRAM, partition it here */
#define SHARED_SRAM_BASE		0
#define SHARED_SRAM_SIZE		0x10000

#define BAIKAL_TRUSTED_MAILBOX_BASE	BL1_RW_BASE
#define BAIKAL_TRUSTED_MAILBOX_SIZE	(8 + BAIKAL_HOLD_SIZE)
#define BAIKAL_HOLD_BASE		(BAIKAL_TRUSTED_MAILBOX_BASE + 8)
#define BAIKAL_HOLD_SIZE		(PLATFORM_CORE_COUNT * BAIKAL_HOLD_ENTRY_SIZE)
#define BAIKAL_HOLD_ENTRY_SIZE		8
#define BAIKAL_HOLD_STATE_WAIT		0
#define BAIKAL_HOLD_STATE_GO		1

/* FDT related constants */
#define BAIKAL_SEC_DTB_BASE		(SEC_DRAM_BASE + BL1_XLAT_SIZE)
#define BAIKAL_NS_DTB_BASE		NS_DRAM0_BASE
#define BAIKAL_NS_IMAGE_OFFSET		NS_DRAM1_BASE
#define BAIKAL_NS_IMAGE_MAX_SIZE	NS_DRAM1_SIZE

/*
 * BL1 specific defines.
 *
 * BL1 RW data is relocated from ROM to RAM at runtime so we need 2 sets of
 * addresses.
 * Put BL1 RW at the top of the Secure SRAM. BL1_RW_BASE is calculated using
 * the current BL1 RW debug size plus a little space for growth.
 */
#define BL1_XLAT_BASE			SEC_DRAM_BASE
#define BL1_XLAT_SIZE			0x10000

#define BAIKAL_FIP_BASE			(SEC_DRAM_BASE + BL1_XLAT_SIZE + BAIKAL_DTB_MAX_SIZE)
#define BAIKAL_FIP_LIMIT		(BAIKAL_FIP_BASE + BAIKAL_FIP_MAX_SIZE)

/*
 * To limit amount of low-speed peripheral reads, that can be quite error prone,
 * once the program read it and configured DRAM, put it into specific location
 * to acces later when required.
 */
#define PLAT_DDR_SPD_BASE		(BAIKAL_FIP_BASE - 0x600)

#define BL1_RO_BASE			SHARED_SRAM_BASE
#define BL1_RO_SIZE			0xd000
#define BL1_RO_LIMIT			(SHARED_SRAM_BASE + BL1_RO_SIZE)
#define BL1_RW_BASE			(SHARED_SRAM_BASE + BL1_RO_SIZE)
#define SCP_SERVICE_BASE		0xeff0
#define BL1_RW_LIMIT			0xf000

/*
 * BL2 specific defines.
 *
 * Put BL2 just below BL3-1. BL2_BASE is calculated using the current BL2 debug
 * size plus a little space for growth.
 */
#define BL2_BASE			BAIKAL_FIP_LIMIT
#define BL2_SIZE			0x22000
#define BL2_LIMIT			(BL2_BASE + BL2_SIZE)

#define BL_RAM_BASE			BL2_BASE
#define BL_RAM_SIZE			(BL2_LIMIT - BL2_BASE)

/*
 * BL3-1 specific defines.
 *
 * Put BL3-1 at the top of the Trusted SRAM. BL31_BASE is calculated using the
 * current BL3-1 debug size plus a little space for growth.
 */
#define BL31_BASE			BL2_LIMIT
#define BL31_SIZE			0x100000
#define BL31_LIMIT			(BL31_BASE + BL31_SIZE)
#define BL31_PROGBITS_LIMIT		(BL31_LIMIT + 0x12000)

/*
 * BL3-2 specific defines.
 *
 * BL3-2 can execute from Secure SRAM, or Secure DRAM.
 */
#define BL32_SRAM_BASE			BL_RAM_BASE
#define BL32_SRAM_LIMIT			BL31_BASE
#define BL32_DRAM_BASE			BL31_PROGBITS_LIMIT
#define BL32_DRAM_LIMIT			(SEC_DRAM_BASE + SEC_DRAM_SIZE)

#define BL32_MEM_BASE			SEC_DRAM_BASE
#define BL32_MEM_SIZE			SEC_DRAM_SIZE
#define BL32_BASE			BL32_DRAM_BASE
#define BL32_LIMIT			BL32_DRAM_LIMIT

/* CCN related constants */
#define PLAT_ARM_CCN_BASE		U(0x9000000)
#define PLAT_ARM_CLUSTER_TO_CCN_ID_MAP	1, 9, 11, 19
#define PLAT_ARM_CLUSTER_COUNT		PLATFORM_CLUSTER_COUNT
#define CCN_HNF_OFFSET			U(0x200000)
#define CCN_XP_OFFSET			U(0x400000)
#define XP_DEV_QOS_CONTROL(x, y)	(PLAT_ARM_CCN_BASE + CCN_XP_OFFSET + 0x10000 * (x) + 0x100 * (y) + 0x110)
#define DEV_QOS_OVERRIDE_MASK		GENMASK(   19, 16)
#define DEV_QOS_OVERRIDE(x)		SETMASK(x, 19, 16)
#define DEV_QOS_OVERRIDE_EN		BIT(2)

/* Parts of the boot image */
#define BAIKAL_BOOT_OFFSET		0
#define BAIKAL_BOOT_MAX_SIZE		(8 * 1024 * 1024)

#define BAIKAL_SCP_MAX_SIZE		(512 * 1024)
#define BAIKAL_BL1_MAX_SIZE		(256 * 1024)
#define BAIKAL_DTB_MAX_SIZE		(256 * 1024)
#define BAIKAL_VAR_MAX_SIZE		(768 * 1024)
#define BAIKAL_FIP_MAX_SIZE		(BAIKAL_BOOT_MAX_SIZE - BAIKAL_FIP_OFFSET)

#define BAIKAL_BL1_OFFSET		0
#define BAIKAL_DTB_OFFSET		(BAIKAL_BL1_OFFSET + BAIKAL_BL1_MAX_SIZE)
#define BAIKAL_VAR_OFFSET		(BAIKAL_DTB_OFFSET + BAIKAL_DTB_MAX_SIZE)
#define BAIKAL_FIP_OFFSET		(BAIKAL_VAR_OFFSET + BAIKAL_VAR_MAX_SIZE)
#define BAIKAL_FAT_OFFSET		(BAIKAL_FIP_OFFSET + BAIKAL_FIP_MAX_SIZE)

#define BAIKAL_SD_FIRMWARE
#define BAIKAL_SD_FIRMWARE_DEBUG
#define BAIKAL_SD_FIRMWARE_OFFSET	(2048 * 512) /* dev/mmcblk0p1: LBA 2048 */

#endif /* PLATFORM_DEF_H */
