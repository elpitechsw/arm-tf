/*
 * Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.
 *
 * Font header file
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BM1000_FONT_H
#define BM1000_FONT_H

#include <stdint.h>

#define FONT_WIDTH	8	/* witdh > 8 is not supported */
#define FONT_HEIGHT	19
#define FONT_FIRST_CHAR	0x20
#define FONT_LAST_CHAR	0x7f

extern const uint8_t font[];

#endif /* BM1000_FONT_H */
