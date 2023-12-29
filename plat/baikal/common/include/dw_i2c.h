/*
 * Copyright (c) 2020-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DW_I2C_H
#define DW_I2C_H

int i2c_txrx(const uintptr_t base,
	     const unsigned int iclk,
	     const unsigned int targetaddr,
	     const void *const txbuf,
	     const unsigned int txbufsize,
	     void *const rxbuf,
	     const unsigned int rxbufsize);

#endif /* DW_I2C_H */
