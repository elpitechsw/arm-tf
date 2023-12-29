/*
 * Copyright (c) 2020-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BM1000_SMBUS_H
#define BM1000_SMBUS_H

enum smbus_sht {
	SMBUS_SHT_400KHZ = 0,
	SMBUS_SHT_100KHZ = 1
};

unsigned int smbus_txrx(const uintptr_t base,
			const unsigned int iclk,
			const enum smbus_sht sht,
			const unsigned int sclclk,
			const unsigned int targetaddr,
			const void *const txbuf,
			const unsigned int txbufsize,
			void *const rxbuf,
			const unsigned int rxbufsize);

#endif /* BM1000_SMBUS_H */
