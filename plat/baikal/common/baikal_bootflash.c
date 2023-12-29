/*
 * Copyright (c) 2022, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <baikal_bootflash.h>
#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
# include <bm1000_scp_flash.h>
#else
# include <baikal_def.h>
# include <dw_spi_flash.h>
#endif

int bootflash_init(void)
{
#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
	return scp_flash_init();
#else
	return spi_flash_init(BAIKAL_BOOT_SPI_SS_LINE);
#endif
}

int bootflash_erase(uint32_t addr, size_t size)
{
#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
	return scp_flash_erase(addr, size);
#else
	return spi_flash_erase(BAIKAL_BOOT_SPI_SS_LINE, addr, size);
#endif
}

int bootflash_read(uint32_t addr, void *buf, size_t size)
{
#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
	return scp_flash_read(addr, buf, size);
#else
	return spi_flash_read(BAIKAL_BOOT_SPI_SS_LINE, addr, buf, size);
#endif
}

int bootflash_write(uint32_t addr, void *data, size_t size)
{
#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
	return scp_flash_write(addr, data, size);
#else
	return spi_flash_write(BAIKAL_BOOT_SPI_SS_LINE, addr, data, size);
#endif
}
