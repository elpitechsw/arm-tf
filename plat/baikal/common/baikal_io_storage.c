/*
 * Copyright (c) 2018-2022, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/io/io_block.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>
#include <drivers/io/io_storage.h>
#include <endian.h>
#include <lib/utils_def.h>
#include <libfdt.h>
#include <tools_share/firmware_image_package.h>

#include <baikal_bootflash.h>
#include <crc.h>
#include <platform_def.h>

#include <drivers/mmc.h>
#include <baikal_mshc.h>
#include <baikal_def.h>

/* IO devices */
static const io_dev_connector_t *fip_dev_con;
static uintptr_t fip_dev_handle;
static const io_dev_connector_t *memmap_dev_con;
static uintptr_t memmap_dev_handle;

static const io_block_spec_t fip_block_spec = {
	.offset = BAIKAL_FIP_BASE,
	.length = BAIKAL_FIP_MAX_SIZE
};

static const io_uuid_spec_t bl2_uuid_spec = {
	.uuid = UUID_TRUSTED_BOOT_FIRMWARE_BL2,
};

static const io_uuid_spec_t bl31_uuid_spec = {
	.uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31,
};

static const io_uuid_spec_t bl32_uuid_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32,
};

static const io_uuid_spec_t bl32_extra1_uuid_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32_EXTRA1,
};

static const io_uuid_spec_t bl32_extra2_uuid_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32_EXTRA2,
};

static const io_uuid_spec_t bl33_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33,
};

#if TRUSTED_BOARD_BOOT
static const io_uuid_spec_t bl2_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_BOOT_FIRMWARE_BL2_CERT,
};

static const io_uuid_spec_t trusted_key_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_KEY_CERT,
};

static const io_uuid_spec_t bl31_key_cert_uuid_spec = {
	.uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31_KEY_CERT,
};

static const io_uuid_spec_t bl32_key_cert_uuid_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32_KEY_CERT,
};

static const io_uuid_spec_t bl33_key_cert_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33_KEY_CERT,
};

static const io_uuid_spec_t bl31_cert_uuid_spec = {
	.uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31_CERT,
};

static const io_uuid_spec_t bl32_cert_uuid_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32_CERT,
};

static const io_uuid_spec_t bl33_cert_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33_CERT,
};
#endif /* TRUSTED_BOARD_BOOT */

static int open_fip(const uintptr_t spec);
static int open_memmap(const uintptr_t spec);

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

/* By default, ARM platforms load images from the FIP */
static const struct plat_io_policy policies[] = {
	[FIP_IMAGE_ID] = {
		&memmap_dev_handle,
		(uintptr_t)&fip_block_spec,
		open_memmap
	},
	[BL2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl2_uuid_spec,
		open_fip
	},
	[BL31_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl31_uuid_spec,
		open_fip
	},
	[BL32_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_uuid_spec,
		open_fip
	},
	[BL32_EXTRA1_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_extra1_uuid_spec,
		open_fip
	},
	[BL32_EXTRA2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_extra2_uuid_spec,
		open_fip
	},
	[BL33_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl33_uuid_spec,
		open_fip
	},
#if TRUSTED_BOARD_BOOT
	[BL2_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl2_cert_uuid_spec,
		open_fip
	},
	[TRUSTED_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&trusted_key_cert_uuid_spec,
		open_fip
	},
	[BL31_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl31_key_cert_uuid_spec,
		open_fip
	},
	[BL32_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_key_cert_uuid_spec,
		open_fip
	},
	[BL33_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl33_key_cert_uuid_spec,
		open_fip
	},
	[BL31_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl31_cert_uuid_spec,
		open_fip
	},
	[BL32_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_cert_uuid_spec,
		open_fip
	},
	[BL33_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl33_cert_uuid_spec,
		open_fip
	},
#endif /* TRUSTED_BOARD_BOOT */
};

static int open_fip(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	/* See if a Firmware Image Package is available */
	result = io_dev_init(fip_dev_handle, (uintptr_t)FIP_IMAGE_ID);
	if (result == 0) {
		result = io_open(fip_dev_handle, spec, &local_image_handle);
		if (result == 0) {
			io_close(local_image_handle);
			return 0;
		}
	}

	return local_image_handle;
}

#ifdef IMAGE_BL1
/* Return 0 for equal uuids. */
static inline int compare_uuids(const uuid_t *uuid1, const uuid_t *uuid2)
{
	return memcmp(uuid1, uuid2, sizeof(uuid_t));
}

size_t bootflash_read_blocks(int lba, uintptr_t buf, size_t size)
{
	if (bootflash_read(lba * MMC_BLOCK_SIZE, (void *)buf, size)) {
		return 0;
	} else {
		return size;
	}
}

/* TODO: use io_open, io_read */
static int read_fdt(uintptr_t src, uintptr_t dst, void *func)
{
	size_t (*read_blocks)(int lba, uintptr_t dst, size_t size) = func;
	int err;
	const struct fdt_header *const fdt_header = (void *)dst;
	uint32_t fdt_totalsize;

	err = read_blocks((src / MMC_BLOCK_SIZE),
		ROUND_DOWN(dst),
		ROUND_UP(sizeof(struct fdt_header)));
	if (!err) {
		VERBOSE("%s: -- read_blocks\n", __func__);
		return -1;
	}

	if (be32toh(fdt_header->magic) != FDT_MAGIC) {
		ERROR("%s: FDT header magic is wrong\n", __func__);
		return -1;
	}

	fdt_totalsize = be32toh(fdt_header->totalsize);
	if (fdt_totalsize < sizeof(struct fdt_header) ||
	    fdt_totalsize > BAIKAL_DTB_MAX_SIZE) {
		ERROR("%s: FDT total size is wrong: %u\n", __func__, fdt_totalsize);
		return -1;
	}

	err = read_blocks((src / MMC_BLOCK_SIZE),
		ROUND_DOWN(dst),
		ROUND_UP(fdt_totalsize));
	if (!err) {
		VERBOSE("%s: -- read_blocks\n", __func__);
		return -1;
	}

	INFO("BL1: DTB crc32:0x%08x size:%u\n",
		crc32((void *)BAIKAL_SEC_DTB_BASE, fdt_totalsize, 0),
		fdt_totalsize);

	return 0;
}
static int read_fip(uintptr_t src, uintptr_t dst, uintptr_t local_image_handle, void *func)
{
	size_t (*read_blocks)(int lba, uintptr_t dst, size_t size) = func;
	fip_toc_entry_t entry = {0};
	int result, size = 0;
	size_t bytes_read;
	static const uuid_t uuid_null = {0};

	/* Read FIP Header part */
	result = io_seek(local_image_handle, IO_SEEK_SET, sizeof(fip_toc_header_t));
	if (result != 0) {
		VERBOSE("%s: -- io_seek\n", __func__);
		return -1;
	}

	result = read_blocks((src / MMC_BLOCK_SIZE),
		ROUND_DOWN(dst),
		ROUND_UP(sizeof(fip_toc_header_t)));
	if (!result) {
		VERBOSE("%s: -- read_blocks\n", __func__);
		return -1;
	}

	src += sizeof(fip_toc_header_t);
	dst += sizeof(fip_toc_header_t);
	do {
		result = read_blocks((src / MMC_BLOCK_SIZE),
			ROUND_DOWN(dst),
			ROUND_UP(sizeof(fip_toc_entry_t)));
		if (!result) {
			VERBOSE("%s: -- read_blocks\n", __func__);
			return -1;
		}

		result = io_read(local_image_handle,
				 (uintptr_t)&entry,
				 sizeof(entry),
				 &bytes_read);
		if (result != 0) {
			VERBOSE("%s: -- io_read\n", __func__);
			return -1;
		}

		src += sizeof(fip_toc_entry_t);
		dst += sizeof(fip_toc_entry_t);
		size += entry.size;

	} while (compare_uuids(&entry.uuid, &uuid_null) != 0);

	result = read_blocks((src / MMC_BLOCK_SIZE),
		ROUND_DOWN(dst),
		ROUND_UP(size));
	if (!result) {
		VERBOSE("%s: -- read_blocks\n", __func__);
		return -1;
	}

	INFO("BL1: FIP crc32:0x%08x size:%lu\n",
		crc32((void *)BAIKAL_FIP_BASE, dst + size - BAIKAL_FIP_BASE, 0),
		dst + size - BAIKAL_FIP_BASE);

	return 0;
}

#ifdef BAIKAL_SD_FIRMWARE_DEBUG
int mmc_test(uint32_t dst)
{
	int i = 0;
	uint32_t cnt[] = {1, 100, 1000, 0};

	while (cnt[i]) {
		uint32_t size = MMC_BLOCK_SIZE * cnt[i];
		uintptr_t buf  = BAIKAL_FIP_BASE + 0 * size;
		uintptr_t bufa = BAIKAL_FIP_BASE + 1 * size;
		uintptr_t bufb = BAIKAL_FIP_BASE + 2 * size;

		memset((void *)bufa, 0xaa, size);
		memset((void *)bufb, 0xbb, size);

		mmc_read_blocks(dst	/ MMC_BLOCK_SIZE, buf,  size); /* save */
		mmc_write_blocks(dst	/ MMC_BLOCK_SIZE, bufa, size);
		mmc_read_blocks(dst	/ MMC_BLOCK_SIZE, bufb, size);
		mmc_write_blocks(dst	/ MMC_BLOCK_SIZE, buf,  size); /* restore */

		int ret = memcmp((void *)bufa, (void *)bufb, size);

		if (ret) {
			VERBOSE("%s: -- mmc fail\n", __func__);
			return ret;
		}

		i++;
	}

	return 0;
}
int mmc_copy(uint32_t dst, uint32_t src, uint32_t size)
{
	uintptr_t buf = BAIKAL_FIP_BASE;
	uint64_t part;

	bootflash_init();
	while (size) {
		part = MIN(size, 1024 * MMC_BLOCK_SIZE);
		bootflash_read(src, (void *)buf, part);
		mmc_write_blocks(dst / MMC_BLOCK_SIZE, buf, part);
		src  += part;
		dst  += part;
		size -= part;
	}

	return 0;
}
#endif /* BAIKAL_SD_FIRMWARE_DEBUG */

static int read_image(uintptr_t local_image_handle)
{
	int ret;

	/* sd */
#ifdef BAIKAL_SD_FIRMWARE
	ret = dw_mshc_init();
	if (ret) {
		goto skip;
	}

#ifdef BAIKAL_SD_FIRMWARE_TEST
	VERBOSE("BL1: test sd/mmc...\n");
	ret = mmc_test(BAIKAL_SD_FIRMWARE_OFFSET);
	if (ret) {
		goto skip;
	}
#endif

#ifdef BAIKAL_SD_FIRMWARE_DEBUG
	VERBOSE("BL1: copy firmware to sd/mmc...\n");
	ret = mmc_copy(BAIKAL_SD_FIRMWARE_OFFSET, BAIKAL_BOOT_OFFSET, BAIKAL_BOOT_MAX_SIZE);
	if (ret) {
		goto skip;
	}
#endif /* BAIKAL_SD_FIRMWARE_DEBUG */

	VERBOSE("BL1: read firmware from sd/mmc...\n");
	ret = read_fdt(BAIKAL_SD_FIRMWARE_OFFSET + BAIKAL_DTB_OFFSET, BAIKAL_SEC_DTB_BASE, (void *)mmc_read_blocks);
	if (ret) {
		goto skip;
	}
	ret = read_fip(BAIKAL_SD_FIRMWARE_OFFSET + BAIKAL_FIP_OFFSET, BAIKAL_FIP_BASE, local_image_handle, (void *)mmc_read_blocks);

skip:
	dw_mshc_off();
	if (ret == 0) {
		return ret;
	}
#endif /* BAIKAL_SD_FIRMWARE */

	/* flash */
	ret = bootflash_init();
	if (ret) {
		return ret;
	}
	VERBOSE("BL1: read firmware from spi flash...\n");
	ret  = read_fdt(BAIKAL_DTB_OFFSET, BAIKAL_SEC_DTB_BASE, (void *)bootflash_read_blocks);
	if (ret) {
		return ret;
	}
	ret = read_fip(BAIKAL_FIP_OFFSET, BAIKAL_FIP_BASE, local_image_handle, (void *)bootflash_read_blocks);
	return ret;
}
#endif /* IMAGE_BL1 */

static inline int is_valid_header(fip_toc_header_t *header)
{
	if ((header->name == TOC_HEADER_NAME) && (header->serial_number != 0)) {
		return 1;
	} else {
		return 0;
	}
}

static int open_memmap(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	result = io_dev_init(memmap_dev_handle, (uintptr_t)NULL);

	if (result == 0) {
		result = io_open(memmap_dev_handle, spec, &local_image_handle);
		if (result == 0) {
#ifdef IMAGE_BL1
			result = read_image(local_image_handle);
#endif
			if (!(is_valid_header((fip_toc_header_t *)BAIKAL_FIP_BASE))) {
				VERBOSE("%s: -- broken fip\n", __func__);
				result = -1;
			}

			io_close(local_image_handle);
		}
	}

	return result;
}

void plat_baikal_io_setup(void)
{
	int io_result;

	io_result = register_io_dev_fip(&fip_dev_con);
	assert(io_result == 0);

	io_result = register_io_dev_memmap(&memmap_dev_con);
	assert(io_result == 0);

	/* Open connections to devices and cache the handles */
	io_result = io_dev_open(fip_dev_con, (uintptr_t)NULL,
				&fip_dev_handle);
	assert(io_result == 0);

	io_result = io_dev_open(memmap_dev_con, (uintptr_t)NULL,
				&memmap_dev_handle);
	assert(io_result == 0);

	/* Ignore improbable errors in release builds */
	(void)io_result;
}

/*
 * Return an IO device handle and specification which can be used to access
 * an image. Use this to enforce platform load policy
 */
int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	int result;
	const struct plat_io_policy *policy;

	assert(image_id < ARRAY_SIZE(policies));

	policy = &policies[image_id];
	result = policy->check(policy->image_spec);
	if (result == 0) {
		*image_spec = policy->image_spec;
		*dev_handle = *(policy->dev_handle);
	}

	return result;
}
