/*
 * Copyright (c) 2020-2021, Baikal Electronics, JSC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/console.h>

#include <baikal_console.h>
#include <baikal_def.h>

#if defined(BAIKAL_CONSOLE_16550)
#include <drivers/ti/uart/uart_16550.h>
#elif defined(BAIKAL_CONSOLE_PL011)
#include <drivers/arm/pl011.h>
#endif

static console_t baikal_boot_console;

void baikal_console_boot_end(void)
{
	console_flush();
	(void)console_unregister(&baikal_boot_console);
}

void baikal_console_boot_init(void)
{
	int rc;
#if defined(BAIKAL_CONSOLE_16550)
	rc = console_16550_register(
#elif defined(BAIKAL_CONSOLE_PL011)
	rc = console_pl011_register(
#endif
				    BAIKAL_UART_BASE,
				    BAIKAL_UART_CLK_IN_HZ,
				    BAIKAL_UART_BAUDRATE,
				    &baikal_boot_console);
	if (rc == 0) {
		/*
		 * The crash console doesn't use the multi console API, it uses
		 * the core console functions directly. It is safe to call panic
		 * and let it print debug information.
		 */
		panic();
	}
#if DEBUG
	console_set_scope(&baikal_boot_console,	CONSOLE_FLAG_BOOT  |
						CONSOLE_FLAG_CRASH |
						CONSOLE_FLAG_RUNTIME);
#endif
}
