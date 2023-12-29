/*
 * Copyright (c) 2018-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/utils_def.h>

#include <baikal_def.h>
#include <dw_gpio.h>
#include <dw_spi_flash.h>

#define SPI_NOR_MAX_ID_LEN	20

/* Registers */
#define SPI_CTRLR0	*(volatile uint32_t *)(BAIKAL_BOOT_SPI_BASE + 0x00)
#define SPI_CTRLR1	*(volatile uint32_t *)(BAIKAL_BOOT_SPI_BASE + 0x04)
#define SPI_SPIENR	*(volatile uint32_t *)(BAIKAL_BOOT_SPI_BASE + 0x08)
#define SPI_SER		*(volatile uint32_t *)(BAIKAL_BOOT_SPI_BASE + 0x10)
#define SPI_BAUDR	*(volatile uint32_t *)(BAIKAL_BOOT_SPI_BASE + 0x14)
#define SPI_TXFTLR	*(volatile uint32_t *)(BAIKAL_BOOT_SPI_BASE + 0x18)
#define SPI_RXFTLR	*(volatile uint32_t *)(BAIKAL_BOOT_SPI_BASE + 0x1c)
#define SPI_SR		*(volatile uint32_t *)(BAIKAL_BOOT_SPI_BASE + 0x28)
#define SPI_IMR		*(volatile uint32_t *)(BAIKAL_BOOT_SPI_BASE + 0x2c)
#define SPI_RISR	*(volatile uint32_t *)(BAIKAL_BOOT_SPI_BASE + 0x34)
#define SPI_DMACR	*(volatile uint32_t *)(BAIKAL_BOOT_SPI_BASE + 0x4c)
#define SPI_DMATDLR	*(volatile uint32_t *)(BAIKAL_BOOT_SPI_BASE + 0x50)
#define SPI_DMARDLR	*(volatile uint32_t *)(BAIKAL_BOOT_SPI_BASE + 0x54)
#define SPI_DR		*(volatile uint32_t *)(BAIKAL_BOOT_SPI_BASE + 0x60)
#define SPI_RX_DLY	*(volatile uint32_t *)(BAIKAL_BOOT_SPI_BASE + 0xf0)

/* CTRLR0 */
#define SPI_DFS_OFFSET		0
#define SPI_TMOD_OFFSET		8
#define SPI_TMOD_MASK		(3 << SPI_TMOD_OFFSET)
#define SPI_TMOD_TXRX		0
#define SPI_TMOD_TX		1
#define SPI_TMOD_RX		2
#define SPI_TMOD_EEPROM		3

/* SPIENR */
#define SPI_SPIENR_SPI_DE	0
#define SPI_SPIENR_SPI_EN	1

/* SR */
#define SPI_SR_BUSY		BIT(0) /* Serial transfer in progress */
#define SPI_SR_TFNF		BIT(1) /* Tx FIFO Not Full */
#define SPI_SR_TFE		BIT(2) /* Tx FIFO Empty */
#define SPI_SR_RFNE		BIT(3) /* Rx FIFO Not Empty */
#define SPI_SR_RFF		BIT(4) /* Rx FIFO Full */
#define SPI_SR_DCOL		BIT(6) /* Data Collision Error */

/* RISR */
#define SPI_RISR_TXE		BIT(0) /* TX Empty */
#define SPI_RISR_TXO		BIT(1) /* TX Overflow */
#define SPI_RISR_RXU		BIT(2) /* RX Underflow */
#define SPI_RISR_RXO		BIT(3) /* RX Overflow */
#define SPI_RISR_RXF		BIT(4) /* RX Full */
#define SPI_RISR_MST		BIT(5) /* Multi Master */
#define SPI_RISR_ERR		(SPI_RISR_TXO | SPI_RISR_RXU | SPI_RISR_RXO | SPI_RISR_RXF)

/* SPI Flash status register */
#define SPI_FLASH_SR_WIP	BIT(0) /* Write In Progress */
#define SPI_FLASH_SR_WEL	BIT(1) /* Write Enable Latch */

/* SPI Flash commands */
#define CMD_FLASH_RDID		0x9f /* (0, 1 .. 20) Read identification */
#define CMD_FLASH_READ		0x03 /* (3, 1 .. inf) Read Data Bytes */
#define CMD_FLASH_WREN		0x06 /* (0, 0) Write Enable */
#define CMD_FLASH_WRDI		0x04 /* (0, 0) Write Disable */
#define CMD_FLASH_PP		0x02 /* (3, 256) Page Program */
#define CMD_FLASH_SSE		0x20 /* (3, 0) SubSector Erase */
#define CMD_FLASH_SE		0xd8 /* (3, 0) Sector Erase */
#define CMD_FLASH_RDSR		0x05 /* (0, 1) Read Status Register */
#define CMD_FLASH_WRSR		0x01 /* (0, 1 .. inf) Write Status Register */
#define CMD_FLASH_RFSR		0x70 /* (1 .. inf) Read Flag Status Register */
#define CMD_FLASH_BE		0xc7 /* (0) Bulk Erase */
#define CMD_FLASH_EN4BYTEADDR	0xb7 /* Enter 4-byte address mode */
#define CMD_FLASH_EX4BYTEADDR	0xe9 /* Exit 4-byte address mode */
#define CMD_FLASH_CLFSR		0x50 /* Clear Flag Status Register */
#define CMD_FLASH_RDNVCR	0xb5 /* Read Non Volatile Configuration Register */
#define CMD_FLASH_WRNVCR	0xb1 /* Write Non Volatile Configuration Register */
#define CMD_FLASH_RDVCR		0x85 /* Read Volatile Configuration Register */
#define CMD_FLASH_WRVCR		0x81 /* Write Volatile Configuration Register */
#define CMD_FLASH_RDVECR	0x65 /* Read Volatile Enhanced Configuration Register */
#define CMD_FLASH_WRVECR	0x61 /* Write Volatile Enhanced Configuration Register */
#define CMD_FLASH_WREAR		0xc5 /* Write Extended Address Register */
#define CMD_FLASH_RDEAR		0xc8 /* Read Extended Address Register */

/* Put address */
#define ADR_MODE_3BYTE		3
#define SPI_ADR_LEN_3BYTE	3
#define SPI_SET_ADDRESS_3BYTE(a, b)		\
({	uint8_t *_b = (void *)(b);		\
	_b[1] = (((a) >> 8 * 2) & 0xff);	\
	_b[2] = (((a) >> 8 * 1) & 0xff);	\
	_b[3] = (((a) >> 8 * 0) & 0xff);	\
})

#define ADR_MODE_4BYTE		4
#define SPI_ADR_LEN_4BYTE	4
#define SPI_SET_ADDRESS_4BYTE(a, b)		\
({	uint8_t *_b = (void *)(b);		\
	_b[1] = (((a) >> 8 * 3) & 0xff);	\
	_b[2] = (((a) >> 8 * 2) & 0xff);	\
	_b[3] = (((a) >> 8 * 1) & 0xff);	\
	_b[4] = (((a) >> 8 * 0) & 0xff);	\
})

#define SPI_CMD_LEN		(1 + SPI_ADR_LEN_4BYTE)
#define SPI_MAX_READ		UL(0x10000)
#define SPI_MAX_WRITE		UL(256) /* (3, 256) Page Program */

static unsigned int adr_mode;

static int transfer(const unsigned int line,
		    void *cmd_, uint32_t cmd_len,
		    void *tx_, uint32_t tx_len,
		    void *rx_, uint32_t rx_len)
{
	int err = 0;
	uint8_t *cmd = cmd_;
	uint8_t *tx = tx_;
	uint8_t *rx = rx_;
	uint8_t *cmdend	= (void *)((intptr_t)cmd + (intptr_t)cmd_len);
	uint8_t *txend	= (void *)((intptr_t)tx	 + (intptr_t)tx_len);
	uint8_t *rxend	= (void *)((intptr_t)rx	 + (intptr_t)rx_len);
	uint64_t timeout = timeout_init_us(2 * 1000 * 1000);

	if (cmd == NULL || !cmd_len || line > 3 ||
	    (rx_len > 0 && rx == NULL) || rx_len > SPI_MAX_READ) {
		err = -EINVAL;
		ERROR("SPI: %s error %d (L%u)\n", __func__, err, __LINE__);
		return err;
	}

	SPI_SPIENR = SPI_SPIENR_SPI_DE;
	SPI_SER = 0;
#ifdef BAIKAL_BOOT_SPI_CS_GPIO_PIN
	gpio_out_rst(BAIKAL_BOOT_SPI_GPIO_BASE, BAIKAL_BOOT_SPI_CS_GPIO_PIN); /* GPIO = 0 */
	gpio_dir_set(BAIKAL_BOOT_SPI_GPIO_BASE, BAIKAL_BOOT_SPI_CS_GPIO_PIN);
#endif
	if (SPI_BAUDR < BAIKAL_BOOT_SPI_BAUDR) {
		SPI_BAUDR = BAIKAL_BOOT_SPI_BAUDR;
	}

	if (rx_len == 0) {
		SPI_CTRLR0 = (SPI_CTRLR0 & ~SPI_TMOD_MASK) | (SPI_TMOD_TX << SPI_TMOD_OFFSET);
		SPI_SPIENR = SPI_SPIENR_SPI_EN; /* enable FIFO */
		while (cmd < cmdend) {
			if (SPI_SR & SPI_SR_TFNF) {
				SPI_DR = *cmd++;
			}
		}

		SPI_SER = 1 << line; /* start sending */
		while (tx < txend) {
			if (SPI_SR & SPI_SR_TFNF) {
				SPI_DR = *tx++;
			} else if (timeout_elapsed(timeout)) {
				err = -ETIMEDOUT;
				ERROR("SPI: %s error %d (L%u)\n", __func__, err, __LINE__);
				goto exit;
			}
		}
	} else {
		SPI_CTRLR0 = (SPI_CTRLR0 & ~SPI_TMOD_MASK) | (SPI_TMOD_EEPROM << SPI_TMOD_OFFSET);
		SPI_CTRLR1 = rx_len - 1; /* set read size */
		SPI_SPIENR = SPI_SPIENR_SPI_EN; /* enable FIFO */
		while (cmd < cmdend) {
			if (SPI_SR & SPI_SR_TFNF) {
				SPI_DR = *cmd++;
			}
		}

		SPI_SER = 1 << line; /* start sending */
		while (rx < rxend) { /* read incoming data */
			if (SPI_SR & SPI_SR_RFNE) {
				*rx++ = SPI_DR;
			} else if (timeout_elapsed(timeout)) {
				err = -ETIMEDOUT;
				ERROR("SPI: %s error %d (L%u)\n", __func__, err, __LINE__);
				goto exit;
			}
		}
	}

	while (SPI_SR & SPI_SR_BUSY) {
		if (timeout_elapsed(timeout)) {
			err = -ETIMEDOUT;
			ERROR("SPI: %s error %d (L%u)\n", __func__, err, __LINE__);
			goto exit;
		}
	}

exit:
#ifdef BAIKAL_BOOT_SPI_CS_GPIO_PIN
	gpio_out_set(BAIKAL_BOOT_SPI_GPIO_BASE, BAIKAL_BOOT_SPI_CS_GPIO_PIN); /* GPIO = 1 */
	gpio_dir_clr(BAIKAL_BOOT_SPI_GPIO_BASE, BAIKAL_BOOT_SPI_CS_GPIO_PIN);
#endif
	SPI_SER = 0;
	SPI_SPIENR = SPI_SPIENR_SPI_DE;
	if (SPI_RISR & SPI_RISR_ERR) {
		err = -1;
		ERROR("SPI: %s error %d (L%u)\n", __func__, err, __LINE__);
	}

	if (SPI_SR & (SPI_SR_RFF | SPI_SR_DCOL)) {
		err = -1;
		ERROR("SPI: %s error %d (L%u)\n", __func__, err, __LINE__);
	}

	return err;
}

static int exec(int line,
	 uint8_t cmd_op,
	 uint32_t address,
	 void *buf,
	 uint32_t lenbuf)
{
	int err;
	uint8_t cmd[SPI_CMD_LEN];
	uint8_t *in = 0, *out = 0;
	uint32_t lencmd = 0, lenin = 0, lenout = 0;

	/* Save the SPI flash instruction */
	cmd[0] = cmd_op;
	lencmd += sizeof(cmd_op);

	/* Prepare arguments for the SPI transaction */
	switch (cmd_op) {
	case CMD_FLASH_RDID:
	case CMD_FLASH_RDSR:
	case CMD_FLASH_RDNVCR:
	case CMD_FLASH_RDVCR:
	case CMD_FLASH_RDVECR:
	case CMD_FLASH_RDEAR:
	case CMD_FLASH_RFSR:
		out = buf;
		lenout = lenbuf;
		break;
	case CMD_FLASH_READ:
		out = buf;
		lenout = lenbuf;
		/* fallthrough */
	case CMD_FLASH_SSE:
	case CMD_FLASH_SE:
		if (adr_mode == ADR_MODE_4BYTE) {
			SPI_SET_ADDRESS_4BYTE(address, cmd);
			lencmd += SPI_ADR_LEN_4BYTE;
		} else if (adr_mode == ADR_MODE_3BYTE) {
			SPI_SET_ADDRESS_3BYTE(address, cmd);
			lencmd += SPI_ADR_LEN_3BYTE;
		} else {
			ERROR("SPI: %s: incorrect address mode\n", __func__);
			return -ECAPMODE;
		}
		break;
	case CMD_FLASH_WREAR:
	case CMD_FLASH_WRSR:
	case CMD_FLASH_WRVCR:
	case CMD_FLASH_WRVECR:
	case CMD_FLASH_WRNVCR:
		in = buf;
		lenin = lenbuf;
		break;
	case CMD_FLASH_EN4BYTEADDR:
	case CMD_FLASH_EX4BYTEADDR:
	case CMD_FLASH_WRDI:
	case CMD_FLASH_WREN:
	case CMD_FLASH_BE:
		break;
	case CMD_FLASH_PP:
		if (lenbuf > SPI_MAX_WRITE) {
			ERROR("SPI: %s: incorrect lenbuf: %u\n", __func__, lenbuf);
			return -EFAULT;
		}

		in = buf;
		lenin = lenbuf;
		if (adr_mode == ADR_MODE_4BYTE) {
			SPI_SET_ADDRESS_4BYTE(address, cmd);
			lencmd += SPI_ADR_LEN_4BYTE;
		} else if (adr_mode == ADR_MODE_3BYTE) {
			SPI_SET_ADDRESS_3BYTE(address, cmd);
			lencmd += SPI_ADR_LEN_3BYTE;
		} else {
			ERROR("SPI: %s: incorrect address mode\n", __func__);
			return -ECAPMODE;
		}

		break;
	default:
		ERROR("SPI: %s: unknown cmd_op: 0x%x\n", __func__, cmd_op);
		return -EINVAL;
	}

	err = transfer(line, cmd, lencmd, in, lenin, out, lenout);
	return err;
}

static int wren(int line)
{
	int err;
	uint8_t status;

	err = exec(line, CMD_FLASH_WREN, 0, 0, 0);
	if (err) {
		return err;
	}

	err = exec(line, CMD_FLASH_RDSR, 0, &status, 1);
	if (err) {
		return err;
	}

	return !(status & SPI_FLASH_SR_WEL);
}

static int wait(int line)
{
	int err;
	uint8_t status;
	const uint64_t timeout = timeout_init_us(1000 * 1000);

	do {
		err = exec(line, CMD_FLASH_RDSR, 0, &status, 1);
		if (err) {
			return err;
		}

		if (timeout_elapsed(timeout)) {
			err = -ETIMEDOUT;
			return err;
		}
	} while (status & SPI_FLASH_SR_WIP);

	return 0;
}

int spi_flash_erase(int line, uint32_t adr, size_t size)
{
	int err;

	VERBOSE("SPI: %s(0x%x, 0x%lx)\n", __func__, adr, size);

	if (size % BAIKAL_BOOT_SPI_SUBSECTOR) {
		ERROR("SPI: wrong erase size\n");
		return -1;
	}

	while (size) {
		err = wren(line);
		if (err) {
			return err;
		}

		err = exec(line, CMD_FLASH_SSE, adr, 0, 0);
		if (err) {
			return err;
		}

		err = wait(line);
		if (err) {
			return err;
		}

		adr  += BAIKAL_BOOT_SPI_SUBSECTOR;
		size -= BAIKAL_BOOT_SPI_SUBSECTOR;
	}

	return 0;
}

int spi_flash_write(int line, uint32_t adr, void *data, size_t size)
{
	int err;
	uint8_t *pdata = data;

	VERBOSE("SPI: %s(0x%x, 0x%lx)\n", __func__, adr, size);

	while (size) {
		int part = MIN(size, SPI_MAX_WRITE);
		int p1 = adr / SPI_MAX_WRITE; /* page number */
		int p2 = (adr + part) / SPI_MAX_WRITE;

		if (p1 != p2) { /* page overflow ? */
			part = p2 * SPI_MAX_WRITE - adr; /* fix part size */
		}

		err = wren(line);
		if (err) {
			return err;
		}

		err = exec(line, CMD_FLASH_PP, adr, pdata, part);
		if (err) {
			return err;
		}

		err = wait(line);
		if (err) {
			return err;
		}

		adr   += part;
		pdata += part;
		size  -= part;
	}

	return 0;
}

int spi_flash_read(int line, uint32_t adr, void *data, size_t size)
{
	int err;
	uint8_t *pdata = data;

	VERBOSE("SPI: %s(0x%x, 0x%lx)\n", __func__, adr, size);

	while (size) {
		int part = MIN(size, SPI_MAX_READ);

		err = exec(line, CMD_FLASH_READ, adr, pdata, part);
		if (err) {
			return err;
		}

		adr   += part;
		pdata += part;
		size  -= part;
	}

	return 0;
}

static int spi_flash_detect(int line)
{
	int err;
	uint8_t id[SPI_NOR_MAX_ID_LEN];
	int try = 10;

	while (try--) {
		err = exec(line, CMD_FLASH_RDID, 0, id, sizeof(id));
		if (err) {
			return err;
		}

		if ((id[0] == 0x00 && id[1] == 0x00 && id[2] == 0x00) ||
		    (id[0] == 0xff && id[1] == 0xff && id[2] == 0xff)) {
			return -1;
		}
	}

	INFO("SPI: flash chip detected, JEDEC ID: 0x%02x%02x%02x\n", id[0], id[1], id[2]);
	return 0;
}

static int spi_flash_3byte(int line)
{
	int err;

	err = wren(line);
	if (err) {
		return err;
	}

	err = exec(line, CMD_FLASH_EX4BYTEADDR, 0, 0, 0);
	if (err) {
		return err;
	}

	adr_mode = ADR_MODE_3BYTE;
	return 0;
}

static int spi_flash_4byte(int line)
{
	int err;

	err = wren(line);
	if (err) {
		return err;
	}

	err = exec(line, CMD_FLASH_EN4BYTEADDR, 0, 0, 0);
	if (err) {
		return err;
	}

	adr_mode = ADR_MODE_4BYTE;
	return 0;
}

int spi_flash_init(int line)
{
	int err;

#ifndef IMAGE_BL1
	INFO("SPI: default parameters\n");
	INFO("SPI: clock div:  %u\n", BAIKAL_BOOT_SPI_BAUDR);
#ifdef BAIKAL_BOOT_SPI_CS_GPIO_PIN
	INFO("SPI: flash CS#:  %u\n", BAIKAL_BOOT_SPI_CS_GPIO_PIN);
#endif
	INFO("SPI: flash size: %u MiB\n", BAIKAL_BOOT_SPI_SIZE / 1024 / 1024);
#endif

	SPI_SPIENR  = SPI_SPIENR_SPI_DE;
	SPI_CTRLR0  = 7 << SPI_DFS_OFFSET;
	SPI_CTRLR1  = 0;
	SPI_BAUDR   = BAIKAL_BOOT_SPI_BAUDR;
	SPI_IMR     = 0;
	SPI_SER     = 0;
	SPI_TXFTLR  = 0;
	SPI_RXFTLR  = 0;
	SPI_DMACR   = 0;
	SPI_DMATDLR = 0;
	SPI_DMARDLR = 0;
	SPI_RX_DLY  = 0;

	err = spi_flash_detect(line);
	if (err) {
		ERROR("SPI: not found\n");
		return err;
	}

	/* Set address mode */
	if (BAIKAL_BOOT_SPI_SIZE > 16 * 1024 * 1024) {
		spi_flash_4byte(line);
	} else {
		spi_flash_3byte(line);
	}

	return 0;
}
