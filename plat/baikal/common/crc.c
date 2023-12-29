/*
 * Copyright (c) 2020-2023, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arm_acle.h>
#include <crc.h>

uint16_t crc16(const void *data, size_t size, uint16_t crc)
{
	const uint8_t *ptr = data;

	while (size--) {
		unsigned int i = 0;

		crc ^= *ptr++ << 8;
		while (i++ < 8) {
			if (crc & 0x8000) {
				crc = crc << 1 ^ 0x1021;
			} else {
				crc <<= 1;
			}
		}
	}

	return crc;
}

/*
 * compute CRC-32 using Arm intrinsic function
 *
 * This function is similar to tf_crc32() from common/tf_crc32.c.
 * The main difference is crc32() allows input buffer to start from
 * address 0 (which is important for checksumming contents of mailbox).
 *
 * Make sure to add a compile switch '-march=armv8-a+crc"
 * for successful compilation of this file.
 */
uint32_t crc32(const void *data, size_t size, uint32_t crc)
{
	uint32_t calc_crc = ~crc;
	const unsigned char *local_buf = data;
	size_t local_size = size;

	/* Calculate CRC over byte data */
	while (local_size != 0UL) {
		calc_crc = __crc32b(calc_crc, *local_buf);
		local_buf++;
		local_size--;
	}

	return ~calc_crc;
}
